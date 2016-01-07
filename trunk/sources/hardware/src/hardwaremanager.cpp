// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    :
// // Version :
// // Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//

#include "hardwaremanager.h"

//Variant
#include "utils/misc/variant.h"

//thelib
#include "system/systemmanager.h"
#include "system/systemdefine.h"
#include "system/nvramdefine.h"
#include "network/networkmanager.h"
#include "application/baseclientapplication.h"

// Camera
#include "qic/qicmjpgvideocapture.h"
#include "qic/qicavcvideocapture.h"
#include "qic/qic32avcvideocapture2.h"
#include "qic/qicmotioncapture2.h"

//Platform dependent header files
//Audio capture device
#if 0
#if (defined(__x86__))
#include "audio/alsaaudiocapdevice.h"
#elif (defined(__CL1830__))
#include "audio/pcmaudiocapdevice.h"
#elif (defined(__MT7620__))
#if (defined(__I2SPCM__))
#include "audio/pcmaudiocapdevice.h"
#elif (defined(__ALSA__))
#include "audio/alsaaudiocapdevice.h"
#endif
#else
#error "audio platform not supported"
#endif
#include "audio/speaker.h"
#endif

//GPIO control device
#if (defined(__x86__))
#include "gpio/gpiox86.h"
#elif (defined(__CL1830__))
#include "gpio/gpiocl1830.h"
#elif (defined(__MT7620__))
#include "gpio/gpiomt7620.h"
#else
#error "gpio platform not supported"
#endif

//NVRam
#define NVRAM_SECTION "2860"
//#if (defined(__x86__))
#include "nvram/nvramx86.h"
#define NVRAM_PATH "./nvram_default.xml"
/*#elif (defined(__CL1830__))
#include "nvram/nvramcl1830.h"
#elif (defined(__MT7620__))
#include "nvram/nvrammt7620.h"
#else
#error "nvram platform not supported"
#endif*/

//CPU
#include "basecpudevice.h"

//Memory
#include "basememorydevice.h"

//Storage
#include "storage/basestoragedevice.h"
#include "storage/nfsstoragedevice.h"

//Network
#if (defined(__x86__))
#define EXTERNAL_IF "eth0"
#elif (defined(__CL1830__))
#define EXTERNAL_IF "br0"
#elif (defined(__MT7620__))
#define EXTERNAL_IF "br0"
#else
#error "network external interface not defined"
#endif

//Network
#if (defined(__x86__))
#define SCRIPT_PATH "scripts"
#elif (defined(__CL1830__))
#define SCRIPT_PATH "/sbin/scripts"
#elif (defined(__MT7620__))
#define SCRIPT_PATH "/sbin/scripts"
#else
#error "network script path not defined"
#endif

#include "system/eventdefine.h"


map<HardwareType, BaseHardwareAbstractDevice*> HardwareManager::_hwInstances;
map<uint32_t, DeviceErrorStatus> HardwareManager::_status;
int HardwareManager::_pipe_fds[2];

bool HardwareManager::ShutdownHardware()
{
  FOR_MAP(_hwInstances, HardwareType, BaseHardwareAbstractDevice*, i) {
    MAP_VAL(i)->Close();
  }
  return true;
}

// Initialize hardware device and relevant information
bool HardwareManager::InitHardwareDevice(Variant& configuration)
{

  DEBUG("\n \
  //----------------------------------------------------------------------------\n \
  // Initialize System\n \
  //----------------------------------------------------------------------------\n \
  ");

  // CPU
  CreateCPU("/proc/stat");
  BaseCPUDevice *pCPUDevice=reinterpret_cast<BaseCPUDevice*>(GetHardwareInstance(HT_CPU));
  pCPUDevice->InitializeDevice();



  // NVRAM
  if(configuration.HasKeyChain(V_MAP, false, 1, "nvram")) {
    Variant nvramConfig = configuration["nvram"];
//#if (defined(__x86__))
    string nvramPath = NVRAM_PATH;
    if (nvramConfig.HasKey("path")) {
      nvramPath = (string) nvramConfig["path"];
    } else{
      FATAL("Unable to find nvram path. Default path is %s ", STR(nvramPath));
    }
    CreateNVRam(nvramPath);
/*#elif (defined(__CL1830__))
    CreateNVRam("");
#elif (defined(__MT7620__))
    CreateNVRam("");
#else
#error "CreateNVRam not defined"
#endif*/
    string nvramSection = NVRAM_SECTION;
    if (nvramConfig.HasKey("section")) {
      nvramSection = (string) nvramConfig["section"];
    } else{
      FATAL("Unable to find nvram section. Default section is %s ", STR(nvramSection));
    }
    SystemManager::SetNVRamSection(nvramSection);
  }
  else{
    FATAL("Unable to find nvram setting");
    return false;
  }

  //Memory, need to read nvram setting
  uint32_t minMemSize=stringToNum<uint32_t>(SystemManager::GetNVRam(NVRAM_EVENT_TMP_MINSIZE));
  CreateMemory(minMemSize);
  BaseMemoryDevice *pMemoryDevice=reinterpret_cast<BaseMemoryDevice*>(GetHardwareInstance(HT_MEM));
  pMemoryDevice->InitializeDevice();


  DEBUG("\n \
  //----------------------------------------------------------------------------\n \
  // Initialize Video\n \
  //----------------------------------------------------------------------------\n \
  ");

  bool enableAVC = false;
  bool enableMJPG = false;

  if(pipe(_pipe_fds) < 0) {
    FATAL("Create pipe failed, %s\n", strerror(errno));
    return NULL;
  }

  if (configuration.HasKeyChain(V_MAP, false, 1, "video")) {
    Variant videoConfig = configuration["video"];

    // AVC
    if (videoConfig.HasKey("avc", false)) {
      Variant temp = videoConfig["avc"];
      if(temp.HasKey("enable", false)) {
        enableAVC = (bool) temp["enable"];
      }
      if(temp.HasKey("deviceNode")){
        string avcDeviceNode=(string) temp["deviceNode"];
        if(enableAVC){
          INFO("Create AVC Video Capture");
          CreateQICAVCVideoCapture(avcDeviceNode);
        }
      }
    }
    // MJPG
    if (videoConfig.HasKey("mjpg", false)) {
      Variant temp = videoConfig["mjpg"];
      if (temp.HasKey("enable", false)){
        enableMJPG = (bool) temp["enable"];
      }
      if(temp.HasKey("deviceNode")){
        string mjpgDeviceNode=(string) temp["deviceNode"];
        if(enableMJPG){
          INFO("Create MJPG Video Capture");
          CreateQICMJPGVideoCapture(mjpgDeviceNode);
        }
      }
    }
    // Motion
    if(enableAVC){
      string motionDeviceNode = "/dev/null";
      CreateQICMotionCapture(motionDeviceNode);
    }
//    if(videoConfig.HasKey("motion", "false")) {
//      Variant temp = videoConfig["motion"];
//      if(temp.HasKey("deviceNode")){
//        string motionDeviceNode=(string) temp["deviceNode"];
//        if(enableAVC){
//          //enableMotion = true;
//          /*if(motionDeviceNode.compare("/dev/input/event0")==0)
//            INFO("create motionDeviceNode = /dev/input/event0");
//          else
//            INFO("create motionDeviceNode is not /dev/input/event0");*/
//          CreateQICMotionCapture(motionDeviceNode);
//        }
//      }
//    }
  }

  DEBUG("\n \
  //----------------------------------------------------------------------------\n \
  // Initialize Audio\n \
  //----------------------------------------------------------------------------\n \
  ");

  string micDeviceNode;
  string speakerDeviceNode;
  string rmsTh=SystemManager::GetNVRam(NVRAM_AUDIO_RMSTH);
  string audioAlarm=
      SystemManager::GetNVRam(NVRAM_AUDIO_ALARM_ENABLED);

  if(configuration.HasKeyChain(V_MAP, false, 1, "audio")) {
    Variant audioConfig = configuration["audio"];
    if (audioConfig.HasKey("microphone")) {
      Variant mic = audioConfig["microphone"];
      if (mic.HasKey("deviceNode")){
        micDeviceNode=(string) mic["deviceNode"];
      }
    }
    if (audioConfig.HasKey("speaker")) {
      Variant spk = audioConfig["speaker"];
      if(spk.HasKey("deviceNode")){
        speakerDeviceNode=(string) spk["deviceNode"];
      }
    }
  }

#if 0
  if (!micDeviceNode.empty()){
    CreateAudioCapture(micDeviceNode);

    BaseAudioCaptureDevice *pBaseAudioCapture=reinterpret_cast<BaseAudioCaptureDevice*>(GetHardwareInstance(HT_MIC));
    if (pBaseAudioCapture) {
      if (!rmsTh.empty() && isNumeric(rmsTh)) {
        pBaseAudioCapture->SetAudioRMS(stringToNum<double>(rmsTh));
      }
      if (audioAlarm.compare("0")==0) {
        pBaseAudioCapture->DisableAlarm();
      }
      else {
        pBaseAudioCapture->EnableAlarm();
      }
    }
  }

  if(!speakerDeviceNode.empty()){
    CreateAudioPlayer(speakerDeviceNode);
    Speaker *pSpeaker=reinterpret_cast<Speaker*>(GetHardwareInstance(HT_SPEAKER));
    if (pSpeaker) {
      pSpeaker->InitializeDevice();
    }
  }
#endif

  DEBUG("\n \
  //----------------------------------------------------------------------------\n \
  // Initialize IO\n \
  //----------------------------------------------------------------------------\n \
  ");
  // GPIO
  gpio_status defaultSetting = {0};
  defaultSetting.enablePIR = false;
  defaultSetting.enableALS = false;

  string nvramStr = SystemManager::GetNVRam(NVRAM_IO_PIR_ENABLED);
  if(isNumeric(nvramStr)){
    defaultSetting.enablePIR = (bool) stringToNum<bool>(nvramStr);
    //FATAL("enablePIR:%d, nvramStr:%s", defaultSetting.enablePIR, nvramStr.c_str());
  }

  if(GetHardwareInstance(HT_GPIO) == NULL){
#if (defined(__x86__))
    GPIOX86::GetInstance(defaultSetting);
#elif (defined(__CL1830__))
    GPIOCL1830::GetInstance(defaultSetting);
#elif (defined(__MT7620__))
    //GPIOMT7620::GetInstance(defaultSetting);
#else
#error ("GPIO Get Instanace not defined");
#endif
  } else{
    WARN("GPIO devices was already created");
  }

  //Need nvram hardware instance to get nvram value, put after the nvram creation
  DEBUG("\n \
  //----------------------------------------------------------------------------\n \
  // Initialize Storage\n \
  //----------------------------------------------------------------------------\n \
  ");

  string mountPath=SystemManager::GetNVRam(NVRAM_STORAGE_MOUNT_PATH);
  uint64_t minSize=stringToNum<uint64_t>(SystemManager::GetNVRam(NVRAM_RECORD_MINSIZE));
  CreateStorage(mountPath);
  BaseStorageDevice *pStorageDevice=reinterpret_cast<BaseStorageDevice*>(GetHardwareInstance(HT_STORAGE));
  pStorageDevice->InitializeDevice();
  pStorageDevice->SetMinSize(minSize);

  // Network Storage
  string nfsPath=SystemManager::GetNVRam(NVRAM_NETWORK_NFS_LOCAL_PATH);
  CreateNFSStorage(nfsPath);
  NFSStorageDevice *pNFSStorageDevice=reinterpret_cast<NFSStorageDevice*>(GetHardwareInstance(HT_NFS_STORAGE));
  pNFSStorageDevice->InitializeDevice();
  pNFSStorageDevice->SetMinSize(minSize);

  // Network
  string extNetworkIF = EXTERNAL_IF;
  if(configuration.HasKeyChain(V_MAP, false, 1, "network")) {
    Variant networkConfig = configuration["network"];
    if (networkConfig.HasKey("externalInterface")) {
      extNetworkIF = (string) networkConfig["externalInterface"];
    }
    else{
      FATAL("Unable to find external network interface from streamer.lua");
    }
  }else{
      FATAL("External network interface is not defined. Default is %s", STR(extNetworkIF));
  }
  SystemManager::SetExternalNetworkIF(extNetworkIF);
  // cache network interface
  SystemManager::UpdateNetworkInfo();

  //Create CPU hardware for monitoring
  // Config
  SystemManager::SetConfigInfo(DEVICE_HW_CONFIG, configuration);
  // cache device information
// Raylin temp remove to prevent crash
  SystemManager::UpdateDeviceInfo();

  return true;
}

bool HardwareManager::RegisterHardwareDevice(BaseHardwareAbstractDevice *pInstance)
{
  HardwareType hwType;
  if(pInstance == NULL){
    return false;
  }
  hwType = pInstance->GetType();
  if(!MAP_HAS1(_hwInstances, hwType)){
    _hwInstances[hwType] = pInstance;
  }else{
    WARN("device type %d was already registered", hwType);
    return false;
  }
  return true;
}

bool HardwareManager::UnRegisterHardwareDevice(HardwareType hwType)
{
  if(!MAP_HAS1(_hwInstances, hwType)){
    WARN("deviice type %d was not registered", hwType);
  }else{
    _hwInstances.erase(hwType);
  }
  return true;
}


BaseHardwareAbstractDevice* HardwareManager::GetHardwareInstance(HardwareType hwType)
{
  if(MAP_HAS1(_hwInstances, hwType)){
    return _hwInstances[hwType];
  }
  return NULL;
}

/*
 * Private Memeber Function
 */

void HardwareManager::CreateQICMJPGVideoCapture(string devNode) {
  if(GetHardwareInstance(HT_VIDEO_MJPG) == NULL){
    QICMJPGVideoCapture::GetInstance(devNode);
  }else{
    WARN("MJPG video capture was already created");
  }
}

void HardwareManager::CreateQICMotionCapture(string devNode) {
#if (defined(QIC1832))
  if(GetHardwareInstance(HT_VIDEO_MOTION) == NULL){
    //QICMotionCapture::GetInstance(devNode);
    QICMotionCapture2::GetInstance(devNode, _pipe_fds[0]);
  }else{
    WARN("Motion capture was already created");
  }
#endif
}

void HardwareManager::CreateQICAVCVideoCapture(string devNode) {
  if(GetHardwareInstance(HT_VIDEO_AVC) == NULL){
#if (defined(SKYPEAPI))
    SkypeAVCVideoCapture::GetInstance(devNode);
#elif (defined(QIC1816))
    QICAVCVideoCapture::GetInstance(devNode);
#elif (defined(QIC1832))
    //QIC32AVCVideoCapture::GetInstance(devNode);
    QIC32AVCVideoCapture2::GetInstance(devNode, _pipe_fds[1]);
#endif
  }else{
    WARN("AVC video capture was already created");
  }
}

void HardwareManager::CreateStorage(string mountPath) {
  if(GetHardwareInstance(HT_STORAGE) == NULL) {
    BaseStorageDevice::GetInstance(mountPath);
  }
  else {
    WARN("Storage was already created");
  }
}

void HardwareManager::CreateCPU(string path) {
  if(GetHardwareInstance(HT_CPU) == NULL) {
    BaseCPUDevice::GetInstance(path);
  }
  else {
    WARN("CPU was already created");
  }
}

void HardwareManager::CreateMemory(uint32_t size) {
  if(GetHardwareInstance(HT_MEM) == NULL) {
    BaseMemoryDevice::GetInstance(size);
  }
  else {
    WARN("Memory was already created");
  }
}

void HardwareManager::CreateNFSStorage(string mountPath) {
  if(GetHardwareInstance(HT_NFS_STORAGE) == NULL) {
    NFSStorageDevice::GetInstance(mountPath);
  }
  else {
    WARN("Storage was already created");
  }
}

void HardwareManager::CreateAudioCapture(string devNode) {
#if 0
  if(GetHardwareInstance(HT_MIC) == NULL){
#if (defined(__x86__))
    AlsaAudioCapDevice::GetInstance(devNode);
#elif (defined(__MT7620__))
#if (defined(__I2SPCM__))
    I2SAudioCapDevice::GetInstance(devNode);
#elif (defined(__ALSA__))
    AlsaAudioCapDevice::GetInstance(devNode);
#endif
#elif (defined(__CL1830__))
    I2SAudioCapDevice::GetInstance(devNode);
#else
#error ("GetInstance in CreateAudioCapture not defined")
#endif
  }else{
    WARN("Audio capture was already created");
  }
#endif
}

void HardwareManager::CreateAudioPlayer(string devNode) {
//  if(GetHardwareInstance(HT_SPEAKER) == NULL){
//    Speaker::GetInstance(devNode);
//  }else{
//    WARN("Audio playback was already created");
//  }
}

void HardwareManager::CreateNVRam(string filePath) {
  if(GetHardwareInstance(HT_NVRAM) == NULL){
//#if (defined(__x86__))
    NVRamX86::GetInstance(filePath);
/*#elif (defined(__CL1830__))
    NVRamCL1830::GetInstance();
#elif (defined(__MT7620__))
    NVRamCL1830::GetInstance();
#else
#error ("NVRam GetInstance not defined");
#endif*/
  }else{
    WARN("NVRam device was already created");
  }

}

void HardwareManager::CheckStatus(BaseClientApplication *pApp, double timeStamp) {
  //Check Storage device
  FOR_MAP(_hwInstances, HardwareType, BaseHardwareAbstractDevice*, it) {
    MAP_VAL(it)->CheckStatus(timeStamp);
  }
  FOR_MAP (_status, uint32_t, DeviceErrorStatus, i) {
    if (MAP_VAL(i).bSend) {
      if (pApp->OnNotifyEvent(CLOUD_MSG_ERROR, MAP_KEY(i), 0, "")) {
        MAP_VAL(i).bSend = false;
      }
    }
  }
}

void HardwareManager::SetStatus(uint32_t statusCode, bool bSend) {
  if (MAP_HAS1(_status, statusCode)) {
    if (!_status[statusCode].bError) {
      _status[statusCode].bError = true;
      if (bSend) {
        _status[statusCode].bSend = true;
      }
    }
  }
  else {
    DeviceErrorStatus errorStatus={true, bSend};
    _status.insert(pair<uint32_t, DeviceErrorStatus>(statusCode, errorStatus));
  }
}

void HardwareManager::ClearStatus(uint32_t statusCode) {
  if (MAP_HAS1(_status, statusCode)) {
    _status[statusCode].bError = false;
  }
}

bool HardwareManager::IsErrorStatus(uint32_t errStatus) {
  if (MAP_HAS1(_status, errStatus)) {
    return _status[errStatus].bError;
  }
  return false;
}

bool HardwareManager::GetSendStatus(uint32_t statusCode) {
  if (MAP_HAS1(_status, statusCode)) {
    return _status[statusCode].bSend;
  }
  return false;
}

void HardwareManager::ClearSendStatus(uint32_t statusCode) {
  if (MAP_HAS1(_status, statusCode)) {
    _status[statusCode].bSend=true;
  }
}
