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

//thelib
#include "clidefine.h"
#include "system/nvramdefine.h"
#include "protocols/avhttpstream/avstreamdefine.h"
#include "streaming/streamstypes.h"

//hardware
#include "hardwaremanager.h"
#include "system/systemmanager.h"
#include "qic/qicmjpgvideocapture.h"
#include "video/baseavcvideocapture.h"
#include "video/basevideostream.h"
//jennifer
//#include "streaming/baseinstream.h"

//application
#include "application/baseclientapplication.h"
#include "actionhandler/videoactionhandler.h"
using namespace app_qicstreamer;

VideoActionHandler::VideoActionHandler(string version, BaseClientApplication *pApplication)
: BaseActionHandler(AT_VIDEO, version, pApplication)
{
}

VideoActionHandler::~VideoActionHandler() {

  (*_actionMaps[QIC_CLI_VIDEO_COMMON])[true]->clear();
  (*_actionMaps[QIC_CLI_VIDEO_COMMON])[false]->clear();
  (*_actionMaps[QIC_CLI_VIDEO_AVC])[true]->clear();
  (*_actionMaps[QIC_CLI_VIDEO_AVC])[false]->clear();
  (*_actionMaps[QIC_CLI_VIDEO_MJPG])[true]->clear();
  (*_actionMaps[QIC_CLI_VIDEO_MJPG])[false]->clear();
  _actionMaps[QIC_CLI_VIDEO_COMMON]->clear();
  _actionMaps[QIC_CLI_VIDEO_AVC]->clear();
  _actionMaps[QIC_CLI_VIDEO_MJPG]->clear();
  _actionMaps.clear();

}

bool VideoActionHandler::Initialize() {

  ActionMap *pAVCGetActionMap = new ActionMap;
  ActionMap *pAVCPostActionMap = new ActionMap;
  ActionMap *pMJPGGetActionMap = new ActionMap;
  ActionMap *pMJPGPostActionMap = new ActionMap;
  ActionMap *pCommonGetActionMap = new ActionMap;
  ActionMap *pCommonPostActionMap = new ActionMap;
  CollectionMap *pAVCMap= new CollectionMap;
  CollectionMap *pMJPGMap= new CollectionMap;
  CollectionMap *pCommonMap= new CollectionMap;


  _pMJPGCaptureInstance = reinterpret_cast<QICMJPGVideoCapture *>(HardwareManager::GetHardwareInstance(HT_VIDEO_MJPG));
  _pAVCCaptureInstance = reinterpret_cast<BaseAVCVideoCapture *>(HardwareManager::GetHardwareInstance(HT_VIDEO_AVC));

  if(_pMJPGCaptureInstance == NULL){
    FATAL("Unable to access MJPG video camera through hardware manager.");
    return false;
  }
  if(_pAVCCaptureInstance == NULL){
    FATAL("Unable to access AVC video camera through hardware manager.");
    return false;
  }

  map<uint32_t, BaseStream *> inStreams = _pApplication->GetStreamsManager()->FindByTypeByName(ST_IN_CAM_MJPG, QIC_STREAM_MJPG, true, true);
  if (inStreams.size() == 0) return false;
  _pMJPGInStream = (BaseVideoStream *) MAP_VAL(inStreams.begin());
  //jennifer : Can't get stream status in BaseInStream
  //_pMJPGInStream = (BaseInStream *) MAP_VAL(inStreams.begin());

  //construct map
  (*pMJPGMap)[true] = pMJPGGetActionMap;
  (*pMJPGMap)[false] = pMJPGPostActionMap;
  (*pAVCMap)[true] = pAVCGetActionMap;
  (*pAVCMap)[false] = pAVCPostActionMap;
  (*pCommonMap)[true] = pCommonGetActionMap;
  (*pCommonMap)[false] = pCommonPostActionMap;
  _actionMaps[QIC_CLI_VIDEO_MJPG] = pMJPGMap;
  _actionMaps[QIC_CLI_VIDEO_AVC] = pAVCMap;
  _actionMaps[QIC_CLI_VIDEO_COMMON] = pCommonMap;

  // action functions
  // MJPG Get
  (*pMJPGGetActionMap)[QIC_CLI_VIDEO_MJPG_CAPTURE] = reinterpret_cast<Action>(&VideoActionHandler::GetMJPGCaptureStatus);
  (*pMJPGGetActionMap)[QIC_CLI_VIDEO_MJPG_STREAM] = reinterpret_cast<Action>(&VideoActionHandler::GetMJPGStatus);
  (*pMJPGGetActionMap)[QIC_CLI_VIDEO_MJPG_RESOLUTION] = reinterpret_cast<Action>(&VideoActionHandler::GetMJPGResolution);
  // TODO : Controlling fps of mjpg NEED to be supported by firmware.
  //(*pMJPGGetActionMap)[QIC_CLI_VIDEO_MJPG_FPS] = reinterpret_cast<Action>(&VideoActionHandler::GetMJPGFrameRate);
  (*pMJPGGetActionMap)[QIC_CLI_VIDEO_MJPG_SETTING] = reinterpret_cast<Action>(&VideoActionHandler::GetAllMJPGSetting);
  (*pMJPGGetActionMap)[QIC_CLI_VIDEO_MJPG_DEFAULT_SETTING] = reinterpret_cast<Action>(&VideoActionHandler::GetAllMJPGDefaultSetting);
  (*pMJPGGetActionMap)[QIC_CLI_VIDEO_MJPG_CONSTRAINTS] = reinterpret_cast<Action>(&VideoActionHandler::GetMJPGSettingConstraints);
  // MJPG Post
  (*pMJPGPostActionMap)[QIC_CLI_VIDEO_MJPG_CAPTURE] = reinterpret_cast<Action>(&VideoActionHandler::StartAndStopMJPGVideoCapture);
  (*pMJPGPostActionMap)[QIC_CLI_VIDEO_MJPG_STREAM] = reinterpret_cast<Action>(&VideoActionHandler::ResumeAndPauseMJPGVideoCapture);
  (*pMJPGPostActionMap)[QIC_CLI_VIDEO_MJPG_RESOLUTION] = reinterpret_cast<Action>(&VideoActionHandler::SetMJPGResolution);
  // TODO : Controlling fps of mjpg NEED to be supported by firmware.
  //(*pMJPGPostActionMap)[QIC_CLI_VIDEO_MJPG_FPS] = reinterpret_cast<Action>(&VideoActionHandler::SetMJPGFrameRate);
  (*pMJPGPostActionMap)[QIC_CLI_VIDEO_MJPG_SETTING] = reinterpret_cast<Action>(&VideoActionHandler::SetAllMJPGSetting);
  (*pMJPGPostActionMap)[QIC_CLI_VIDEO_MJPG_DEFAULT_SETTING] = reinterpret_cast<Action>(&VideoActionHandler::SetAllMJPGDefaultSetting);

  // AVC Get
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_CAPTURE] = reinterpret_cast<Action>(&VideoActionHandler::GetAVCCaptureStatus);
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_STREAM] = reinterpret_cast<Action>(&VideoActionHandler::GetAVCStatus);
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_RESOLUTION] = reinterpret_cast<Action>(&VideoActionHandler::GetAVCResolution);
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_FPS] = reinterpret_cast<Action>(&VideoActionHandler::GetAVCFrameRate);
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_BITRATE] = reinterpret_cast<Action>(&VideoActionHandler::GetBitRate);
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_GOP] = reinterpret_cast<Action>(&VideoActionHandler::GetGOP);
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_SETTING] = reinterpret_cast<Action>(&VideoActionHandler::GetAllAVCSetting);
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_DEFAULT_SETTING] = reinterpret_cast<Action>(&VideoActionHandler::GetAllAVCDefaultSetting);
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_CONSTRAINTS] = reinterpret_cast<Action>(&VideoActionHandler::GetAVCSettingConstraints);
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_CAPABILITIES] = reinterpret_cast<Action>(&VideoActionHandler::GetAVCCapabilities);
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_OSD] = reinterpret_cast<Action>(&VideoActionHandler::GetAVCOSDInformation);
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_GENERATE_KEY_FRAME] = reinterpret_cast<Action>(&VideoActionHandler::GetKeyFrame);
  (*pAVCGetActionMap)[QIC_CLI_VIDEO_AVC_MMIO] = reinterpret_cast<Action>(&VideoActionHandler::GetRegisteryValue);
  // AVC Post
  (*pAVCPostActionMap)[QIC_CLI_VIDEO_AVC_CAPTURE] = reinterpret_cast<Action>(&VideoActionHandler::StartAndStopAVCVideoCapture);
  (*pAVCPostActionMap)[QIC_CLI_VIDEO_AVC_STREAM] = reinterpret_cast<Action>(&VideoActionHandler::ResumeAndPauseAVCVideoCapture);
  (*pAVCPostActionMap)[QIC_CLI_VIDEO_AVC_RESOLUTION] = reinterpret_cast<Action>(&VideoActionHandler::SetAVCResolution);
  (*pAVCPostActionMap)[QIC_CLI_VIDEO_AVC_FPS] = reinterpret_cast<Action>(&VideoActionHandler::SetAVCFrameRate);
  (*pAVCPostActionMap)[QIC_CLI_VIDEO_AVC_BITRATE] = reinterpret_cast<Action>(&VideoActionHandler::SetBitRate);
  (*pAVCPostActionMap)[QIC_CLI_VIDEO_AVC_GOP] = reinterpret_cast<Action>(&VideoActionHandler::SetGOP);
  (*pAVCPostActionMap)[QIC_CLI_VIDEO_AVC_SETTING] = reinterpret_cast<Action>(&VideoActionHandler::SetAllAVCSetting);
  (*pAVCPostActionMap)[QIC_CLI_VIDEO_AVC_DEFAULT_SETTING] = reinterpret_cast<Action>(&VideoActionHandler::SetAllAVCDefaultSetting);
  (*pAVCPostActionMap)[QIC_CLI_VIDEO_AVC_OSD] = reinterpret_cast<Action>(&VideoActionHandler::SetOSD);
  (*pAVCPostActionMap)[QIC_CLI_VIDEO_AVC_CLEAR_ALL_OSD] = reinterpret_cast<Action>(&VideoActionHandler::ClearAllOSD);

  // Common Get
  (*pCommonGetActionMap)[QIC_CLI_VIDEO_COMMON_BRIGHTNESS] = reinterpret_cast<Action>(&VideoActionHandler::GetBrightness);
  (*pCommonGetActionMap)[QIC_CLI_VIDEO_COMMON_CONTRAST] = reinterpret_cast<Action>(&VideoActionHandler::GetContrast);
  (*pCommonGetActionMap)[QIC_CLI_VIDEO_COMMON_HUE] = reinterpret_cast<Action>(&VideoActionHandler::GetHue);
  (*pCommonGetActionMap)[QIC_CLI_VIDEO_COMMON_SATURATION] = reinterpret_cast<Action>(&VideoActionHandler::GetSaturation);
  (*pCommonGetActionMap)[QIC_CLI_VIDEO_COMMON_SHARPNESS] = reinterpret_cast<Action>(&VideoActionHandler::GetSharpness);
  (*pCommonGetActionMap)[QIC_CLI_VIDEO_COMMON_GAMMA] = reinterpret_cast<Action>(&VideoActionHandler::GetGamma);
  (*pCommonGetActionMap)[QIC_CLI_VIDEO_COMMON_BACKLIGHT_COMPENSATION] = reinterpret_cast<Action>(&VideoActionHandler::GetBacklightCompensation);
  (*pCommonGetActionMap)[QIC_CLI_VIDEO_COMMON_FLIPMODE] = reinterpret_cast<Action>(&VideoActionHandler::GetFlipMode);
  (*pCommonGetActionMap)[QIC_CLI_VIDEO_COMMON_SETTING] = reinterpret_cast<Action>(&VideoActionHandler::GetAllCommonSetting);
  (*pCommonGetActionMap)[QIC_CLI_VIDEO_COMMON_CONSTRAINTS] = reinterpret_cast<Action>(&VideoActionHandler::GetCommonConstraints);
//#if ((defined(__x86__)) || (defined(__MT7620__)))
//jennifer
  (*pCommonGetActionMap)[QIC_CLI_VIDEO_COMMON_Y_VALUE] = reinterpret_cast<Action>(&VideoActionHandler::GetCommonYValue);
//#endif
  // Common Post
  (*pCommonPostActionMap)[QIC_CLI_VIDEO_COMMON_BRIGHTNESS] = reinterpret_cast<Action>(&VideoActionHandler::SetBrightness);
  (*pCommonPostActionMap)[QIC_CLI_VIDEO_COMMON_CONTRAST] = reinterpret_cast<Action>(&VideoActionHandler::SetContrast);
  (*pCommonPostActionMap)[QIC_CLI_VIDEO_COMMON_HUE] = reinterpret_cast<Action>(&VideoActionHandler::SetHue);
  (*pCommonPostActionMap)[QIC_CLI_VIDEO_COMMON_SATURATION] = reinterpret_cast<Action>(&VideoActionHandler::SetSaturation);
  (*pCommonPostActionMap)[QIC_CLI_VIDEO_COMMON_SHARPNESS] = reinterpret_cast<Action>(&VideoActionHandler::SetSharpness);
  (*pCommonPostActionMap)[QIC_CLI_VIDEO_COMMON_GAMMA] = reinterpret_cast<Action>(&VideoActionHandler::SetGamma);
  (*pCommonPostActionMap)[QIC_CLI_VIDEO_COMMON_BACKLIGHT_COMPENSATION] = reinterpret_cast<Action>(&VideoActionHandler::SetBacklightCompensation);
  (*pCommonPostActionMap)[QIC_CLI_VIDEO_COMMON_FLIPMODE] = reinterpret_cast<Action>(&VideoActionHandler::SetFlipMode);
  (*pCommonPostActionMap)[QIC_CLI_VIDEO_COMMON_SETTING] = reinterpret_cast<Action>(&VideoActionHandler::SetAllCommonSetting);
  (*pCommonPostActionMap)[QIC_CLI_VIDEO_COMMON_DEFAULT_SETTING] = reinterpret_cast<Action>(&VideoActionHandler::SetCommonToDefaultSetting);

  (*pCommonPostActionMap)[QIC_CLI_VIDEO_COMMON_FW_UPGRADE] = reinterpret_cast<Action>(&VideoActionHandler::SetCommonVideoFWUpgrade);

  return true;
}

// Public Functions
bool VideoActionHandler::IsValidAction(string action) {
  return true;
  //return MAP_HAS1(_actionMap, action);
}

RestHTTPCode VideoActionHandler::DoAction(BaseProtocol *pFrom,
                                          bool isRO,
                                          vector<string> &resource,
                                          Variant &payload,
                                          Variant &message) {
  string collection = resource[3];
  string id = resource[4];
  Action pAction;

  string AVMuxStreamName(QIC_STREAM_MUX);
  string RelayMuxStreamName(QIC_STREAM_RELAYMUX);
  map<uint32_t, BaseStream *> inStreams;

  if(collection.compare("avc") == 0 ){
    inStreams = _pApplication->GetStreamsManager()->FindByTypeByName(ST_IN_AV_MUX, AVMuxStreamName, true, true);
    if (inStreams.size() == 0)
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_701_STREAM_NOT_FOUND, format("Unable to find avc stream with name %s", STR(AVMuxStreamName)), message);

    _pAVMuxInStream = (BaseVideoStream *) MAP_VAL(inStreams.begin());

    inStreams = _pApplication->GetStreamsManager()->FindByTypeByName(ST_IN_AV_RELAYMUX, RelayMuxStreamName, true, true);
      if (inStreams.size() == 0)
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_701_STREAM_NOT_FOUND, format("Unable to find avc stream with name %s", STR(RelayMuxStreamName)), message);

    _pRelayMuxInStream = (BaseVideoStream *) MAP_VAL(inStreams.begin());

    if(!_pAVCCaptureInstance->SelectDeviceStream(AVMuxStreamName)){
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Selecting device stream failed", message);
    }
  }
  else if(_pAVCCaptureInstance->IsSimulcastSupported()){
    if(collection.compare("avc1") == 0 ){
      AVMuxStreamName.append("1");
      if(_pAVMuxInStream->GetName() != AVMuxStreamName){
        inStreams = _pApplication->GetStreamsManager()->FindByTypeByName(ST_IN_AV_MUX, AVMuxStreamName, true, true);
        if (inStreams.size() == 0)
          return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_701_STREAM_NOT_FOUND, format("Unable to find avc stream with name %s", STR(AVMuxStreamName)), message);

        _pAVMuxInStream = (BaseVideoStream *) MAP_VAL(inStreams.begin());

        inStreams = _pApplication->GetStreamsManager()->FindByTypeByName(ST_IN_AV_RELAYMUX, RelayMuxStreamName, true, true);
        if (inStreams.size() == 0)
          return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_701_STREAM_NOT_FOUND, format("Unable to find avc stream with name %s", STR(RelayMuxStreamName)), message);

        _pRelayMuxInStream = (BaseVideoStream *) MAP_VAL(inStreams.begin());
      }

      if(!_pAVCCaptureInstance->SelectDeviceStream(AVMuxStreamName)){
        return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Selecting device stream failed", message);
      }
      collection = "avc";
    }
    else if(collection.compare("avc2") == 0 ){
      AVMuxStreamName.append("2");
      if(_pAVMuxInStream->GetName() != AVMuxStreamName){
        inStreams = _pApplication->GetStreamsManager()->FindByTypeByName(ST_IN_AV_MUX, AVMuxStreamName, true, true);
        if (inStreams.size() == 0)
          return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_701_STREAM_NOT_FOUND, format("Unable to find avc stream with name %s", STR(AVMuxStreamName)), message);

        _pAVMuxInStream = (BaseVideoStream *) MAP_VAL(inStreams.begin());

        inStreams = _pApplication->GetStreamsManager()->FindByTypeByName(ST_IN_AV_RELAYMUX, RelayMuxStreamName, true, true);
        if (inStreams.size() == 0)
          return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_701_STREAM_NOT_FOUND, format("Unable to find avc stream with name %s", STR(RelayMuxStreamName)), message);

        _pRelayMuxInStream = (BaseVideoStream *) MAP_VAL(inStreams.begin());
      }

      if(!_pAVCCaptureInstance->SelectDeviceStream(AVMuxStreamName)){
        return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Selecting device stream failed", message);
      }
      collection = "avc";
    }
    else if(collection.compare("avc3") == 0 ){
      AVMuxStreamName.append("3");
      if(_pAVMuxInStream->GetName() != AVMuxStreamName){
        inStreams = _pApplication->GetStreamsManager()->FindByTypeByName(ST_IN_AV_MUX, AVMuxStreamName, true, true);
        if (inStreams.size() == 0)
          return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_701_STREAM_NOT_FOUND, format("Unable to find avc stream with name %s", STR(AVMuxStreamName)), message);

        _pAVMuxInStream = (BaseVideoStream *) MAP_VAL(inStreams.begin());

        inStreams = _pApplication->GetStreamsManager()->FindByTypeByName(ST_IN_AV_RELAYMUX, RelayMuxStreamName, true, true);
        if (inStreams.size() == 0)
          return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_701_STREAM_NOT_FOUND, format("Unable to find avc stream with name %s", STR(RelayMuxStreamName)), message);

        _pRelayMuxInStream = (BaseVideoStream *) MAP_VAL(inStreams.begin());
      }

      if(!_pAVCCaptureInstance->SelectDeviceStream(AVMuxStreamName)){
        return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Selecting device stream failed", message);
      }
      collection = "avc";
    }
  }// support simulcast

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

  if(_pMJPGCaptureInstance == NULL)
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_301_VIDEO_DEVICE_NOT_FOUND, "No AVC video device", message);

  if(_pAVCCaptureInstance == NULL)
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_301_VIDEO_DEVICE_NOT_FOUND, "No MJPG video device", message);

  pAction = (*(*_actionMaps[collection])[isRO])[id];
  return ((this->*pAction)(payload[REST_PARAM_STR], message));
}

// Private Functions
bool VideoActionHandler::IsValidMJPGAction(string action) {
  return true;
  //return MAP_HAS1(_mjpgActionMap, action);
}

bool VideoActionHandler::IsValidAVCAction(string action) {
  return true;
  //return MAP_HAS1(_avcActionMap, action);
}

// Actions
RestHTTPCode VideoActionHandler::StartAndStopMJPGVideoCapture(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("enabled"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Parameter \'enabled\' is required", message);

  string enabled = (string) parameters["enabled"];
  if( (enabled.compare("true")==0) || (enabled.compare("1")==0) ){
    if(!_pMJPGCaptureInstance->StartCapture())
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_302_VIDEO_CAPTURE_CONTROL_ERROR, "Start mjpg video failed!", message);
  }
  else if( (enabled.compare("false")==0) || (enabled.compare("0")==0) ){
    if(!_pMJPGCaptureInstance->StopCapture())
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_302_VIDEO_CAPTURE_CONTROL_ERROR, "Stop mjpg video failed!", message);

    // Because avc stream register to mjpg video caputure as a observer
    // ( when initializing media stream in qicstreamerapplication)
    // Stop capturing mjpg will ALSO make AVCStream receive a VCM_STOPPED message
    // That will update video stream status as VSS_STOP.
    // This will result in NO avc video streaming out and we don't want that.
    if(!_pAVMuxInStream->Resume())
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_303_VIDEO_STREAM_CONTROL_ERROR, "Resume avc video failed!", message);
  }else{
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Boolean parameters are expected.", message);
  }

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetMJPGCaptureStatus(Variant &parameters, Variant &message) {
  message["enabled"] = _pMJPGCaptureInstance->IsStarted();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::ResumeAndPauseMJPGVideoCapture(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("enabled"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Parameter \'enabled\' is required", message);

  string enabled = (string) parameters["enabled"];
  if( (enabled.compare("true")==0) || (enabled.compare("1")==0) ){
    if(!_pMJPGInStream->Resume())
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_303_VIDEO_STREAM_CONTROL_ERROR, "Streaming MJPG video failed!", message);
  }
  else if( (enabled.compare("false")==0) || (enabled.compare("0")==0) ){
    if(!_pMJPGInStream->Pause())
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_303_VIDEO_STREAM_CONTROL_ERROR, "Suspending MJPG video failed!", message);
  }else{
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Boolean parameters are expected.", message);
  }
  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetMJPGStatus(Variant &parameters, Variant &message) {
  message["enabled"] = (VSS_START == _pMJPGInStream->GetStatus());
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::GetMJPGResolution(Variant &parameters, Variant &message) {
  message["width"] = _pMJPGCaptureInstance->GetWidth();
  message["height"] = _pMJPGCaptureInstance->GetHeight();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetMJPGResolution(Variant &parameters, Variant &message) {
  // Check key existence
  if(!parameters.HasKey("width") || !parameters.HasKey("height"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Missing parameter width or height", message);

  // Check type
  if((parameters["height"] != _V_NUMERIC) || (parameters["width"] != _V_NUMERIC))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS,"Number type is expected", message);

  uint16_t width = (uint16_t) parameters["width"];
  uint16_t height = (uint16_t) parameters["height"];
  if(!_pMJPGCaptureInstance->VerifyResolution(width,height))
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! width : %d, height : %d", width, height), message);

  if(!_pMJPGCaptureInstance->SetResolution(width, height))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set mjpg resolution failed! width %d, height %d", width, height), message);

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetMJPGFrameRate(Variant &parameters, Variant &message) {
  message["fps"] = _pMJPGCaptureInstance->GetFrameRate();
  return HC_200_OK;
}

// TODO : Ask firmware team to support this on qic1832?
RestHTTPCode VideoActionHandler::SetMJPGFrameRate(Variant &parameters, Variant &message) {
  // Check key existence
  if(!parameters.HasKey("value"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Frame Interval is undefined", message);

  // Check type
  if(parameters.HasKey("value") != _V_NUMERIC)
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS,"Number type is expected", message);

  int32_t fps = (int32_t) parameters["value"];
  if(!_pMJPGCaptureInstance->SetFrameRate(fps))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,format("Set frame rate failed! %d", fps), message);

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetAllMJPGSetting(Variant &parameters, Variant &message) {
  Variant resolution;
  resolution["width"] = _pMJPGCaptureInstance->GetWidth();
  resolution["height"] = _pMJPGCaptureInstance->GetHeight();
  message["resolution"] = resolution;
  // TODO : Controlling fps of mjpg NEED to be supported by firmware.
  //message["fps"] = _pMJPGCaptureInstance->GetFrameRate();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetAllMJPGSetting(Variant &parameters, Variant &message) {
  bool isSync = false;

  // Check key existence - option here
  if(parameters.HasKey("resolution")){
    if(parameters["resolution"] != V_MAP)
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS,"JSON object type is expected", message);

    Variant resolution = parameters["resolution"];
    // Check key existence
    if(!resolution.HasKey("width") || !resolution.HasKey("height"))
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Missing parameter width or height", message);

    // Check type
    if((resolution["height"] != _V_NUMERIC) || (resolution["width"] != _V_NUMERIC))
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS,"Number type is expected", message);

    uint16_t width = (uint16_t) resolution["width"];
    uint16_t height = (uint16_t) resolution["height"];
    if(!_pMJPGCaptureInstance->VerifyResolution(width,height))
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! width : %d, height : %d", width, height), message);

    if(!_pMJPGCaptureInstance->SetResolution(width, height))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set mjpg resolution failed! width %d, height %d", width, height), message);

    // Inform client to synchronize setting
    isSync = true;
  }

  // TODO : Controlling fps of mjpg NEED to be supported by firmware.
  //if(parameters.HasKey("fps") ){
  //  string fps = (string) parameters["fps"];
  //  if(isNumeric(fps)){
  //    if(!_pMJPGCaptureInstance->SetFrameRate((int32_t)atoi(STR(fps))))
  //      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,format("Set frame rate failed! %s", STR(fps)), message);
  //      // Inform client to synchronize setting
  //      isSync = true;
  //  }else{
  //    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Numeric parameters are expected.", message);
  //  }
  //}
  if(isSync)
    return HC_202_ACCEPTED;
  else
    return HC_200_OK;
}

RestHTTPCode VideoActionHandler::GetAllMJPGDefaultSetting(Variant &parameters, Variant &message) {
  message["resolution"] = _pMJPGCaptureInstance->GetDefaultResolution();
  // TODO :
  // 1. add frame rate field in NVRAM;
  // 2. Add method to get default frame rate in MJPG Capture;
  // 3. Complete this method
  // 4. Test this method
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetAllMJPGDefaultSetting(Variant &parameters, Variant &message) {
  bool isSync = false;

  // Check key existence - option here
  if(parameters.HasKey("resolution")){
    if(parameters["resolution"] != V_MAP)
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS,"JSON object type is expected", message);

    Variant resolution = parameters["resolution"];
    // Check key existence
    if(!resolution.HasKey("width") || !resolution.HasKey("height"))
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Missing parameter width or height", message);

    // Check type
    if((resolution["height"] != _V_NUMERIC) || (resolution["width"] != _V_NUMERIC))
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS,"Number type is expected", message);

    uint16_t widthNo = (uint16_t) resolution["width"];
    uint16_t heightNo = (uint16_t) resolution["height"];
    if(!_pMJPGCaptureInstance->VerifyResolution(widthNo,heightNo))
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! width : %d, height : %d", widthNo, heightNo), message);

    string width = (string) resolution["width"];
    string height = (string) resolution["height"];
    if(!_pMJPGCaptureInstance->SetDefaultResolution(width, height))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_309_NVRAM_ACCESS_ERROR, format("Setting MJPG resolution : %sx%s as default failed", STR(width), STR(height)), message);

    isSync = true;
  }// resolution

  // TODO : Controlling fps of mjpg NEED to be supported by firmware.
  //if(parameters.HasKey("fps")){
  //  string fps = (string) parameters["fps"];
  //  if(isNumeric(fps)){
  //    if(!_pMJPGCaptureInstance->SetDefaultFrameRate(fps))
  //      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_309_NVRAM_ACCESS_ERROR,format("Setting framerate %s as default failed!", STR(fps)), message);
  //      // Inform client to synchronize setting
  //      isSync = true;
  //  }else{
  //    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Numeric parameters are expected", message);
  //  }
  //}// fps
  if(isSync)
    return HC_202_ACCEPTED;
  else
    return HC_200_OK;
}

RestHTTPCode VideoActionHandler::GetMJPGSettingConstraints(Variant &parameters, Variant &message) {
  message = _pMJPGCaptureInstance->GetConstraints();
  return HC_200_OK;
}

/*
 * TODO
 */
RestHTTPCode VideoActionHandler::GetKeyFrame(Variant &parameters, Variant &message) {
  if(!_pAVCCaptureInstance->GetKeyFrame()){
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,"Requesting key frame error !", message);
  }
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::GetRegisteryValue(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("address")){
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Need a key-value pair to specify register address", message);
  }

  string address = parameters["address"];
  unsigned int value = 0;
  std::stringstream str;
  unsigned int nAddress;
  //FATAL("Target address to be read is : %s with length: %d", STR(address), address.length());
  str << address;
  str >> std::hex >> nAddress;
  //FATAL("Target address to be read is : %x", nAddress);
  if(!_pAVCCaptureInstance->ReadMMIO(nAddress, &value))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,format("Read register address failed!%s", STR(address)), message);

  message["value"] = value;

  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::GetAVCCaptureStatus(Variant &parameters, Variant &message) {
  message["enabled"] = _pAVCCaptureInstance->IsStarted();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::StartAndStopAVCVideoCapture(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("enabled"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Parameter \'enabled\' is required", message);

  string enabled = (string) parameters["enabled"];
  if( (enabled.compare("true")==0) || (enabled.compare("1")==0) ){
      if(!_pAVCCaptureInstance->StartCapture())
        return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_302_VIDEO_CAPTURE_CONTROL_ERROR,"Start AVC video failed!", message);
  }
  else if( (enabled.compare("false")==0) || (enabled.compare("0")==0) ){
      if(!_pAVCCaptureInstance->StopCapture())
        return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_302_VIDEO_CAPTURE_CONTROL_ERROR,"Stop AVC video failed!", message);
  }else{
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Boolean parameters are expected.", message);
  }

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetAVCStatus(Variant &parameters, Variant &message) {
  message["enabled"] = (VSS_START == _pAVMuxInStream->GetStatus());
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::ResumeAndPauseAVCVideoCapture(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("enabled"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Parameter \'enabled\' is required", message);

  string enabled = (string) parameters["enabled"];
  if( (enabled.compare("true")==0) || (enabled.compare("1")==0) ){
      if(!_pAVMuxInStream->Resume())
        return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_303_VIDEO_STREAM_CONTROL_ERROR,"Resume AVC video failed!", message);
      if(!_pRelayMuxInStream->Resume())
        return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_303_VIDEO_STREAM_CONTROL_ERROR,"Resume AVC video failed!", message);
  }
  else if( (enabled.compare("false")==0) || (enabled.compare("0")==0) ){
      if(!_pAVMuxInStream->Pause())
        return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_303_VIDEO_STREAM_CONTROL_ERROR,"Pause AVC video failed!", message);
      if(!_pRelayMuxInStream->Pause())
        return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_303_VIDEO_STREAM_CONTROL_ERROR,"Pause AVC video failed!", message);
  }else{
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Boolean parameters are expected.", message);
  }

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetAVCResolution(Variant &parameters, Variant &message) {
  message["width"] = _pAVCCaptureInstance->GetWidth();
  message["height"] = _pAVCCaptureInstance->GetHeight();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetAVCResolution(Variant &parameters, Variant &message) {
  // Check key existence
  if(!parameters.HasKey("width") || !parameters.HasKey("height"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Missing parameter width or height", message);

  // Check type
  if((parameters["height"] != _V_NUMERIC) || (parameters["width"] != _V_NUMERIC))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS,"Number type is expected", message);

  uint16_t width = (uint16_t) parameters["width"];
  uint16_t height = (uint16_t) parameters["height"];
  if(!_pAVCCaptureInstance->VerifyResolution(width,height))
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! width : %d, height : %d", width, height), message);

  if(!_pAVCCaptureInstance->SetResolution(width, height))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set mjpg resolution failed! width %d, height %d", width, height), message);

  return HC_202_ACCEPTED;
}


RestHTTPCode VideoActionHandler::GetAVCFrameRate(Variant &parameters, Variant &message) {
  message["value"] = _pAVCCaptureInstance->GetFrameRate();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetAVCFrameRate(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("value"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Need a key-value pair to specify frame rate value", message);

  if(parameters["value"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Numeric parameters is expected", message);

  uint32_t fps = parameters["value"];
  if(!_pAVCCaptureInstance->VerifyFPS(fps))
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! frame rate : %d", fps), message);

  if(!_pAVCCaptureInstance->SetFrameRate(fps))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,format("Set frame interval failed! %d", fps), message);

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetBitRate(Variant &parameters, Variant &message) {
  message["value"] = _pAVCCaptureInstance->GetBitRate();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetBitRate(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("value"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Need a key-value pair to specify frame rate value", message);

  if(parameters["value"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Numeric parameters is expected", message);

  int32_t bitrate = parameters["value"];
  if(!_pAVCCaptureInstance->VerifyBitrate(bitrate))
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! bitrate : %d", bitrate), message);

  if(!_pAVCCaptureInstance->SetBitRate(bitrate))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,format("Set bitrate failed! %d", bitrate), message);

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetGOP(Variant &parameters, Variant &message) {
  message["value"] = _pAVCCaptureInstance->GetGOP();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetGOP(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("value"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Need a key-value pair to specify frame rate value", message);

  if(parameters["value"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Numeric parameters is expected", message);

  int32_t gop = parameters["value"];
  if(!_pAVCCaptureInstance->VerifyGOP(gop))
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! GOP : %d", gop), message);

  if(!_pAVCCaptureInstance->SetGOP(gop))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,format("Set GOP failed! %d", gop), message);

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetAllAVCSetting(Variant &parameters, Variant &message) {
  Variant resolution;
  resolution["width"] = _pAVCCaptureInstance->GetWidth();
  resolution["height"] = _pAVCCaptureInstance->GetHeight();
  message["resolution"] = resolution;
  message["bitrate"] = _pAVCCaptureInstance->GetBitRate();
  message["fps"] = _pAVCCaptureInstance->GetFrameRate();
  message["gop"] = _pAVCCaptureInstance->GetGOP();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetAllAVCSetting(Variant &parameters, Variant &message) {
  bool isSync = false;
  if(parameters.HasKey("resolution")){
    Variant resolution = parameters["resolution"];

    // Check key existence
    if(!resolution.HasKey("width") || !resolution.HasKey("height"))
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Missing parameter width or height", message);

    // Check type
    if((resolution["height"] != _V_NUMERIC) || (resolution["width"] != _V_NUMERIC))
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS,"Parameter of number type are expected for width and height", message);

    uint16_t width = (uint16_t) resolution["width"];
    uint16_t height = (uint16_t) resolution["height"];
    if(!_pAVCCaptureInstance->VerifyResolution(width,height))
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! width : %d, height : %d", width, height), message);

    if(!_pAVCCaptureInstance->SetResolution(width, height))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set mjpg resolution failed! width %d, height %d", width, height), message);
    isSync = true;
  }// resolution

  if(parameters.HasKey("fps")){
    if(parameters["fps"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Numeric parameters is expected for framerate", message);

    uint32_t fps = parameters["fps"];

    if(!_pAVCCaptureInstance->VerifyFPS(fps))
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! frame rate : %d", fps), message);

    if(!_pAVCCaptureInstance->SetFrameRate(fps))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,format("Set framerate failed! %d", fps), message);
    // Inform client to synchronize setting
    isSync = true;
  }// fps

  if(parameters.HasKey("bitrate")){
    if(parameters["bitrate"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Numeric parameters is expected for bitrate", message);

    uint32_t bitrate = parameters["bitrate"];

    if(!_pAVCCaptureInstance->VerifyBitrate(bitrate))
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! bitrate : %d", bitrate), message);

    if(!_pAVCCaptureInstance->SetBitRate(bitrate))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,format("Set bitrate failed! %d", bitrate), message);
    // Inform client to synchronize setting
    isSync = true;
  }// bitrate

  if(parameters.HasKey("gop")){
    if(parameters["gop"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Numeric parameters is expected for GOP", message);

    uint32_t gop = parameters["gop"];

    if(!_pAVCCaptureInstance->VerifyGOP(gop))
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! GOP : %d", gop), message);

    if(!_pAVCCaptureInstance->SetGOP(gop))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,format("Set GOP failed! %d", gop), message);
    // Inform client to synchronize setting
    isSync = true;
  }// gop
  if(isSync)
    return HC_202_ACCEPTED;
  else
    return HC_200_OK;
}

RestHTTPCode VideoActionHandler::GetAllAVCDefaultSetting(Variant &parameters, Variant &message) {
  Variant resolution;
  message["resolution"] = _pAVCCaptureInstance->GetDefaultResolution();
  message["fps"] = _pAVCCaptureInstance->GetDefaultFrameRate();
  message["bitrate"] = _pAVCCaptureInstance->GetDefaultBitRate();
  message["gop"] = _pAVCCaptureInstance->GetDefaultGOP();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetAllAVCDefaultSetting(Variant &parameters, Variant &message){
  bool isSync = false;
  if(parameters.HasKey("resolution")){
    Variant resolution = parameters["resolution"];

    // Check key existence
    if(!resolution.HasKey("width") || !resolution.HasKey("height"))
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Missing parameter width or height", message);

    // Check type
    if((resolution["height"] != _V_NUMERIC) || (resolution["width"] != _V_NUMERIC))
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS,"Parameter of number type are expected for width and height", message);

    uint16_t widthNo = (uint16_t) resolution["width"];
    uint16_t heightNo = (uint16_t) resolution["height"];
    if(!_pAVCCaptureInstance->VerifyResolution(widthNo,heightNo))
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! width : %d, height : %d", widthNo, heightNo), message);

    string width = (string) resolution["width"];
    string height = (string) resolution["height"];
    if(!_pAVCCaptureInstance->SetDefaultResolution(width, height))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set mjpg resolution failed! width %s, height %s", STR(width), STR(height)), message);
    isSync = true;
  }// resolution

  if(parameters.HasKey("fps")){
    if(parameters["fps"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Numeric parameters is expected for framerate", message);

    uint32_t fpsNo = (uint32_t) parameters["fps"];

    if(!_pAVCCaptureInstance->VerifyFPS(fpsNo))
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! frame rate : %d", fpsNo), message);

    string fps = (string) parameters["fps"];
    if(!_pAVCCaptureInstance->SetDefaultFrameRate(fps))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,format("Set framerate failed! %s", STR(fps)), message);
    // Inform client to synchronize setting
    isSync = true;
  }// fps

  if(parameters.HasKey("bitrate")){
    if(parameters["bitrate"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Numeric parameters is expected for bitrate", message);

    uint32_t bitrateNo = parameters["bitrate"];

    if(!_pAVCCaptureInstance->VerifyBitrate(bitrateNo))
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! bitrate : %d", bitrateNo), message);

    string bitrate = (string)parameters["bitrate"];
    if(!_pAVCCaptureInstance->SetDefaultBitRate(bitrate))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,format("Set bitrate failed! %s", STR(bitrate)), message);
    // Inform client to synchronize setting
    isSync = true;
  }// bitrate

  if(parameters.HasKey("gop")){
    if(parameters["gop"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Numeric parameters is expected for GOP", message);

    uint32_t gopNo = parameters["gop"];

    if(!_pAVCCaptureInstance->VerifyGOP(gopNo))
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,format("Parameter out of range! GOP : %d", gopNo), message);

    string gop = (string)parameters["gop"];
    if(!_pAVCCaptureInstance->SetDefaultGOP(gop))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,format("Set GOP failed! %d", STR(gop)), message);
    // Inform client to synchronize setting
    isSync = true;
  }// gop

  if(isSync)
    return HC_202_ACCEPTED;
  else
    return HC_200_OK;
}

RestHTTPCode VideoActionHandler::GetAVCSettingConstraints(Variant &parameters, Variant &message) {
  message = _pAVCCaptureInstance->GetConstraints();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::GetAVCCapabilities(Variant &parameters, Variant &message) {
  message["OSD"] = _pAVCCaptureInstance->IsOSDSupported();
  message["simulcast"] = _pAVCCaptureInstance->IsSimulcastSupported();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::GetAVCOSDInformation(Variant &parameters, Variant &message) {
  if(!_pAVCCaptureInstance->IsOSDSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "OSD is not supported", message);

  message = _pAVCCaptureInstance->GetOSDInformation();
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetOSD(Variant &parameters, Variant &message) {
  if(!_pAVCCaptureInstance->IsOSDSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "OSD is not supported", message);

  bool isNameEnabled, isTimeEnabled;
  int8_t namePosition = -1, datePosition = -1, dateFormatIndex = -1;
  string name, enabled;

  if(!parameters.HasKey("camera"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Camera information is not provided", message);

  if(!parameters.HasKey("date"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Time information is not provided", message);

  Variant camera = parameters["camera"];
  Variant date = parameters["date"];

  // camera enabled
  if(!camera.HasKey("enabled"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Mandatory field \'enabled\' is not provided", message);

  enabled = (string) camera["enabled"];
  if( (enabled.compare("true")==0) || (enabled.compare("1")==0) )
    isNameEnabled = true;
  else if( (enabled.compare("false")==0) || (enabled.compare("0")==0) )
    isNameEnabled = false;
  else
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Boolean parameters is expected for \'enabled\' %s", STR(enabled)), message);

  if(isNameEnabled){
    // camera name
    if(camera.HasKey("name")){
      name = (string)camera["name"];
      if(!_pAVCCaptureInstance->IsValidOSDName(name.length()))
        return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Name is too long. %s", STR(name)), message);
    }else{
      if(isNameEnabled){
        name = "iCamera";
      }
    }
  }

  // date enabled
  if(!date.HasKey("enabled"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Mandatory field \'enabled\' is not provided", message);

  enabled = (string)date["enabled"];
  if( (enabled.compare("true")==0) || (enabled.compare("1")==0) )
    isTimeEnabled = true;
  else if( (enabled.compare("false")==0) || (enabled.compare("0")==0) )
    isTimeEnabled = false;
  else
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Boolean parameters is expected for \'enabled\' %s", STR(enabled)), message);
  // date name
  if(isTimeEnabled){
    if(date.HasKey("format")){
      if(date["format"] != _V_NUMERIC)
        return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for date format"), message);

      int8_t dateFormat = date["format"];

      if(!_pAVCCaptureInstance->IsValidOSDDateFormat(dateFormat))
        return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Date format is out of range. %d", dateFormat), message);

      dateFormatIndex = dateFormat;
        //dateFormatIndex = (int8_t)atoi(STR(dateFormat));
        //if(!_pAVCCaptureInstance->IsValidOSDDateFormat(dateFormatIndex))
        //  return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Date format is out of range. %d", dateFormatIndex), message);
    }else{
      dateFormatIndex = 0;
    }
  }

  if(!_pAVCCaptureInstance->SetOSD(isNameEnabled, isTimeEnabled, name, namePosition, dateFormatIndex, datePosition))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,"Setting OSD failed", message);

  return HC_202_ACCEPTED;
}

/* jennifer : This is available only when hardware get looser constraint
RestHTTPCode VideoActionHandler::SetOSD(Variant &parameters, Variant &message) {
  bool isNameEnabled, isTimeEnabled;
  int8_t namePosition = -1, datePosition = -1, dateFormatIndex = -1;
  string name;
  if(parameters.HasKey("camera")){
    Variant camera = parameters["camera"];
    // camera enabled
    if(camera.HasKey("enabled")){
      string enabled = (string)parameters["enabled"];
      if( (enabled.compare("true")==0) || (enabled.compare("1")==0) )
        isNameEnabled = true;
      else if( (enabled.compare("false")==0) || (enabled.compare("0")==0) )
        isNameEnabled = false;
        else
        return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Boolean parameters is expected for \'enabled\' %s", STR(enabled)), message);
      }else{
        return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Boolean parameters is expected for \'enabled\' %s", STR(enabled)), message);
      }
    }else{
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Mandatory field \'enabled\' is not provided", message);
    }

    if(isNameEnabled){
      // camera name
      if(camera.HasKey("name")){
        name = (string)camera["name"];
        if(!_pAVCCaptureInstance->IsValidOSDName(name.length()))
          return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Name is too long. %s", STR(name)), message);
      }else{
        if(isNameEnabled){
          name = "iCamera";
        }
      }
      // camera position
      if(camera.HasKey("position")){
        string position = camera["position"];
          if(isNumeric(position)){
            namePosition = (uint8_t)atoi(STR(position));
            if(!_pAVCCaptureInstance->IsValidOSDPosition(namePosition))
              return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Name position is out of range. %d", namePosition), message);
          }else{
            return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for camera position %s", STR(position)), message);
          }
      }else{
        namePosition = -1;
      }
    }
  }else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Camera information is not provided", message);
  }

  if(parameters.HasKey("date")){
    Variant date = parameters["date"];
    // date enabled
    if(date.HasKey("enabled")){
      string enabled = (string) parameters["enabled"];
      if( (enabled.compare("true")==0) || (enabled.compare("1")==0) )
        isTimeEnabled = true;
      else if( (enabled.compare("false")==0) || (enabled.compare("0")==0) )
        isTimeEnabled = false;
      else
        return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Boolean parameters is expected for \'enabled\' %s", STR(enabled)), message);
    }else{
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Mandatory field \'enabled\' is not provided", message);
    }
    // date name
    if(isTimeEnabled){
      if(date.HasKey("format")){
        string format = date["format"];
        if(isNumeric(format)){
          dateFormatIndex = (int8_t)atoi(STR(format));
          if(!_pAVCCaptureInstance->IsValidOSDDateFormat(dateFormatIndex))
            return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Date format is out of range. %d", dateFormatIndex), message);
        }else{
          return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for date format %s", STR(format)), message);
        }
      }else{
        dateFormatIndex = 0;
      }
      // date position
      if(date.HasKey("position")){
        string position = date["position"];
        if(isTimeEnabled){
          if(isNumeric(position)){
            datePosition = (uint8_t)atoi(STR(position));
            if(!_pAVCCaptureInstance->IsValidOSDPosition(namePosition))
              return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Date position is out of range. %d", namePosition), message);
          }else{
            return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for date position %s", STR(position)), message);
          }
        }
      }else{
        namePosition = -1;
      }
    }
  }else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Time information is not provided", message);
  }

  if(!_pAVCCaptureInstance->SetOSD(isNameEnabled, isTimeEnabled, name, namePosition, dateFormatIndex, datePosition))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,"Setting OSD failed", message);

  return HC_202_ACCEPTED;
}
 */

RestHTTPCode VideoActionHandler::ClearAllOSD(Variant &parameters, Variant &message) {
  if(!_pAVCCaptureInstance->IsOSDSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "OSD is not supported", message);

  if(!_pAVCCaptureInstance->ClearAllOSD())
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR,"Clearing OSD failed", message);
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::GetBrightness(Variant &parameters, Variant &message) {
  message["value"] = _pAVCCaptureInstance->GetCaptureFilters()->brightness;
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetBrightness(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("value"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Brightness is undefined", message);

  if(parameters["value"] != _V_NUMERIC)
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for brightness"), message);

  int32_t brightness = parameters["value"];
  if(!_pAVCCaptureInstance->SetBrightness(brightness))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set brightness failed! %d", brightness), message);

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetContrast(Variant &parameters, Variant &message) {
  message["value"] = _pAVCCaptureInstance->GetCaptureFilters()->contrast;
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetContrast(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("value"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Contrast is undefined", message);

  if(parameters["value"] != _V_NUMERIC)
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for contrast"), message);

  int32_t contrast = parameters["value"];
  if(!_pAVCCaptureInstance->SetContrast(contrast))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set contrast failed! %d", contrast), message);

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetHue(Variant &parameters, Variant &message) {
  message["value"] = _pAVCCaptureInstance->GetCaptureFilters()->hue;
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetHue(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("value"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Hue is undefined", message);

  if(parameters["value"] != _V_NUMERIC)
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for hue"), message);

  int32_t hue = parameters["value"];
  if(!_pAVCCaptureInstance->SetHUE(hue))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set hue failed! %d", hue), message);

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetSaturation(Variant &parameters, Variant &message) {
  message["value"] = _pAVCCaptureInstance->GetCaptureFilters()->saturation;
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetSaturation(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("value"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Saturation is undefined", message);

  if(parameters["value"] != _V_NUMERIC)
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for saturation"), message);

  int32_t saturation = parameters["value"];
  if(!_pAVCCaptureInstance->SetSaturation(saturation))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set saturation failed! %d", saturation), message);

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetSharpness(Variant &parameters, Variant &message) {
  message["value"] = _pAVCCaptureInstance->GetCaptureFilters()->sharpness;
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetSharpness(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("value"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Sharpness is undefined", message);

  if(parameters["value"] != _V_NUMERIC)
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for sharpness"), message);

  int32_t sharpness = parameters["value"];
  if(!_pAVCCaptureInstance->SetSharpness(sharpness))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set sharpness failed! %d", sharpness), message);

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetGamma(Variant &parameters, Variant &message) {
  message["value"] = _pAVCCaptureInstance->GetCaptureFilters()->gamma;
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetGamma(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("value"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Gamma is undefined", message);

  if(parameters["value"] != _V_NUMERIC)
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for gamma"), message);

  int32_t gamma = parameters["value"];
  if(!_pAVCCaptureInstance->SetGamma(gamma))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set gamma failed! %d", gamma), message);

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetBacklightCompensation(Variant &parameters, Variant &message) {
  message["value"] = _pAVCCaptureInstance->GetCaptureFilters()->backlightCompensation;
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetBacklightCompensation(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("value"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Backlight compensation is undefined", message);

  if(parameters["value"] != _V_NUMERIC)
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for backlight compensation"), message);

  int32_t backlight = parameters["value"];
  if(!_pAVCCaptureInstance->SetBacklightCompensation(backlight))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set backlight failed! %d", backlight), message);

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetFlipMode(Variant &parameters, Variant &message) {
    uint8_t vflip, hflip;
    if(_pAVCCaptureInstance->GetFlipMode(&vflip, &hflip) != 0){
      FATAL("Getting flipmode failed !");
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Getting flipmode failed.", message);
    }
    //FATAL("Fipmode is : %d, %d", vflip, hflip);

    if(0 == vflip && 0 == hflip)
      message["mode"] = "normal";
    else if(0 == vflip && 1 == hflip)
      message["mode"] = "mirror";
    else if(1 == vflip && 0 == hflip)
      message["mode"] = "upsidedown";
    else if(1 == vflip && 1 == hflip)
      message["mode"] = "rotate";
    else
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Getting flipmode failed.", message);

  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetFlipMode(Variant &parameters, Variant &message) {
  if(!parameters.HasKey("mode"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS, "Mode is undefined", message);

  if(parameters["mode"] != V_STRING)
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("String parameter is expected for flip mode"), message);

  string flipmode = parameters["mode"];
  if("normal" == flipmode){
    if(_pAVCCaptureInstance->SetFlipMode(0,0) != 0){
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set flipmode failed!%s",STR(flipmode)), message);
    }
  }
  else if("mirror" == flipmode){
    if(_pAVCCaptureInstance->SetFlipMode(0,1) != 0){
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set flipmode failed!%s",STR(flipmode)), message);
    }
  }
  else if("upsidedown" == flipmode){
    if(_pAVCCaptureInstance->SetFlipMode(1,0) != 0){
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set flipmode failed!%s",STR(flipmode)), message);
    }
  }
  else if("rotate" == flipmode){
    if(_pAVCCaptureInstance->SetFlipMode(1,1) != 0){
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set flipmode failed!%s",STR(flipmode)), message);
    }
  }
  else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS, format("Unsupported mode! %s",STR(flipmode)), message);
  }

  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetAllCommonSetting(Variant &parameters, Variant &message) {
  uint8_t vflip, hflip;
  Variant flipmode;

  message["brightness"] = _pAVCCaptureInstance->GetCaptureFilters()->brightness;
  message["contrast"] = _pAVCCaptureInstance->GetCaptureFilters()->contrast;
  message["hue"] = _pAVCCaptureInstance->GetCaptureFilters()->hue;
  message["saturation"] = _pAVCCaptureInstance->GetCaptureFilters()->saturation;
  message["sharpness"] = _pAVCCaptureInstance->GetCaptureFilters()->sharpness;
  message["gamma"] = _pAVCCaptureInstance->GetCaptureFilters()->gamma;
  message["backlightCompensation"] = _pAVCCaptureInstance->GetCaptureFilters()->backlightCompensation;
  //FATAL("Common setting constraint : %s", STR(message.ToString()));

  if(_pAVCCaptureInstance->GetFlipMode(&vflip, &hflip) != 0){
    FATAL("Getting flipmode failed !");
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Getting flipmode failed.", message);
  }
  //FATAL("Fipmode is : %d, %d", vflip, hflip);

  if(0 == vflip && 0 == hflip)
    flipmode["mode"] = "normal";
  else if(0 == vflip && 1 == hflip)
    flipmode["mode"] = "mirror";
  else if(1 == vflip && 0 == hflip)
    flipmode["mode"] = "upsidedown";
  else if(1 == vflip && 1 == hflip)
    flipmode["mode"] = "rotate";
  else
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Getting flipmode failed.", message);

  message["flip"] = flipmode;
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetAllCommonSetting(Variant &parameters, Variant &message) {
  bool isSync = false;
  if(parameters.HasKey("brightness")){
    if(parameters["brightness"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for brightness"), message);

    int32_t brightness = parameters["brightness"];
    if(!_pAVCCaptureInstance->SetBrightness(brightness))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set brightness failed! %d", brightness), message);

    // Inform client to synchronize setting
    isSync = true;
  }

  if(parameters.HasKey("contrast")){
    if(parameters["contrast"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for contrast"), message);

    int32_t contrast = parameters["contrast"];
    if(!_pAVCCaptureInstance->SetContrast(contrast))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set contrast failed! %d", contrast), message);

    // Inform client to synchronize setting
    isSync = true;
  }

  if(parameters.HasKey("hue")){
    if(parameters["hue"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for hue"), message);

    int32_t hue = parameters["hue"];
    if(!_pAVCCaptureInstance->SetHUE(hue))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set hue failed! %d", hue), message);

    // Inform client to synchronize setting
    isSync = true;
  }

  if(parameters.HasKey("saturation")){
    if(parameters["saturation"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for saturation"), message);

    int32_t saturation = parameters["saturation"];
    if(!_pAVCCaptureInstance->SetSaturation(saturation))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set saturation failed! %d", saturation), message);

    // Inform client to synchronize setting
    isSync = true;
  }

  if(parameters.HasKey("sharpness")){
    if(parameters["sharpness"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for sharpness"), message);

    int32_t sharpness = parameters["sharpness"];
    if(!_pAVCCaptureInstance->SetSharpness(sharpness))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set sharpness failed! %d", sharpness), message);

    // Inform client to synchronize setting
    isSync = true;
  }

  if(parameters.HasKey("gamma")){
    if(parameters["gamma"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for gamma"), message);

    int32_t gamma = parameters["gamma"];
    if(!_pAVCCaptureInstance->SetGamma(gamma))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set gamma failed! %d", gamma), message);

    // Inform client to synchronize setting
    isSync = true;
  }

  if(parameters.HasKey("backlightCompensation")){
    if(parameters["backlightCompensation"] != _V_NUMERIC)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Numeric parameters is expected for backlight compensation"), message);

    int32_t backlight = parameters["backlightCompensation"];
    if(!_pAVCCaptureInstance->SetBacklightCompensation(backlight))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set backlight failed! %d", backlight), message);

    // Inform client to synchronize setting
    isSync = true;
  }

  // The parameter in the form of object of object if for client convenience sake
  if(parameters.HasKey("flip")){
    Variant flip= parameters["flip"];

    if(!flip.HasKey("mode"))
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS, "Mode object is undefined", message);

    if(flip["mode"] != V_STRING)
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("String parameter is expected for flip mode"), message);

    string flipmode = flip["mode"];
    uint8_t vflip, hflip;

    if("normal" == flipmode){
      vflip = 0;
      hflip = 0;
    }
    else if("mirror" == flipmode){
      vflip = 0;
      hflip = 1;
    }
    else if("upsidedown" == flipmode){
      vflip = 1;
      hflip = 0;
    }
    else if("rotate" == flipmode){
      vflip = 1;
      hflip = 1;
    }
    else{
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS, format("Unsupported mode! %s",STR(flipmode)), message);
    }

    if(_pAVCCaptureInstance->SetFlipMode(vflip,hflip) != 0){
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set flipmode failed!%s",STR(flipmode)), message);
    }
    // Inform client to synchronize setting
    isSync = true;
  }

  if(isSync)
    return HC_202_ACCEPTED;
  else
    return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetCommonToDefaultSetting(Variant &parameters, Variant &message) {
  uint8_t vflip, hflip;
  Variant flipmode;

  if(!_pAVCCaptureInstance->SetCaptureFiltersToDefault())
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Setting advanced setting to default failed.", message);

  if(_pAVCCaptureInstance->SetFlipMode(0,0) != 0){
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, format("Set flipmode to default failed!"), message);
  }

  message["brightness"] = _pAVCCaptureInstance->GetCaptureFilters()->brightness;
  message["contrast"] = _pAVCCaptureInstance->GetCaptureFilters()->contrast;
  message["hue"] = _pAVCCaptureInstance->GetCaptureFilters()->hue;
  message["saturation"] = _pAVCCaptureInstance->GetCaptureFilters()->saturation;
  message["sharpness"] = _pAVCCaptureInstance->GetCaptureFilters()->sharpness;
  message["gamma"] = _pAVCCaptureInstance->GetCaptureFilters()->gamma;
  message["backlightCompensation"] = _pAVCCaptureInstance->GetCaptureFilters()->backlightCompensation;

  if(_pAVCCaptureInstance->GetFlipMode(&vflip, &hflip) != 0){
    FATAL("Getting flipmode failed !");
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Getting flipmode failed.", message);
  }
  //FATAL("Fipmode is : %d, %d", vflip, hflip);

  if(0 == vflip && 0 == hflip)
    flipmode["mode"] = "normal";
  else if(0 == vflip && 1 == hflip)
    flipmode["mode"] = "mirror";
  else if(1 == vflip && 0 == hflip)
    flipmode["mode"] = "upsidedown";
  else if(1 == vflip && 1 == hflip)
    flipmode["mode"] = "rotate";
  else
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Getting flipmode failed.", message);

  message["flip"] = flipmode;
  return HC_202_ACCEPTED;
}

RestHTTPCode VideoActionHandler::GetCommonConstraints(Variant &parameters, Variant &message) {
  Variant filter = _pAVCCaptureInstance->GetCaptureFiltersConstraint();
  Variant flip;
  Variant mode;
  mode.PushToArray("normal");
  mode.PushToArray("mirror");
  mode.PushToArray("upsidedown");
  mode.PushToArray("rotate");
  flip["mode"] = mode;
  filter["flip"] = flip;
  message["filter"] = filter;
  return HC_200_OK;
}

RestHTTPCode VideoActionHandler::SetCommonVideoFWUpgrade(Variant &params, Variant &message) {
  Variant msg;
  string mountPath = (string)SystemManager::GetNVRam(NVRAM_STORAGE_MOUNT_PATH);

  if(!params.HasKey("filename")){
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "no filename specified", message);
  }

  string filename = mountPath+"/"+(string)params["filename"];
  if(!fileExists(filename)){
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "file not found: " + filename, message);
  }

  string touchFile = "/tmp/video_upgrade.act";
  string sha1sumCmd;
  if(params.HasKey("sha1sum") && params["sha1sum"]==V_STRING){
    sha1sumCmd = "echo "+(string)params["sha1sum"]+" >> "+touchFile;
  }
  else{
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "madatory parameter: sha1sum", message);
  }

  string ISPFileCmd = "";
  string ISPSha1sumCmd = "";
  if(params.HasKey("ISPParameter")){
    string ISPParameter = mountPath+"/"+(string)params["ISPParameter"];
    if (!fileExists(ISPParameter)){
      return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
          "file not found: " + ISPParameter, message);
    }
    ISPFileCmd = "echo "+ISPParameter+" >> "+touchFile;

    if (params.HasKey("ISPSha1sum") && params["ISPSha1sum"]==V_STRING){
      ISPSha1sumCmd="echo "+(string)params["ISPSha1sum"]+" >> "+touchFile;
    }
    else {
      return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
          "madatory parameter: ISPSha1sum", message);
    }
  }

  string filenameCmd = "echo "+filename+" >> "+touchFile;
  system(STR("touch "+touchFile));
  system(STR(filenameCmd));
  system(STR(sha1sumCmd));
  system(STR(ISPFileCmd));
  system(STR(ISPSha1sumCmd));
  return HC_200_OK;
}

//#if ((defined(__x86__)) || (defined(__MT7620__)))
//jennifer
RestHTTPCode VideoActionHandler::GetCommonYValue(Variant &params, Variant &message) {
  unsigned short yValue;
  if(!_pAVCCaptureInstance->GetAEInfo(yValue))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Getting y value failed", message);
  message["value"] = yValue;

  return HC_200_OK;
}
//#endif
