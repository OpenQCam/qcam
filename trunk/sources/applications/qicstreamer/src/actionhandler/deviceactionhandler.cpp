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
//

#ifdef HAS_PROTOCOL_CLI
#include <sys/time.h>
#include <arpa/inet.h>
//thelib
#include "clidefine.h"
#include "network/networkmanager.h"
#include "system/systemmanager.h"
#include "system/systemdefine.h"
#include "system/eventdefine.h"
#include "system/systeminfo.h"
#include "system/nvramdefine.h"
#include "protocols/http/httpauthhelper.h"
#include "application/baseclientapplication.h"
//hardware
#include "video/videodefine.h"
#include "video/basev4l2videocapture.h"
#include "video/baseavcvideocapture.h"
#include "gpio/basegpiodevice.h"
#include "nvram/basenvramdevice.h"
#include "hardwaremanager.h"
//application
#include "actionhandler/deviceactionhandler.h"
#include "appprotocolhandler/wsclientappprotocolhandler.h"


using namespace app_qicstreamer;


DeviceActionHandler::DeviceActionHandler(string version, BaseClientApplication *pApplication)
: BaseActionHandler(AT_DEVICE, version, pApplication),
  _pMJPGCaptureInstance(0),
  _pAVCCaptureInstance(0),
  _pSystemInfo(0) {

  _pMJPGCaptureInstance = reinterpret_cast<BaseV4L2VideoCapture *>(HardwareManager::GetHardwareInstance(HT_VIDEO_MJPG));
  _pAVCCaptureInstance = reinterpret_cast<BaseAVCVideoCapture *>(HardwareManager::GetHardwareInstance(HT_VIDEO_AVC));


  if(_pSystemInfo == NULL){
    _pSystemInfo = new SystemInfo(GetCallbackId(), pApplication->GetStreamsManager());
    _pSystemInfo->EnqueueForTimeEvent(1);
  }
}

DeviceActionHandler::~DeviceActionHandler() {
  /*no need to delete a handler, the iohandlermanger will delete it */
  //if(_pSystemInfo) delete _pSystemInfo;

  (*_actionMaps[QIC_CLI_DEVICE_HARDWARE])[true]->clear();
  (*_actionMaps[QIC_CLI_DEVICE_HARDWARE])[false]->clear();
  (*_actionMaps[QIC_CLI_DEVICE_SYSTEM])[true]->clear();
  (*_actionMaps[QIC_CLI_DEVICE_SYSTEM])[false]->clear();
  _actionMaps[QIC_CLI_DEVICE_HARDWARE]->clear();
  _actionMaps[QIC_CLI_DEVICE_SYSTEM]->clear();
  _actionMaps.clear();

}

bool DeviceActionHandler::Initialize() {
  if(_pMJPGCaptureInstance == NULL) return false;
  if(_pAVCCaptureInstance == NULL) return false;
  SystemManager::SetConfigInfo(DEVICE_APP_CONFIG, _pApplication->GetConfiguration());

  ActionMap *pHardwareGetActionMap = new ActionMap;
  ActionMap *pHardwarePostActionMap = new ActionMap;
  ActionMap *pSystemGetActionMap = new ActionMap;
  ActionMap *pSystemPostActionMap = new ActionMap;
  CollectionMap *pHardwareMap = new CollectionMap;
  CollectionMap *pSystemMap = new CollectionMap;

  //construct map
  (*pHardwareMap)[true] = pHardwareGetActionMap;
  (*pHardwareMap)[false] = pHardwarePostActionMap;
  (*pSystemMap)[true] = pSystemGetActionMap;
  (*pSystemMap)[false] = pSystemPostActionMap;
  _actionMaps[QIC_CLI_DEVICE_HARDWARE] = pHardwareMap;
  _actionMaps[QIC_CLI_DEVICE_SYSTEM] = pSystemMap;

  //Hardware API
  (*pHardwareGetActionMap)[QIC_CLI_DEVICE_HW_DEVICE_INFO] = reinterpret_cast<Action>(&DeviceActionHandler::GetHWDeviceInfo);
  (*pHardwareGetActionMap)[QIC_CLI_DEVICE_HW_FW_INFO] = reinterpret_cast<Action>(&DeviceActionHandler::GetHWFirmwareInfo);
  (*pHardwareGetActionMap)[QIC_CLI_DEVICE_HW_CAPABILITIES] = reinterpret_cast<Action>(&DeviceActionHandler::GetHWCapabilities);
  (*pHardwareGetActionMap)[QIC_CLI_DEVICE_HW_STATUS] = reinterpret_cast<Action>(&DeviceActionHandler::GetHWStatus);

  //System API
  (*pSystemGetActionMap)[QIC_CLI_DEVICE_SYS_SYSINFO] = reinterpret_cast<Action>(&DeviceActionHandler::GetSystemInfo);
  (*pSystemGetActionMap)[QIC_CLI_DEVICE_SYS_TIMESTAMP] = reinterpret_cast<Action>(&DeviceActionHandler::GetTimeStamp);
  (*pSystemGetActionMap)[QIC_CLI_DEVICE_SYS_NVRAM] = reinterpret_cast<Action>(&DeviceActionHandler::GetSystemNVRam);
  (*pSystemGetActionMap)[QIC_CLI_DEVICE_SYS_SETTINGS] = reinterpret_cast<Action>(&DeviceActionHandler::GetSystemSettings);
  (*pSystemGetActionMap)[QIC_CLI_DEVICE_SYS_DATETIME] = reinterpret_cast<Action>(&DeviceActionHandler::GetSystemDateTime);

  (*pSystemPostActionMap)[QIC_CLI_DEVICE_SYS_NVRAM] = reinterpret_cast<Action>(&DeviceActionHandler::PostSystemNVRam);
  (*pSystemPostActionMap)[QIC_CLI_DEVICE_SYS_SYSINFO] = reinterpret_cast<Action>(&DeviceActionHandler::PostSystemInfo);
  (*pSystemPostActionMap)[QIC_CLI_DEVICE_SYS_DATETIME] = reinterpret_cast<Action>(&DeviceActionHandler::PostSystemDateTime);
  (*pSystemPostActionMap)[QIC_CLI_DEVICE_SYS_SETTINGS] = reinterpret_cast<Action>(&DeviceActionHandler::PostSystemSettings);
  (*pSystemPostActionMap)[QIC_CLI_DEVICE_SYS_FW_DOWNLOAD] = reinterpret_cast<Action>(&DeviceActionHandler::PostSystemFWDownload);
  (*pSystemPostActionMap)[QIC_CLI_DEVICE_SYS_FW_UPGRADE] = reinterpret_cast<Action>(&DeviceActionHandler::PostSystemFWUpgrade);
  (*pSystemPostActionMap)[QIC_CLI_DEVICE_SYS_FACTORY_RESET] = reinterpret_cast<Action>(&DeviceActionHandler::PostSystemFactoryReset);
  (*pSystemPostActionMap)[QIC_CLI_DEVICE_SYS_REBOOT] = reinterpret_cast<Action>(&DeviceActionHandler::PostSystemReboot);
  (*pSystemPostActionMap)[QIC_CLI_DEVICE_SYS_COMMAND] = reinterpret_cast<Action>(&DeviceActionHandler::PostSystemCommand);
  (*pSystemPostActionMap)[QIC_CLI_DEVICE_SYS_COMMAND] = reinterpret_cast<Action>(&DeviceActionHandler::PostSystemCommand);
  return true;
}

void DeviceActionHandler::OnCmdFinished(uint32_t msgId, uint8_t*pData,
                                        uint32_t dataLength) {
}

bool DeviceActionHandler::IsValidAction(map<string, CollectionMap*> *map,
                                        string collection,
                                        string id,
                                        bool isReadyOnly) {
  return true;
}


bool DeviceActionHandler::IsValidAction(string action) {
  return true;
}


RestHTTPCode DeviceActionHandler::DoAction(BaseProtocol *pFrom,
                                           bool isRO,
                                           vector<string> &resource,
                                           Variant &payload,
                                           Variant &message) {
  string collection = resource[3];
  string id = resource[4];

  if(!MAP_HAS1(_actionMaps, collection)) {
    return SendFailure(HC_400_BAD_REQUEST, EC_106_COLLECTION_NOT_FOUND,
        "Unable to find collection: " + collection, message);
  }
  if(!MAP_HAS1(*_actionMaps[collection], isRO)){
    return SendFailure(HC_400_BAD_REQUEST, EC_103_METHOD_NOT_ALLOWED,
        "Invalid method: " + isRO, message);
  }
  if(!MAP_HAS1(*(*_actionMaps[collection])[isRO], id)){
    return SendFailure(HC_400_BAD_REQUEST, EC_107_RESOURCE_ID_NOT_FOUND,
        "Unable to find resource id: " + id, message);
  }

  Action pAction = (*(*_actionMaps[collection])[isRO])[id];
  return ((this->*pAction)(payload[REST_PARAM_STR], message));
}

// API Functions
// Hardware
RestHTTPCode DeviceActionHandler::GetHWDeviceInfo(Variant &params, Variant &message) {
  message = SystemManager::GetGeneralInfo();
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::GetHWFirmwareInfo(Variant &params, Variant &message) {
  message = SystemManager::GetFirmwareInfo();
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::GetHWCapabilities(Variant &params, Variant &message) {
  /*
   */
  if(!fileExists("/sbin/icam_get_hardware_capabilities")){
    return SendSystemFailure(HC_500_INTERNAL_SERVER_ERROR,
        ERROR_CODE(EC_EMIT_COMMAND),
        ERROR_DESCRIPTION(EC_EMIT_COMMAND)+"command: icam_get_hardware_capabilities",
        message);
  }
  string output;
  uint32_t start = 0;
  SystemManager::DoSystemCommand("icam_get_hardware_capabilities", output);

  Variant result;
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("error")){
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_200_DEVICE_ERROR, result["error"], message);
  }

  result["ptz"]["pan"] = _pAVCCaptureInstance->IsPTSupported();
  result["ptz"]["tilt"] = _pAVCCaptureInstance->IsPTSupported();
  result["led"]["ir"] = _pAVCCaptureInstance->IsIRSupported();
  result["sensor"]["light"] = _pAVCCaptureInstance->IsALSSupported();
  message = result;
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::GetHWStatus(Variant &params, Variant &message) {
  NYI;
  return HC_200_OK;
}


RestHTTPCode DeviceActionHandler::GetTimeStamp(Variant &params, Variant &message) {
  uint64_t timeStamp;
  GETTIMESTAMP(timeStamp);

  //if (HTTPAuthHelper::TicketSize()>100) {
  //  return SendFailure(HC_400_BAD_REQUEST, EC_203_DEVICE_MAXIMUM_TICKET,
  //                     "Maximum ticket", message);
  //}
  message["timeStamp"]=timeStamp;
  //HTTPAuthHelper::SetAuthTimeStamp(timeStamp, timeStamp+5000);

  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::GetSystemInfo(Variant &params, Variant &message) {
  if (!_pSystemInfo) {
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
                       EC_201_DEVICE_SYSINFO_ERROR,
                       "No system info created", message);
  }

  message["enabled"]=_pSystemInfo->GetSystemInfo();
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::PostSystemInfo(Variant &params, Variant &message) {
  if (!params.HasKey("enabled")){
    return SendFailure(HC_400_BAD_REQUEST,
                       EC_108_INVALID_PARAMETERS,
                       "Parameter-enabled is required", message);
  }
  if (!_pSystemInfo) {
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
                       EC_201_DEVICE_SYSINFO_ERROR,
                       "No system info created", message);
  }

  _pSystemInfo->SetSystemInfo((string)params["enabled"]=="true");
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::GetSystemDateTime(Variant &params, Variant &message) {
  message["timeStamp"] = (uint32_t) (SystemManager::GetSystemTime()/1000);
  message["timeZone"] = SystemManager::GetNVRam("NtpTimeZone");
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::PostSystemDateTime(Variant &params, Variant &message) {
  uint64_t ts=0;
  string tz;

  if (params.HasKey("timeStamp")) {
    if (params["timeStamp"]!=V_INT64){
      return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS, "Invalid parameter type: timeStamp", message);
    }
    else {
      ts=(uint64_t)params["timeStamp"];
    }
  }
  else {
    return SendFailure(HC_400_BAD_REQUEST,
                       EC_109_MISSING_MANDATORY_PARAMETERS,
                       "Require parameter - timeStamp", message);
  }

  if (params.HasKey("timeZone")) {
    if (params["timeZone"]!=V_STRING){
      return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS, "Invalid parameter type: timeZone", message);
    }
    else {
      tz=(string)params["timeZone"];
    }
  }

  if (tz!="") {
    SystemManager::SetSystemTime(ts, tz);
  } else {
    SystemManager::SetSystemTime(ts);
  }

  //Renew websocket timeout
  WSClientAppProtocolHandler* pHandler=reinterpret_cast<WSClientAppProtocolHandler*>
    (_pApplication->GetProtocolHandler(PT_OUTBOUND_HTTP_WSCONNECTION));
  pHandler->UpdatePongTime((double)ts*1000);
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::GetSystemNVRam(Variant &params, Variant &message) {
  if(!params.HasKey("key")){
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "Need to specify parameter - key", message);
  }
  vector<string> keyMap;
  split(params["key"], ",", keyMap);
  if (keyMap.empty()) {
    return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
        "Invalid parameter - key", message);
  }

  FOR_VECTOR(keyMap, i) {
    string key=(string)keyMap[i];
    message[key] = SystemManager::GetNVRam(key);
  }
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::PostSystemNVRam(Variant &params, Variant &message) {
  if(!params.HasKey("nvram")){
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "Need to specify parameter - nvram", message);
  }

  if (params["nvram"]!=V_MAP) {
    return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
        "Invalid parameter - nvram", message);
  }

  Variant nvram=params["nvram"];
  FOR_MAP (nvram, string, Variant, i) {
    string key, value;
    if (MAP_VAL(i) == V_STRING) {
      key=MAP_KEY(i);
      value=(string)MAP_VAL(i);
    }
    else {
      return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
        "Invalid parameter value type - nvram", message);
    }
//    DEBUG ("key:%s, value:%s", STR(key), STR(value));
    if(!SystemManager::SetNVRam(key, value, false)){
      string errInfo=format("key:%s, value:%s", STR(params["key"]), STR(params["value"]));
      return SendSystemFailure(HC_500_INTERNAL_SERVER_ERROR,
                               ERROR_CODE(EC_WRITE_NVRAM),
                               ERROR_DESCRIPTION(EC_WRITE_NVRAM)+errInfo,
                               message);
    }
  }
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::GetSystemSettings(Variant &params, Variant &message) {

  BaseNVRamDevice *pNVRam = reinterpret_cast<BaseNVRamDevice *>
      (HardwareManager::GetHardwareInstance(HT_NVRAM));

  if (!pNVRam) {
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_202_DEVICE_NVRAM_NOT_FOUND,
        "No nvram device", message);
  }
  Variant settings;
  pNVRam->GetAll(settings);
  message["nvram"]=settings;
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::PostSystemSettings(Variant &params, Variant &message) {
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::PostSystemFWDownload(Variant &params, Variant &message) {
  Variant msg;
  if(!params.HasKey("filename")){
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "No filename specified", message);
  }
  if(!params.HasKey("serverAddr")) {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "No download server address specified", message);
  }
  if(!params.HasKey("sha1sum")) {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "No chksum specified", message);
  }

  string filename=(string)params["filename"];
  string server_addr=(string)params["serverAddr"];
  string chksum = (string)params["sha1sum"];
  string cmd="icam_download_fw " + filename + " " + server_addr + " " + chksum;

  UnixDomainSocketManager::ExecuteSystemCommand(cmd);
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::PostSystemFWUpgrade(Variant &params, Variant &message) {
  Variant msg;
  string mountPath=(string)SystemManager::GetNVRam(NVRAM_STORAGE_MOUNT_PATH);

  if (!params.HasKey("filename")){
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "no filename specified", message);
  }

  string filename=mountPath+"/"+(string)params["filename"];
  if (!fileExists(filename)){
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "file not found: " + mountPath, message);
  }

  string touchFile="/tmp/upgrade.act";
  string sha1sumCmd;
  if (params.HasKey("sha1sum") && params["sha1sum"]==V_STRING){
    sha1sumCmd="echo "+(string)params["sha1sum"]+" >> "+touchFile;
  }
  else {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "madatory parameter: sha1sum", message);
  }

  string filenameCmd="echo "+filename+" >> "+touchFile;
  system(STR("touch "+touchFile));
  system(STR(filenameCmd));
  system(STR(sha1sumCmd));
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::PostSystemReboot(Variant &params, Variant &message) {
  if(!fileExists("/sbin/icam_reboot")) {
    return SendSystemFailure(HC_500_INTERNAL_SERVER_ERROR,
        ERROR_CODE(EC_EMIT_COMMAND),
        ERROR_DESCRIPTION(EC_EMIT_COMMAND)+"command: icam_reboot",
        message);
  }
  string output;
  SystemManager::DoSystemCommand("icam_reboot", output);
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::PostSystemCommand(Variant &params, Variant &message) {
  string cmd;
  string output;

  if (!params.HasKey("cmd") ) {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "Need to specify parameter - cmd", message);
  }
  if (params["cmd"] != V_STRING) {
    return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
        "Invalid parameter type - cmd!=V_STRING", message);
  }

  cmd = (string)params["cmd"];
  SystemManager::DoSystemCommand(cmd, output);
  message["log"]=output;
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::PostSystemFactoryReset(Variant &params, Variant &message) {
  if(!fileExists("/sbin/icam_factory_reset")) {
    return SendSystemFailure(HC_500_INTERNAL_SERVER_ERROR,
        ERROR_CODE(EC_EMIT_COMMAND),
        ERROR_DESCRIPTION(EC_EMIT_COMMAND)+"command: icam_factory_reset",
        message);
  }
  string output;
  SystemManager::DoSystemCommand("icam_factory_reset", output);
  return HC_200_OK;
}

RestHTTPCode DeviceActionHandler::TestJsonp(Variant &params, Variant &message) {
  message["test"] = "aaaa";
  return HC_200_OK;
}

#endif /* HAS_PROTOCOL_CLI */
