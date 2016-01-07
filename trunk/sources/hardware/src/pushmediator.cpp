#include "pushmediator.h"
//thelib
#include "application/baseclientapplication.h"
#include "streaming/inmsgstream.h"
#include "streaming/streamsmanager.h"
#include "streaming/outjpgstream.h"
#include "system/nvramdefine.h"
#include "system/eventdefine.h"
#include "protocols/avhttpstream/avstreamdefine.h"

//hardware
#include "hardwaremanager.h"
#include "audio/baseaudiocapturedevice.h"
#include "storage/basestoragedevice.h"
#include "basememorydevice.h"
#include "video/basevideostream.h"
#include "video/baseavcvideocapture.h"
#include "streaming/baseaudiostream.h"
#include "mux/alarmmp4muxoutstream.h"
#include "outjpgfilestream.h"
#include "system/systemmanager.h"


#if 0
#define PV_WARN WARN
#define PV_DEBUG  DEBUG
#else
#define PV_WARN(...)
#define PV_DEBUG(...)
#endif

#define VIDEO_FILE_NAME "video.mp4"

PushHandler::PushHandler (StreamsManager *pStreamManager,
                          BaseVideoStream *pAVCStream,
                          BaseVideoStream *pMJPGInStream)
: IOTimer(),
  ISystemCommandCallback(),
  _pushInfo(),
  _lastMSGId(0)
{
  _pStorageDevice=reinterpret_cast<BaseStorageDevice*>(HardwareManager::GetHardwareInstance(HT_STORAGE));
  _pMemoryDevice=reinterpret_cast<BaseMemoryDevice*>(HardwareManager::GetHardwareInstance(HT_MEM));
  _pMP4Stream = new AlarmMP4MuxOutStream(pStreamManager, QIC_STREAM_MP4ALARMFILE, this);
  _pJPGFileStream = new OutJPGFileStream(pStreamManager, this , QIC_STREAM_JPGFILE);

  _pMP4Stream->Initialize();
  if (pAVCStream)
    _pMP4Stream->RegisterStream(reinterpret_cast<BaseInStream*>(pAVCStream));

  if (pMJPGInStream) {
    pMJPGInStream->Link(reinterpret_cast<BaseOutStream*>(_pJPGFileStream), true);
  }

  _firstDuration=5000;
  _secondDuration=_pushInfo.alarmDuration=
      stringToNum<int32_t>(SystemManager::GetNVRam(NVRAM_EVENT_FILE_DURATION));
  _maxVideoClips=stringToNum<uint32_t>(SystemManager::GetNVRam(NVRAM_EVENT_MAX_CLIP));
}


PushHandler::~PushHandler()
{
  BaseHardwareAbstractDevice *pGPIOInstance = reinterpret_cast<BaseHardwareAbstractDevice *>(HardwareManager::GetHardwareInstance(HT_GPIO));
  BaseHardwareAbstractDevice *pMicDevice = reinterpret_cast<BaseHardwareAbstractDevice *>(HardwareManager::GetHardwareInstance(HT_MIC));

  if (pGPIOInstance)
    pGPIOInstance->UnRegisterObserver(this);
  if (pMicDevice)
    pMicDevice->UnRegisterObserver(this);

  if (_pMP4Stream != NULL) {
    _pMP4Stream->UnLink(true);
    delete _pMP4Stream;
  }
  if (_pJPGFileStream) {
    _pJPGFileStream->UnLink(true);
    delete _pJPGFileStream;
  }
}

bool PushHandler::Initialize() {
  BaseHardwareAbstractDevice *pGPIOInstance = reinterpret_cast<BaseHardwareAbstractDevice *>(HardwareManager::GetHardwareInstance(HT_GPIO));
  BaseHardwareAbstractDevice *pMicDevice = reinterpret_cast<BaseHardwareAbstractDevice *>(HardwareManager::GetHardwareInstance(HT_MIC));

  _pushInfo.Reset();
  //Get NVRam setting
  _tmpPath = SystemManager::GetNVRam(NVRAM_EVENT_TMP_PATH)+"/";
  _targetPath = SystemManager::GetNVRam(NVRAM_EVENT_PATH)+"/";
  _dirMetaPath = SystemManager::GetNVRam(NVRAM_EVENT_DIRMETA_PATH)+"/";
  _minReqSize = stringToNum<uint64_t>(SystemManager::GetNVRam(NVRAM_EVENT_TMP_MINSIZE));

  if (pGPIOInstance) {
    INFO ("pushhandler class object registered to gpio");
    pGPIOInstance->RegisterObserver(this);
  }

  if (pMicDevice) {
    INFO ("pushhandler class object registered to mic");
    pMicDevice->RegisterObserver(this);
  }

  return true;
}

void PushHandler::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                uint32_t length) {
  if (msgId==_lastMSGId) {
    _lastMSGId=0;
  }
}



void PushHandler::EnableEvent() {
  _pushInfo.enabled=true;
}

void PushHandler::DisableEvent() {
  _pushInfo.enabled=false;
}

bool PushHandler::IsPushState() {
  return (_pushInfo.alarmState==ALERT_WRITING);
}

bool PushHandler::IsHardwareReady() {
  uint16_t errStatus=0;
  uint32_t errorCheck[] = {
    EC_CPU_BUSY,
    EC_OUT_OF_MEMORY,
    EC_OUT_OF_STORAGE,
    EC_STORAGE,
  };

  //1. check memory, storage
  if (!_pMemoryDevice->IsMemoryAvailable()) {
    errStatus=EC_OUT_OF_MEMORY;
    DEBUG ("out of memory");
  }
  else if (!_pStorageDevice->IsStorageAvailable(_targetPath, _minReqSize)) {
    errStatus=EC_OUT_OF_STORAGE;
    DEBUG ("out of storage");
  }

  //First time the error occurs, send event to cloud
  if (errStatus && (!HardwareManager::IsErrorStatus(errStatus)) ) {
    HardwareManager::SetStatus(errStatus, false);
    DEBUG ("errorStatus:%x", errStatus);
    return false;
  }

  //2. check all errors and send to cloud
  for (uint32_t i=0; i<sizeof(errorCheck)/sizeof(uint32_t); i++) {
    if (HardwareManager::IsErrorStatus(errorCheck[i])) {
      if (HardwareManager::GetSendStatus(errorCheck[i])) {
        BaseClientApplication *pApplication=
            const_cast<BaseClientApplication*>(_pMP4Stream->GetStreamsManager()->GetApplication());
        pApplication->OnNotifyEvent(CLOUD_MSG_ERROR, errorCheck[i], 0, " Ignore push video event.");
      }
      return false;
    }
  }

  if (_lastMSGId!=0) {
    INFO ("Storage is busy");
    return false;
  }
  return true;
}

bool PushHandler::CanTriggerAlarm() {
  bool ret=false;
  uint32_t wday, hour, min;

  //1.check pushvideo enabled
 // if ((!_pushInfo.enabled) || (!SystemManager::IsNTPSynced())) { //temp don't care NTP
  if (!_pushInfo.enabled) {
    DEBUG ("push is disabled %d", _pushInfo.enabled);
    return ret;
  }
  if (!SystemManager::IsNTPSynced())
  	WARN ("NTP is not synced. %d", SystemManager::IsNTPSynced());

  //2.an ignore alarm?
  getWDayHour(wday, hour, min);
  DEBUG ("day:%d, hour:%d, min:%d", wday, hour, min);
  if (!CheckScheduleStatus(wday, hour, NVRAM_EVENT_AUDIO_SCHEDULE) &&
      _pushInfo.eventType==EVT_AUDIO_ALARM) {
    INFO ("Ignore audio event by schedule");
    return ret;
  }
  if (!CheckScheduleStatus(wday, hour, NVRAM_EVENT_PIR_SCHEDULE) &&
      _pushInfo.eventType==EVT_PIR_ALARM) {
    INFO ("Ignore pir event by schedule");
    return ret;
  }

  //3.Check hardware status
  ret=IsHardwareReady();
  if (!ret) {
    DEBUG ("hardware is not ready");
  }

  ret = true; //Temp. workaround IsHardwareReady encountered problems. (icam_check_sdmmc)
  return ret;
}

void PushHandler::OnDeviceMessage(DeviceMessage &msg)
{
  string metaFile;

  if (!(msg.type==GPIO_PIRALRAM || msg.type==ACM_DETECTED)) {
    return;
  }
  if (!SystemManager::IsFactoryMode()) {
    if (!SystemManager::IsCloudRegistered()) {
      DEBUG ("cloud mode and not reigstered");
      return;
    }
    if (_pMP4Stream) {
      if (!_pMP4Stream->GetStreamsManager()->FindFirstByType(ST_OUT_WSNET_MSG)) {
        DEBUG ("cloud not connected");
        return;
      }
    }
  }
  if (_pushInfo.minorVideoNo >= _maxVideoClips) {
    DEBUG ("_videoClipsNo is over :%d", _pushInfo.minorVideoNo);
    return;
  }
  if (!CanTriggerAlarm()) {
    return ;
  }

  switch (_pushInfo.alarmState) {
    case (NO_ALERT):
      if (msg.type==ACM_DETECTED) {
        _pushInfo.eventType = EVT_AUDIO_ALARM;
      }
      else {
        _pushInfo.eventType = EVT_PIR_ALARM;
      }
      _pushInfo.eventId = SystemManager::GetEventId();
      metaFile=_dirMetaPath+_pushInfo.eventId+"."+
          numToString<uint16_t>(_pushInfo.eventType, 0);
      _tmpDirPath=numToString<uint16_t>(_pushInfo.minorVideoNo, 0)+"/";

      if (createFile(metaFile) && createFolder(_tmpPath+_tmpDirPath, true)) {
        _pushInfo.alarmState = ALERT_WAIT_TIME;
        //Capture an event jpg to server
        _pJPGFileStream->SnapShot(0,
                        _tmpDirPath+numToString<uint16_t>(_pushInfo.minorJPGNo, 0)+".jpg",
                        _pushInfo.eventId, numToString<uint16_t>(_pushInfo.eventType, 0));
        _pushInfo.minorJPGNo++;
      }
      PV_WARN ("=====PIR Alarm !!!!! ====  NO_ALERT=>ALERT_WAIT_TIME");
      break;
    case (ALERT_WAIT_TIME):
      break;
    case (ALERT_WRITE_DONE):
      break;
    case (ALERT_WRITING):
      break;
  }
  PV_DEBUG ("Alert state:%d", _pushInfo.alarmState);
}

bool PushHandler::OnStreamEvent(STREAM_EVENT_TYPE streamEvent,
                                Variant* info) {
  string cmd;
  uint32_t msgId=0;
  BaseAVCVideoCapture *pCamera =
      reinterpret_cast<BaseAVCVideoCapture*>(HardwareManager::GetHardwareInstance(HT_VIDEO_AVC));

  switch (streamEvent) {
   case (BUFFER_WRITE_DONE):
      if (_pushInfo.alarmState==ALERT_WRITING) {
        _pushInfo.alarmState = ALERT_WRITE_DONE;
        _pMP4Stream->StopRecord();
        if (_pMP4Stream) {
          if (_pushInfo.minorVideoNo) {
            _pMP4Stream->SetFileDuration(_firstDuration);
          }
          else {//
            _pMP4Stream->SetFileDuration(_secondDuration);
          }
        }
        info->SerializeToBin(cmd);
        msgId=UnixDomainSocketManager::SendCmdToInfoThread(cmd, INFO_OP_CMD, GetCallbackId());
        //TODO(Recardo): Stop capture jpg
        _pushInfo.minorVideoNo++;
        _tmpDirPath=numToString<uint16_t>(_pushInfo.minorVideoNo, 0)+"/";
        _pushInfo.minorJPGNo=0;
        if (_pushInfo.minorVideoNo>=_maxVideoClips) {
          _pushInfo.alarmTick=0;
          if (msgId>0) {
            _lastMSGId=msgId;
            DEBUG ("lastMSGId:%d, videoNo:%d", _lastMSGId, _pushInfo.minorVideoNo);
          }
        }
      }
      break;
   case (REQ_IFRAME):
      pCamera->GetKeyFrame();
      break;
   case (VIDEO_STOPPED):
      if (_pMP4Stream&&_pMP4Stream->IsRecording()) {
        _pushInfo.alarmState = ALERT_WRITE_DONE;
        _pMP4Stream->StopRecord();
      }
      break;
   case (VIDEO_STARTED):
      break;
   case (CAP_JPGFILE_FAILURE):
      WARN ("jpg capture failed");
      break;
   case (CAP_JPGFILE_SUCCESS):
      _pushInfo.minorJPGNo++;
      if (info->HasKey(INFO_SRC_FILE)) {
        if ((*info)[INFO_SRC_FILE]=="0/0.jpg") {
          info->SerializeToBin(cmd);
          UnixDomainSocketManager::SendCmdToInfoThread(cmd, INFO_OP_CMD, GetCallbackId());
        }
      }
      break;
    default:
      break;
  }
  return true;
}

bool PushHandler::CheckScheduleStatus(uint32_t wday, uint32_t hour, string nvramKey) {
  uint8_t index=wday*6+hour/4;
  uint8_t offset=hour%4;
  string scheduleMap=SystemManager::GetNVRam(nvramKey);
  uint8_t byteval=scheduleMap[index];

  byteval -= '0';
  if (byteval > 9) {
    byteval = byteval-('a'-'0')+10;
  }

  uint8_t bitval = byteval&(1<<(3-offset));
  if (bitval) {
    return true;
  }
  return false;
}

bool PushHandler::OnEvent(select_event &event)
{
  BaseAudioCaptureDevice *pDevice = reinterpret_cast<BaseAudioCaptureDevice*>(HardwareManager::GetHardwareInstance(HT_MIC));
  Variant msg;
  bool snapJPG=false;

  _pushInfo.alarmTick -= 1;
  _pushInfo.preDuration -= 1;

  if (!_pJPGFileStream->CheckJPGStatus()) {
    WARN ("capture jpg failed!!");
  }
  if (pDevice)
    pDevice->ResetAlarmMask();

  switch (_pushInfo.alarmState) {
    case NO_ALERT:
      if (_pMP4Stream) {
        _pMP4Stream->SetFileDuration(_firstDuration);
      }
      break;
    case ALERT_WAIT_TIME:
      if (_pushInfo.preDuration <= 0) {
          string videoFileName=VIDEO_FILE_NAME;
          evtfile_info_t evtInfo={ _pushInfo.eventType, _pushInfo.eventId,
                                   _tmpDirPath+videoFileName };

          if (_pMP4Stream->StartRecord(PUSHVIDEO_MAXFRAMES, evtInfo)) {
            _pushInfo.alarmState = ALERT_WRITING;
            PV_WARN ("=============== ALERT_WAIT_TIME=>ALERT_WRITING, startRecord ============");
          }
          else
            _pushInfo.alarmState=ALERT_WRITE_DONE;
      }
      snapJPG=(_pushInfo.minorVideoNo>0);
      break;
    case ALERT_WRITING:
      snapJPG=(_pushInfo.minorVideoNo>0);
      break;
    case ALERT_WRITE_DONE:
      if (_pushInfo.alarmTick<=0) {
        PV_DEBUG ("ALERT_WRITE_DONE=>NO_ALERT");
        _pushInfo.Reset();
      }
      else {
        string videoFileName=VIDEO_FILE_NAME;
        if (createFolder(_tmpPath+_tmpDirPath, true)) {
          evtfile_info_t evtInfo={_pushInfo.eventType, _pushInfo.eventId,
                                  _tmpDirPath+videoFileName};

          if (_pMP4Stream->StartRecord(PUSHVIDEO_MAXFRAMES, evtInfo)) {
            _pushInfo.alarmState = ALERT_WRITING;
            snapJPG=true;
            PV_WARN ("============== ALERT_WRITE_DONE=>ALERT_WRITING, startRecord ============");
          }
          else
            WARN ("start record fail");
        }
      }
      break;
  }

  if (snapJPG) {
    _pJPGFileStream->SnapShot(0,
                     _tmpDirPath+numToString<uint16_t>(_pushInfo.minorJPGNo, 0)+".jpg",
                     _pushInfo.eventId, numToString<uint16_t>(_pushInfo.eventType, 0));
    snapJPG=false;
  }
  return true;
}

void PushHandler::SetFileDuration(uint32_t fileDuration) {
  _secondDuration=fileDuration;
  string duration=numToString<uint32_t>(fileDuration, 0);
  SystemManager::SetNVRam(NVRAM_EVENT_FILE_DURATION, duration, false);
}

uint32_t PushHandler::GetFileDuration() {
  return _secondDuration;
}
