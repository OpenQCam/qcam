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


#include "actionhandler/audioactionhandler.h"
//thelib
#include "clidefine.h"
#include "application/baseclientapplication.h"
#include "protocols/avhttpstream/avstreamdefine.h"
#include "streaming/wsoutnetaudiostream.h"
#include "streaming/audiofilestream.h"
#include "streaming/streamstypes.h"

//hardware
#include "system/systemmanager.h"
#include "system/nvramdefine.h"
#include "hardwaremanager.h"
#include "audio/baseaudiocapturedevice.h"
#include "audio/speaker.h"
#include "audio/pcmplaystream.h"

using namespace app_qicstreamer;

AudioActionHandler::AudioActionHandler(string version, BaseClientApplication *pApplication)
: BaseActionHandler(AT_AUDIO, version, pApplication),
  _pMICInstance(0),
  _pSpeakerInstance(0)
{
  _filePath=SystemManager::GetNVRam(NVRAM_AUDIO_FILE_PATH);
}

AudioActionHandler::~AudioActionHandler() {

  (*_actionMaps[QIC_CLI_AUDIO_MIC])[true]->clear();
  (*_actionMaps[QIC_CLI_AUDIO_MIC])[false]->clear();
  (*_actionMaps[QIC_CLI_AUDIO_SPEAKER])[true]->clear();
  (*_actionMaps[QIC_CLI_AUDIO_SPEAKER])[false]->clear();
  _actionMaps[QIC_CLI_AUDIO_MIC]->clear();
  _actionMaps[QIC_CLI_AUDIO_SPEAKER]->clear();
  _actionMaps.clear();
}


bool AudioActionHandler::Initialize() {
  ActionMap *pMicGetActionMap = new ActionMap;
  ActionMap *pMicPostActionMap = new ActionMap;
  ActionMap *pSpeakerGetActionMap = new ActionMap;
  ActionMap *pSpeakerPostActionMap = new ActionMap;
  CollectionMap *pMicMap= new CollectionMap;
  CollectionMap *pSpeakerMap= new CollectionMap;

  bool enableMIC=stringToNum<bool>(SystemManager::GetNVRam(NVRAM_AUDIO_MIC_ENABLED));
  //Get audio hardware instance
  _pMICInstance = reinterpret_cast<BaseAudioCaptureDevice *>(HardwareManager::GetHardwareInstance(HT_MIC));
  _pSpeakerInstance = reinterpret_cast<Speaker *>(HardwareManager::GetHardwareInstance(HT_SPEAKER));
  if (_pSpeakerInstance) {
    _pSpeakerInstance->RegisterObserver((IDeviceObserver *)this);
  }
  if (_pMICInstance && enableMIC) {
    _pMICInstance->Start();
  }

  //construct map
  (*pMicMap)[true] = pMicGetActionMap;
  (*pMicMap)[false] = pMicPostActionMap;
  (*pSpeakerMap)[true] = pSpeakerGetActionMap;
  (*pSpeakerMap)[false] = pSpeakerPostActionMap;
  _actionMaps[QIC_CLI_AUDIO_MIC] = pMicMap;
  _actionMaps[QIC_CLI_AUDIO_SPEAKER] = pSpeakerMap;
  //audio mic function
  (*pMicGetActionMap)[QIC_CLI_AUDIO_MIC_STREAM] = reinterpret_cast<Action>(&AudioActionHandler::GetMicStream);
  (*pMicGetActionMap)[QIC_CLI_AUDIO_MIC_SAMPLE_RATE] = reinterpret_cast<Action>(&AudioActionHandler::GetMicSampleRate);
//  (*pMicGetActionMap)[QIC_CLI_AUDIO_MIC_DEVICE] = reinterpret_cast<Action>(&AudioActionHandler::GetMicDevice);

  (*pMicPostActionMap)[QIC_CLI_AUDIO_MIC_STREAM] = reinterpret_cast<Action>(&AudioActionHandler::PostMicStream);
  (*pMicPostActionMap)[QIC_CLI_AUDIO_MIC_SAMPLE_RATE] = reinterpret_cast<Action>(&AudioActionHandler::PostMicSampleRate);
//  (*pMicPostActionMap)[QIC_CLI_AUDIO_MIC_DEVICE] = reinterpret_cast<Action>(&AudioActionHandler::PostMicDevice);

  //audio speaker function
  (*pSpeakerGetActionMap)[QIC_CLI_AUDIO_SPEAKER_STREAM] = reinterpret_cast<Action>(&AudioActionHandler::GetSpeakerStream);
  (*pSpeakerGetActionMap)[QIC_CLI_AUDIO_SPEAKER_FILE] = reinterpret_cast<Action>(&AudioActionHandler::GetFileList);
  (*pSpeakerGetActionMap)[QIC_CLI_AUDIO_SPEAKER_VOLUME] = reinterpret_cast<Action>(&AudioActionHandler::GetSpeakerVolume);

  (*pSpeakerPostActionMap)[QIC_CLI_AUDIO_SPEAKER_STREAM] = reinterpret_cast<Action>(&AudioActionHandler::PostSpeakerStream);
  (*pSpeakerPostActionMap)[QIC_CLI_AUDIO_SPEAKER_PLAYBACK] = reinterpret_cast<Action>(&AudioActionHandler::PlayFile);
  (*pSpeakerPostActionMap)[QIC_CLI_AUDIO_SPEAKER_DEVICE] = reinterpret_cast<Action>(&AudioActionHandler::PostSpeakerDevice);
  (*pSpeakerPostActionMap)[QIC_CLI_AUDIO_SPEAKER_VOLUME] = reinterpret_cast<Action>(&AudioActionHandler::PostSpeakerVolume);

  return true;
}

bool AudioActionHandler::IsValidAction(string action) {
  return false;
}


bool AudioActionHandler::IsValidAction(map<string, CollectionMap*> *map,
                                         string collection,
                                         string id,
                                         bool isReadyOnly) {
  return true;
}

RestHTTPCode AudioActionHandler::DoAction(BaseProtocol *pFrom,
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

  if (payload.HasKey(REST_URI_STR)) {
    _uri=(string)payload[REST_URI_STR];
  }
  if (payload.HasKey(REST_TID_STR)) {
    _tid=(string)payload[REST_TID_STR];
  }
  Action pAction = (*(*_actionMaps[collection])[isRO])[id];
  return ((this->*pAction)(payload[REST_PARAM_STR], message));
}

void AudioActionHandler::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                       uint32_t dataLength) {
  NYI;
}

RestHTTPCode AudioActionHandler::GetMicStream(Variant &params, Variant &message) {
  if (_pMICInstance) {
    message["enabled"]=_pMICInstance->IsStart();
  }
  else {
    return SendFailure(HC_503_SERVICE_UNAVAILABLE,
                       EC_401_MIC_DEVICE_NOT_FOUND,
                       "Microphone device not found", message);
  }
  return HC_200_OK;
}

RestHTTPCode AudioActionHandler::PostMicStream(Variant &params, Variant &message) {
  if (!params.HasKey("enabled")) {
    return SendFailure(HC_400_BAD_REQUEST,
                       EC_109_MISSING_MANDATORY_PARAMETERS,
                       "Missing parameter: enabled", message);
  }
  if (params["enabled"]!=V_BOOL) {
    return SendFailure(HC_400_BAD_REQUEST,
                       EC_109_MISSING_MANDATORY_PARAMETERS,
                       "Invalid parameter type: enabled", message);
  }

  bool enabled=((bool)params["enabled"]);
  if (!_pMICInstance) {
    return SendFailure( HC_503_SERVICE_UNAVAILABLE,
                        EC_401_MIC_DEVICE_NOT_FOUND,
                        "Microphone device not found", message);
  }
  if (enabled) {
    _pMICInstance->Start();
  }
  else {
    _pMICInstance->Stop();
  }

  return HC_202_ACCEPTED;
}

RestHTTPCode AudioActionHandler::GetMicSampleRate(Variant &parameters, Variant &message) {
  DEBUG ("GetMicSampleRate");
  return HC_200_OK;
}

RestHTTPCode AudioActionHandler::PostMicSampleRate(Variant &parameters, Variant &message) {
  DEBUG ("PostMicSampleRate");
  return HC_200_OK;
}

//Speaker
RestHTTPCode AudioActionHandler::GetSpeakerStream(Variant &parameters, Variant &message) {
  if (_pSpeakerInstance) {
    message["enabled"]=_pSpeakerInstance->IsStart();
  }
  else {
    return SendFailure(HC_503_SERVICE_UNAVAILABLE,
                       EC_402_SPEAKER_DEVICE_NOT_FOUND,
                       "Speaker device not found", message);
  }
  return HC_200_OK;
}

RestHTTPCode AudioActionHandler::PostSpeakerStream(Variant &params, Variant &message) {
  if (!params.HasKey("enabled")) {
    return SendFailure(HC_400_BAD_REQUEST,
                       EC_109_MISSING_MANDATORY_PARAMETERS,
                       "Missing parameter: enabled", message);
  }

  bool enabled=((string)params["enabled"])=="true";
  if (!_pSpeakerInstance) {
    return SendFailure( HC_503_SERVICE_UNAVAILABLE,
                        EC_402_SPEAKER_DEVICE_NOT_FOUND,
                        "Speaker device not found", message);
  }
  if (enabled) {
    _pSpeakerInstance->Start();
  }
  else {
    _pSpeakerInstance->Stop();
  }

  return HC_202_ACCEPTED;
}

RestHTTPCode AudioActionHandler::PostSpeakerDevice(Variant &params, Variant &message) {
  if (!_pSpeakerInstance) {
    return SendFailure( HC_503_SERVICE_UNAVAILABLE,
                        EC_402_SPEAKER_DEVICE_NOT_FOUND,
                        "Speaker device not found", message);
  }
  if (params.HasKey("enabled")) {
    bool enabled=(string)params["enabled"]=="true";
    if (enabled) {
      _pSpeakerInstance->Open();
    }
    else {
      _pSpeakerInstance->Close();
    }
  }
  return HC_202_ACCEPTED;
}

RestHTTPCode AudioActionHandler::PlayFile(Variant &parameters, Variant &message) {

  if (!_pSpeakerInstance) {
    return SendFailure( HC_503_SERVICE_UNAVAILABLE,
                        EC_402_SPEAKER_DEVICE_NOT_FOUND,
                        "Speaker device not found", message);
  }
  if (!parameters.HasKey("filename")) {
    return SendFailure(HC_400_BAD_REQUEST,
        (RestErrorCode)(EC_109_MISSING_MANDATORY_PARAMETERS),
        "Missing parameter: filename", message);
  }

  string filename = (string)parameters["filename"];
  string fullpath=_filePath+"/"+filename;
  DEBUG ("filepath:%s", STR(fullpath));
  if (!fileExists(fullpath)) {
    return SendFailure(HC_400_BAD_REQUEST, EC_404_SPEAKER_FILE_NOT_FOUND,
        "File not exists", message);
  }

  if (_pSpeakerInstance->IsStart()) {
    return SendFailure(HC_400_BAD_REQUEST, EC_403_SPEAKER_PLAYING,
                       "Speaker is playing...", message);
  }

  if (!_pSpeakerInstance->LinkFileStream(fullpath)) {
    return SendFailure(HC_400_BAD_REQUEST, EC_406_SPEAKER_LINK_STREAM,
                       "Unable to link audio data", message);
  }

  if (!_pSpeakerInstance->Play()) {
    return SendFailure(HC_400_BAD_REQUEST, EC_405_SPEAKER_DEVICE_PLAY,
                       "Unable to play audio", message);
  }
  return HC_200_OK;
}

RestHTTPCode AudioActionHandler::GetFileList(Variant &parameters, Variant &message) {
  string dir=SystemManager::GetNVRam(NVRAM_AUDIO_FILE_PATH);
  string cmd="icam_get_audiofile \""+_uri+"\" "+_tid;
  SetSystemParams(message, cmd, false, NULL);
  return HC_204_NO_CONTENT;
}

RestHTTPCode AudioActionHandler::PostSpeakerVolume(Variant &params, Variant &message) {
  if (!_pSpeakerInstance) {
    return SendFailure( HC_503_SERVICE_UNAVAILABLE,
                        EC_402_SPEAKER_DEVICE_NOT_FOUND,
                        "Speaker device not found", message);
  }
  if (params.HasKey("volume")) {
    uint32_t volume = (uint32_t) atoi(STR(params["volume"]));
    _pSpeakerInstance->SetVolume(volume);
  }
  return HC_202_ACCEPTED;
}

RestHTTPCode AudioActionHandler::GetSpeakerVolume(Variant &params, Variant &message) {
  if (!_pSpeakerInstance) {
    return SendFailure( HC_503_SERVICE_UNAVAILABLE,
                        EC_402_SPEAKER_DEVICE_NOT_FOUND,
                        "Speaker device not found", message);
  }
  int32_t volume = _pSpeakerInstance->GetVolume();
  if (volume < 0) {
    return SendFailure( HC_503_SERVICE_UNAVAILABLE,
                        EC_402_SPEAKER_DEVICE_NOT_FOUND,
                        "Device can not get correct volume", message);
  }
  else {
    message["volume"] = volume;
    return HC_200_OK;
  }
}
