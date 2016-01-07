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
// // Author  : Recardo Cheng (recardo.cheng@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//
//


#include "actionhandler/eventactionhandler.h"
//thelib
#include "clidefine.h"
#include "protocols/avhttpstream/avstreamdefine.h"
#include "application/baseclientapplication.h"
#include "system/systemmanager.h"
#include "system/systemdefine.h"
#include "system/eventdefine.h"
#include "system/nvramdefine.h"
#include "streaming/streamstypes.h"
#include "streaming/inmsgstream.h"
//hardware
#include "hardwaremanager.h"
#include "pushmediator.h"
#include "storage/basestoragedevice.h"
#include "audio/baseaudiocapturedevice.h"

using namespace app_qicstreamer;

EventActionHandler::EventActionHandler(string version, BaseClientApplication *pApplication)
  : BaseActionHandler(AT_EVENT, version, pApplication),
    _pPushHandler(NULL) {
}

EventActionHandler::~EventActionHandler() {
  //iohandler manager will delete it
  _pPushHandler=NULL;

  (*_actionMaps[QIC_CLI_EVENT_AUDIO])[true]->clear();
  (*_actionMaps[QIC_CLI_EVENT_AUDIO])[false]->clear();
  (*_actionMaps[QIC_CLI_EVENT_PIR])[true]->clear();
  (*_actionMaps[QIC_CLI_EVENT_PIR])[false]->clear();
  (*_actionMaps[QIC_CLI_EVENT_RECORD])[true]->clear();
  (*_actionMaps[QIC_CLI_EVENT_RECORD])[false]->clear();
  (*_actionMaps[QIC_CLI_EVENT_STREAMER])[true]->clear();
  (*_actionMaps[QIC_CLI_EVENT_STREAMER])[false]->clear();
  _actionMaps[QIC_CLI_EVENT_AUDIO]->clear();
  _actionMaps[QIC_CLI_EVENT_PIR]->clear();
  _actionMaps[QIC_CLI_EVENT_RECORD]->clear();
  _actionMaps[QIC_CLI_EVENT_STREAMER]->clear();
  _actionMaps.clear();
}

bool EventActionHandler::Initialize() {
  string pushVideoEnabled=SystemManager::GetNVRam(NVRAM_EVENT_ENABLED);
  BaseVideoStream *pAVCInStream = reinterpret_cast<BaseVideoStream*>
     (_pApplication->GetStreamsManager()->FindFirstByTypeByName(ST_IN_AV_MUX, QIC_STREAM_MUX));
  BaseVideoStream *pMJPGInStream = reinterpret_cast<BaseVideoStream*>
     (_pApplication->GetStreamsManager()->FindFirstByTypeByName(ST_IN_CAM_MJPG, QIC_STREAM_MJPG));

  if (pAVCInStream && pMJPGInStream && !_pPushHandler) {
    _pPushHandler= new PushHandler(_pApplication->GetStreamsManager(),
                                   pAVCInStream,
                                   pMJPGInStream);
    _pPushHandler->Initialize();
    _pPushHandler->EnqueueForTimeEvent(1);
    if (!pushVideoEnabled.compare("1")) {
      _pPushHandler->EnableEvent();
    }
  }
  else
    FATAL ("pushhandler not created");

  //map construction
  ActionMap *pAudioGetActionMap = new ActionMap;
  ActionMap *pAudioPostActionMap = new ActionMap;
  ActionMap *pPIRGetActionMap = new ActionMap;
  ActionMap *pPIRPostActionMap = new ActionMap;
  ActionMap *pRecordGetActionMap = new ActionMap;
  ActionMap *pRecordPostActionMap = new ActionMap;
  ActionMap *pStreamerPostActionMap = new ActionMap;
  CollectionMap *pAudioMap= new CollectionMap;
  CollectionMap *pPIRMap= new CollectionMap;
  CollectionMap *pRecordMap= new CollectionMap;
  CollectionMap *pStreamerMap= new CollectionMap;
  (*pAudioMap)[true]=pAudioGetActionMap;
  (*pAudioMap)[false]=pAudioPostActionMap;
  (*pPIRMap)[true]=pPIRGetActionMap;
  (*pPIRMap)[false]=pPIRPostActionMap;
  (*pRecordMap)[true]=pRecordGetActionMap;
  (*pRecordMap)[false]=pRecordPostActionMap;
  (*pStreamerMap)[false]=pStreamerPostActionMap;
  _actionMaps[QIC_CLI_EVENT_AUDIO] = pAudioMap;
  _actionMaps[QIC_CLI_EVENT_PIR] = pPIRMap;
  _actionMaps[QIC_CLI_EVENT_RECORD] = pRecordMap;
  _actionMaps[QIC_CLI_EVENT_STREAMER] = pStreamerMap;

  //Audio Get/Post
  (*pAudioGetActionMap)[QIC_CLI_EVENT_AUDIO_ALARM] = reinterpret_cast<Action>(&EventActionHandler::GetAudioParams);
  (*pAudioGetActionMap)[QIC_CLI_EVENT_AUDIO_RMS] = reinterpret_cast<Action>(&EventActionHandler::GetAudioRMS);
  (*pAudioPostActionMap)[QIC_CLI_EVENT_AUDIO_ALARM] = reinterpret_cast<Action>(&EventActionHandler::PostAudioParams);
  (*pAudioPostActionMap)[QIC_CLI_EVENT_AUDIO_RMS] = reinterpret_cast<Action>(&EventActionHandler::PostAudioRMS);
  (*pAudioPostActionMap)[QIC_CLI_EVENT_AUDIO_TRIGGER] = reinterpret_cast<Action>(&EventActionHandler::PostAudioTrigger);
  //PIR Get/Post
  (*pPIRGetActionMap)[QIC_CLI_EVENT_PIR_ALARM] = reinterpret_cast<Action>(&EventActionHandler::GetPIRParams);
  (*pPIRPostActionMap)[QIC_CLI_EVENT_PIR_ALARM] = reinterpret_cast<Action>(&EventActionHandler::PostPIRParams);
  (*pPIRPostActionMap)[QIC_CLI_EVENT_PIR_TRIGGER] = reinterpret_cast<Action>(&EventActionHandler::PostPIRTrigger);
  //Record Get/Post
  (*pRecordGetActionMap)[QIC_CLI_EVENT_RECORD_DURATION] = reinterpret_cast<Action>(&EventActionHandler::GetFileDuration);
  (*pRecordPostActionMap)[QIC_CLI_EVENT_RECORD_DURATION] = reinterpret_cast<Action>(&EventActionHandler::PostFileDuration);

  (*pRecordGetActionMap)[QIC_CLI_EVENT_RECORD_RETENTION] = reinterpret_cast<Action>(&EventActionHandler::GetFileRetention);
  (*pRecordPostActionMap)[QIC_CLI_EVENT_RECORD_RETENTION] = reinterpret_cast<Action>(&EventActionHandler::PostFileRetention);
  //Streamer Post
  //(*pStreamerPostActionMap)[QIC_CLI_EVENT_STREAMER_SIGNAL] = reinterpret_cast<Action>(&EventActionHandler::PostStreamerSignal);
  return true;
}

bool EventActionHandler::IsValidAction(map<string, CollectionMap*> *map,
                                       string collection,
                                       string id,
                                       bool isReadOnly) {
  return true;
}

RestHTTPCode EventActionHandler::DoAction(BaseProtocol *pFrom,
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

RestHTTPCode EventActionHandler::GetAudioParams (Variant &params, Variant &message) {
  string schedule=SystemManager::GetNVRam(NVRAM_EVENT_AUDIO_SCHEDULE);
  message["schedule"]=schedule.substr(0, schedule.length()-1);
  message["enabled"]=(SystemManager::GetNVRam(NVRAM_AUDIO_ALARM_ENABLED)=="1");
  return HC_200_OK;
}

RestHTTPCode EventActionHandler::GetAudioRMS (Variant &params, Variant &message) {
  BaseAudioCaptureDevice *pAudioDevice=
    reinterpret_cast<BaseAudioCaptureDevice*>(HardwareManager::GetHardwareInstance(HT_MIC));

  if (!pAudioDevice) {
    return SendFailure(HC_503_SERVICE_UNAVAILABLE, EC_801_MIC_DEVICE_NOT_FOUND,
           "Audio device not found ", message);
  }

  message["enabled"]= pAudioDevice->IsRMSEnabled();
  message["value"]=stringToNum<int64_t>(SystemManager::GetNVRam(NVRAM_AUDIO_RMSTH));

  return HC_200_OK;
}

RestHTTPCode EventActionHandler::PostAudioParams (Variant &params, Variant &message) {
  if (params.HasKey("schedule")) {
    string schedule=(string)params["schedule"]+"_";
    SystemManager::SetNVRam(NVRAM_EVENT_AUDIO_SCHEDULE, schedule, false);
  }
  if (params.HasKey("enabled")) {
    BaseAudioCaptureDevice *pAudioDevice=
        reinterpret_cast<BaseAudioCaptureDevice*> (HardwareManager::GetHardwareInstance(HT_MIC));

    if (!pAudioDevice) {
      return SendFailure(HC_503_SERVICE_UNAVAILABLE, EC_801_MIC_DEVICE_NOT_FOUND,
           "Audio device not found ", message);
    }
    string enabled=((string)params["enabled"]=="true")?"1":"0";
    SystemManager::SetNVRam(NVRAM_AUDIO_ALARM_ENABLED, enabled, false);
    if (enabled.compare("1")==0) {
      pAudioDevice->EnableAlarm();
    }
    else {
      pAudioDevice->DisableAlarm();
    }
  }

  return HC_202_ACCEPTED;
}

RestHTTPCode EventActionHandler::PostAudioTrigger (Variant &params, Variant &message) {
  DEBUG ("AudioTrigger");
  return HC_200_OK;
}

RestHTTPCode EventActionHandler::GetPIRParams (Variant &params, Variant &message) {
  string schedule=SystemManager::GetNVRam(NVRAM_EVENT_PIR_SCHEDULE);
  message["schedule"]=schedule.substr(0, schedule.length()-1);
  message["enabled"]=(SystemManager::GetNVRam(NVRAM_IO_PIR_ENABLED)=="1");
  return HC_200_OK;
}

RestHTTPCode EventActionHandler::PostPIRParams (Variant &params, Variant &message) {
  if (params.HasKey("schedule")) {
    string schedule=(string)params["schedule"]+"_";
    SystemManager::SetNVRam(NVRAM_EVENT_PIR_SCHEDULE, schedule, false);
  }
  if (params.HasKey("enabled")) {
    BaseGPIODevice *pGPIO=
        reinterpret_cast<BaseGPIODevice*>(HardwareManager::GetHardwareInstance(HT_GPIO));
    if (!pGPIO) {
      return SendFailure(HC_503_SERVICE_UNAVAILABLE, EC_804_EVENT_IO_PIR_DEVICE_NOT_FOUND,
           "PIR device not found", message);
    }

    string enabled=((string)params["enabled"]=="true")?"1":"0";
    SystemManager::SetNVRam(NVRAM_IO_PIR_ENABLED, enabled, false);
    if (enabled.compare("1")==0) {
      pGPIO->EnablePIRNotification();
    }
    else {
      pGPIO->DisablePIRNotification();
    }
  }
  return HC_202_ACCEPTED;
}

RestHTTPCode EventActionHandler::PostAudioRMS (Variant &params, Variant &message) {
  BaseAudioCaptureDevice *pAudioDevice=
    reinterpret_cast<BaseAudioCaptureDevice*>(HardwareManager::GetHardwareInstance(HT_MIC));

  if (!pAudioDevice) {
    return SendFailure(HC_503_SERVICE_UNAVAILABLE, EC_801_MIC_DEVICE_NOT_FOUND,
           "Audio device not found ", message);
  }
  if (params.HasKey("enabled")) {
    bool enabled=(string)params["enabled"]=="true";
    if (enabled)
      pAudioDevice->EnableRMS();
    else
      pAudioDevice->DisableRMS();
  }
  if (params.HasKey("value")) {
    if (params["value"].IsNumeric()) {
      double rmsTh=(double)params["value"];
      string rmsText=numToString<double>(rmsTh, 0);
      pAudioDevice->SetAudioRMS(rmsTh);
      SystemManager::SetNVRam(NVRAM_AUDIO_RMSTH, rmsText, false);
    }
    else {
      return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
           "Value should be type of number ", message);
    }
  }

  return HC_202_ACCEPTED;
}

RestHTTPCode EventActionHandler::PostPIRTrigger (Variant &params, Variant &message) {
  DEBUG ("PIRTrigger");
  BaseGPIODevice *pGPIODevice=
      reinterpret_cast<BaseGPIODevice*>(HardwareManager::GetHardwareInstance(HT_GPIO));
  pGPIODevice->PIRAlarm();
  return HC_200_OK;
}

RestHTTPCode EventActionHandler::GetFileDuration (Variant &params, Variant &message) {
  if (!_pPushHandler) {
    return SendFailure(HC_503_SERVICE_UNAVAILABLE, EC_803_HANDLER_NOT_FOUND,
         "No push handler", message);
  }
  message["value"]=_pPushHandler->GetFileDuration();
  return HC_200_OK;
}

RestHTTPCode EventActionHandler::PostFileDuration (Variant &params, Variant &message) {
  if (!(params.HasKey("value") && params["value"]==V_INT64)) {
    return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
         "Invalid parameter - value", message);
  }
  if (!_pPushHandler) {
    return SendFailure(HC_503_SERVICE_UNAVAILABLE, EC_803_HANDLER_NOT_FOUND,
         "No push handler", message);
  }

  uint32_t duration=(uint32_t)params["value"];
  if (duration>20000 || duration<5000) {
    return SendFailure(HC_400_BAD_REQUEST, EC_802_OOR,
         "Parameter out of range - duration", message);
  }

  _pPushHandler->SetFileDuration(duration);
  return HC_202_ACCEPTED;
}

RestHTTPCode EventActionHandler::GetFileRetention (Variant &params, Variant &message) {
  message["time"]=SystemManager::GetNVRam(NVRAM_EVENT_RETENTION);
  return HC_200_OK;
}

RestHTTPCode EventActionHandler::PostFileRetention (Variant &params, Variant &message) {
  if (!(params.HasKey("time") && (params["time"]==V_INT64))) {
    return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
         "Invalid parameter - time", message);
  }

  uint64_t time=(int64_t)params["time"];
  if (time>86400 || time<0) {
    return SendFailure(HC_400_BAD_REQUEST, EC_802_OOR,
         "Parameter out of range - time", message);
  }
  SystemManager::SetNVRam(NVRAM_EVENT_RETENTION, params["time"], false);
  return HC_202_ACCEPTED;
}
