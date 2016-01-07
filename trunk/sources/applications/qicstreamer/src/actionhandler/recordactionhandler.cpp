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

#ifdef HAS_PROTOCOL_CLI

#include "actionhandler/recordactionhandler.h"
#include "clidefine.h"
#include "system/systemdefine.h"
#include "system/nvramdefine.h"
#include "application/baseclientapplication.h"
#include "hardwaremanager.h"
#include "video/baseavcvideocapture.h"
#include "storage/basestoragedevice.h"
#include "hardwaremanager.h"
#include "protocols/protocolmanager.h"
#include "protocols/unixdomainsocket/unixdomainsocketprotocol.h"
#include "protocols/unixdomainsocket/streaming/avcrecordstream.h"
#include "mux/manualmp4muxoutstream.h"
#include "mux/alarmmp4muxoutstream.h"
#include "mux/cvrmuxoutstream.h"

#define MANUAL_MAXFRAMES (30*60*60*1) //(frames/s)(s/min)*(min/h)*(hours)
using namespace app_qicstreamer;

//constructor
RecordActionHandler::RecordActionHandler(string version, BaseClientApplication *pApplication)
: BaseActionHandler(AT_RECORD, version, pApplication),
  _pMP4MuxOutStream(0),
  _pAVCCaptureInstance(NULL),
  _pAVCInStream(0),
  _pPCMInStream(0),
  _pCVRMuxStream(0),
  //_pMP3InStream(0),
  _maxFrames(MANUAL_MAXFRAMES)
{
  _pAVCCaptureInstance = reinterpret_cast<BaseAVCVideoCapture *>(HardwareManager::GetHardwareInstance(HT_VIDEO_AVC));
}

RecordActionHandler::~RecordActionHandler() {
  if(_pMP4MuxOutStream){
    delete _pMP4MuxOutStream;
    _pMP4MuxOutStream = NULL;
  }
  _actionMap.clear();
  /* streamers deleted by streamsmanager */
}

bool RecordActionHandler::Initialize() {
  _actionMap[QIC_CLI_RECORD_ACTION_GET_STATUS] = reinterpret_cast<Action>(&RecordActionHandler::GetStatus);
  _actionMap[QIC_CLI_RECORD_ACTION_GET_FILELIST] = reinterpret_cast<Action>(&RecordActionHandler::GetFilesList);
  _actionMap[QIC_CLI_RECORD_ACTION_SET_EVT_DURATION] = reinterpret_cast<Action>(&RecordActionHandler::SetEventFileDuration);
  _actionMap[QIC_CLI_RECORD_ACTION_GET_EVT_DURATION] = reinterpret_cast<Action>(&RecordActionHandler::GetEventFileDuration);
  //Test API
  _actionMap[QIC_CLI_RECORD_ACTION_CREATE_THREAD] = reinterpret_cast<Action>(&RecordActionHandler::CreateVideoRecordThread);
  _actionMap[QIC_CLI_RECORD_ACTION_DUMP_AVC] = reinterpret_cast<Action>(&RecordActionHandler::DumpAVCStream);

  // default setting
  if(_pApplication != NULL){
    Variant config = _pApplication->GetConfiguration();
    if(!config.HasKey("defaultConfig")) return true;

    if(false == ProcessDefaultSettings(config))
      return false;
  }
  // get avc streamer
  _pAVCInStream= reinterpret_cast<BaseInStream*>(_pApplication->GetStreamsManager()->FindFirstByTypeByName(ST_IN_AV_MUX, QIC_STREAM_MUX));
  if (!_pAVCInStream) {
    FATAL("AVC stream not found");
    return false;
  }
  //get pcm
  _pPCMInStream= reinterpret_cast<BaseInStream*>(_pApplication->GetStreamsManager()->FindFirstByTypeByName(ST_IN_AUDIO_PCM, QIC_STREAM_WAVE));
  if (!_pPCMInStream) {
    FATAL("PCM stream not found");
  }


  //Create and register mp4 mux stream
  if(_pMP4MuxOutStream == NULL){
     BaseStorageDevice* pStorageDevice = reinterpret_cast<BaseStorageDevice *>(HardwareManager::GetHardwareInstance(HT_STORAGE));
     if (pStorageDevice) {
       _pMP4MuxOutStream = new ManualMP4MuxOutStream(
           reinterpret_cast<BaseClientApplication *>(_pApplication)->GetStreamsManager(),
           QIC_STREAM_MP4FILE);
      _pMP4MuxOutStream->RegisterStream(reinterpret_cast<BaseInStream *>(_pAVCInStream));
      if (_pPCMInStream) {
        _pMP4MuxOutStream->RegisterStream(reinterpret_cast<BaseInStream *>(_pPCMInStream));
      }
     }
    _pAVCCaptureInstance->RegisterObserver((IDeviceObserver *)_pMP4MuxOutStream);
  }

  //CVR Stream
  _pCVRMuxStream = new CVRMuxOutStream(
      reinterpret_cast<BaseClientApplication *>(_pApplication)->GetStreamsManager(),
      QIC_STREAM_CVRFILE);
  _pCVRMuxStream->RegisterStream(reinterpret_cast<BaseInStream *>(_pAVCInStream));
  if (_pPCMInStream) {
    _pCVRMuxStream->RegisterStream(reinterpret_cast<BaseInStream *>(_pPCMInStream));
  }
  return true;
}
bool RecordActionHandler::IsValidAction(string action) {
  return MAP_HAS1(_actionMap, action);
}

RestHTTPCode RecordActionHandler::DoAction(BaseProtocol *pFrom,
                                           bool isRO,
                                           vector<string> &resource,
                                           Variant &parameters,
                                           Variant &message) {
  return HC_200_OK;
}

void RecordActionHandler::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                        uint32_t dataLength) {
  BaseActionHandler::OnCmdFinished(msgId, pData, dataLength);
}

/* manual control mux stream */
RestHTTPCode RecordActionHandler::StartRecord(Variant &params, Variant &message)
{
  uint8_t status;
  BaseStorageDevice* pStorageDevice =
      static_cast<BaseStorageDevice *>(HardwareManager::GetHardwareInstance(HT_STORAGE));

  if (!pStorageDevice) {
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
      (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_NO_STROAGE_DEVICE),
      "no storage device", message);
  }

  if (!_pMP4MuxOutStream){
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
      (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_NO_MANUAL_MUXSTREAM),
      "no mp4 stream", message);
  }

  //if (!_pMP4MuxOutStream->IsSpaceEnough()) {
  //  return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
  //      (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_NOT_ENOUGH_SPACE),
  //      "storage space is not enough", message);
  //}

  status = _pMP4MuxOutStream->GetStatus();
  if (_pMP4MuxOutStream->IsError()) {
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
        (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_MANUAL_MUX_ERR),
        "recorder in error status", message);
  }

  if (_pMP4MuxOutStream->IsRecording())
   return HC_200_OK;

#ifdef HAS_CODEC_AACSTREAM
  if( status&MP4MUX_REG_AUDIO ) {
    if(!_pAACInStream ){
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
        (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_NO_AUDIO_STREAM),
        "no aac stream", message);
    }
    _pMP4MuxOutStream->RegisterStream(reinterpret_cast<BaseInStream *>(_pAACInStream));
  }
#endif


  if ( !(status&MP4MUX_REG_VIDEO) ) {
    _pAVCCaptureInstance->RegisterObserver((IDeviceObserver *)_pMP4MuxOutStream);
    if (NULL==_pAVCInStream) {
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
        (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_NO_VIDEO_STREAM),
        "No avc stream", message);
    }
    _pMP4MuxOutStream->RegisterStream(reinterpret_cast<BaseInStream *>(_pAVCInStream)); // by this way, mp4 mux instance can operate with avc stream instance
  }

  string dir=SystemManager::GetNVRam(NVRAM_RECORD_MINSIZE);
  string fileName=numToString<uint64_t>((uint64_t)time(NULL), 0)+".mp4";
  if (false == _pMP4MuxOutStream->StartRecord(fileName, _maxFrames)){
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
      (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_START_FAIL),
      "Unable to start recording", message);
  }

  return HC_200_OK;
}

/* manaual control mux stream */
RestHTTPCode RecordActionHandler::StopRecord(Variant &params, Variant &message)
{
  uint8_t status;
  BaseStorageDevice* pStorageDevice = reinterpret_cast<BaseStorageDevice *>(HardwareManager::GetHardwareInstance(HT_STORAGE));

  if (!pStorageDevice) {
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
      (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_NO_STROAGE_DEVICE),
      "no storage device", message);
  }

  if(!_pMP4MuxOutStream){
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
      (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_NO_MANUAL_MUXSTREAM),
      "no mp4 stream", message);
  }

  status = _pMP4MuxOutStream->GetStatus();

#ifdef HAS_CODEC_AACSTREAM
  if( status&MP4MUX_REG_AUDIO ) {
    if( !_pAACInStream ){
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
        (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_NO_AUDIO_STREAM),
        "No aac stream", message);
    }
    _pAACInStream->UnLink(reinterpret_cast<BaseOutStream *>(_pMP4MuxOutStream), true);
    _pMP4MuxOutStream->UnRegisterStream(reinterpret_cast<BaseInStream *>(_pAACInStream));
  }
#endif

  if( status&MP4MUX_REG_VIDEO ) {
    _pAVCCaptureInstance->UnRegisterObserver((IDeviceObserver *)_pMP4MuxOutStream);

    if( !_pAVCInStream ) {
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
        (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_NO_VIDEO_STREAM),
        "No avc stream", message);
    }
    _pMP4MuxOutStream->UnRegisterStream(reinterpret_cast<BaseInStream *>(_pAVCInStream));

    if(false == _pMP4MuxOutStream->StopRecord()){
      return HC_500_INTERNAL_SERVER_ERROR;
    }
  }

  return HC_200_OK;
}

RestHTTPCode RecordActionHandler::GetStatus(Variant &params, Variant &message)
{
//  bool manual=false;
//  bool event=false;
//
//  if(_pMP4MuxOutStream){
//    manual = _pMP4MuxOutStream->IsRecording();
//  }
//
//  StreamsManager *pStreamsManager=_pApplication->GetStreamsManager();
//  if (pStreamsManager) {
//    AlarmMP4MuxOutStream *pAlarmMP4Stream=reinterpret_cast<AlarmMP4MuxOutStream*>
//        (pStreamsManager->FindFirstByTypeByName(ST_OUT_FILE_MP4, QIC_STREAM_MP4ALARMFILE));
//    if (pAlarmMP4Stream) {
//      event = pAlarmMP4Stream->IsRecording();
//    }
//    else
//      WARN ("can not find mp4 alarm stream");
//  }
//
//  message[API_PARAM_MANUAL] = manual;
//  message[API_PARAM_EVENT] = event;
  return HC_200_OK;
}


RestHTTPCode RecordActionHandler::SetEventFileDuration(Variant &params, Variant &message) {
  //if (!params.HasKey("duration")) {
  //  return SendFailure(HC_400_BAD_REQUEST,
  //    (RestErrorCode)(EC_104_INVALID_PARAMETERS),
  //    "no duration param", message);
  //}
  //double duration=stringToNum<double>((string)(params["duration"]));

  //if ((duration > 8000) || (duration < 3000)) {
  //  return SendFailure(HC_400_BAD_REQUEST,
  //    (RestErrorCode)(EC_104_INVALID_PARAMETERS),
  //    "duration out of range", message);
  //}

  //StreamsManager *pStreamsManager=_pApplication->GetStreamsManager();
  //if (pStreamsManager) {
  //  AlarmMP4MuxOutStream *pAlarmMP4Stream=reinterpret_cast<AlarmMP4MuxOutStream*>(pStreamsManager->FindFirstByTypeByName(ST_OUT_FILE_MP4, QIC_STREAM_MP4ALERTFILE));
  //  if (pAlarmMP4Stream) {
  //    pAlarmMP4Stream->SetFileDuration(duration);
  //  }
  //  else {
  //    return SendFailure(HC_400_BAD_REQUEST,
  //      (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_NO_EVENT_MUXSTREAM),
  //      "no event mux stream", message);
  //  }
  //}
  return HC_200_OK;
}

RestHTTPCode RecordActionHandler::GetEventFileDuration(Variant &params, Variant &message) {
  string duration;
  StreamsManager *pStreamsManager=_pApplication->GetStreamsManager();
  if (pStreamsManager) {
    AlarmMP4MuxOutStream *pAlarmMP4Stream=reinterpret_cast<AlarmMP4MuxOutStream*>
        (pStreamsManager->FindFirstByTypeByName(ST_OUT_FILE_MP4, QIC_STREAM_MP4ALARMFILE));
    if (pAlarmMP4Stream) {
      duration=numToString<double>(pAlarmMP4Stream->GetFileDuration(), 0);
    }
    else {
      return SendFailure(HC_400_BAD_REQUEST,
        (RestErrorCode)(EC_500_RECORD_ERROR+EC_RECORD_NO_EVENT_MUXSTREAM),
        "no event mux stream", message);
    }
    message["duration"] = duration;
  }

  return HC_200_OK;
}

RestHTTPCode RecordActionHandler::GetFilesList(Variant &params, Variant &message)
{
  return HC_206_PARTIAL_CONTENT;
}

RestHTTPCode RecordActionHandler::CreateVideoRecordThread(Variant &params, Variant &message) {
//  _pUXThread = new UXThreadTest();
//  _pUXThread->StartThread();
//  message["fileName"] = "avc.dump";
  return HC_200_OK;
}


RestHTTPCode RecordActionHandler::DumpAVCStream(Variant &params, Variant &message) {
//  if(!params.HasKey("protocolID")){
//    FATAL("Unable to find protocol id");
//    //ACTION_STATUS_ERROR(message, "Unable to find protocol ID");
//    return HC_500_INTERNAL_SERVER_ERROR;
//  }
//  uint32_t protocolID = params["protocolID"];
//  BaseProtocol *pProtocol = ProtocolManager::GetProtocol(protocolID);
//  BaseInStream *pInStream = NULL;
//  BaseOutStream *pOutStream = NULL;
//  string streamName = "avcstream";
//
//  if(pProtocol->GetType() != PT_UNIX_DOMAIN_SOCKET){
//    //ACTION_STATUS_ERROR(message, "Unable to support this protocol type");
//    return HC_500_INTERNAL_SERVER_ERROR;
//  }
//
//
//  pOutStream = (BaseOutStream *) new AVCRecordStream(reinterpret_cast<BaseProtocol *>(pProtocol),
//      _pApplication->GetStreamsManager(), md5(generateRandomString(8), true));
//
//  reinterpret_cast<UnixDomainSocketProtocol *>(pProtocol)->RegisterOutStream(pOutStream);
//  map<uint32_t, BaseStream *> inStreams = _pApplication->GetStreamsManager()->FindByTypeByName( ST_IN, streamName, true, true);
//
//  if (inStreams.size() != 1) { // Hardware related stream should be only one
//    FATAL("Stream %s not found", STR(streamName));
//    return HC_500_INTERNAL_SERVER_ERROR;
//  }
//  pInStream = (BaseInStream *) MAP_VAL(inStreams.begin());
//
//
//  if(pInStream == NULL){
//    FATAL("Unable to find instream %s", STR(streamName));
//    return HC_500_INTERNAL_SERVER_ERROR;
//  }
//  if(pOutStream == NULL){
//    FATAL("Unable to create outstream %s", STR(streamName));
//    return HC_500_INTERNAL_SERVER_ERROR;
//  }
//  if (!pInStream->Link(reinterpret_cast<BaseOutStream *>(pOutStream))) {
//    FATAL("Unable to link to the in stream");
//    return HC_500_INTERNAL_SERVER_ERROR;
//  }else{
//    INFO("link to instream %s", STR(streamName));
//  }

  return HC_200_OK;
}

// Private Functions
bool RecordActionHandler::ProcessDefaultSettings(Variant &config){
  return true;
}

#endif
