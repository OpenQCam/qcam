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
#if ((defined(__x86__)) || (defined(__MT7620__)))
//primitive
#include <signal.h>
//thelib
#include "system/systemdefine.h"
#include "system/nvramdefine.h"
#include "clidefine.h"
#include "protocols/http/websocket/websocketdefine.h"
#include "protocols/http/outboundhttpwsprotocol.h"
//hardware
#include "hardwaremanager.h"
#include "system/systemmanager.h"
#include "gpio/basegpiodevice.h"
#include "video/baseavcvideocapture.h"
//application
#include "actionhandler/ioactionhandler.h"


using namespace app_qicstreamer;


IOActionHandler::IOActionHandler(string version, BaseClientApplication *pApplication)
: BaseActionHandler(AT_IO, version, pApplication)
{
}

IOActionHandler::~IOActionHandler() {
  (*_actionMaps[QIC_CLI_IO_PT])[true]->clear();
  (*_actionMaps[QIC_CLI_IO_PT])[false]->clear();
  (*_actionMaps[QIC_CLI_IO_LED])[true]->clear();
  (*_actionMaps[QIC_CLI_IO_LED])[false]->clear();
  (*_actionMaps[QIC_CLI_IO_PIR])[true]->clear();
  (*_actionMaps[QIC_CLI_IO_PIR])[false]->clear();
  _actionMaps[QIC_CLI_IO_PT]->clear();
  _actionMaps[QIC_CLI_IO_LED]->clear();
  _actionMaps[QIC_CLI_IO_PIR]->clear();
  _actionMaps.clear();
}

bool IOActionHandler::Initialize() {

  _pAVCCaptureInstance = reinterpret_cast<BaseAVCVideoCapture *>(HardwareManager::GetHardwareInstance(HT_VIDEO_AVC));

  if(_pAVCCaptureInstance == NULL){
    FATAL("Unable to access MJPG video camera through hardware manager.");
    return false;
  }
  // action functions

  if(_pAVCCaptureInstance->IsPTSupported()){
    _pAVCCaptureInstance->RotatePTToPosition(0,0);
  }

  CollectionMap *pPTMap= new CollectionMap;
  CollectionMap *pLEDMap= new CollectionMap;
  CollectionMap *pPIRMap= new CollectionMap;
  CollectionMap *pIRLEDMap= new CollectionMap;
  CollectionMap *pALSMap= new CollectionMap;
  ActionMap *pPTGetActionMap = new ActionMap;
  ActionMap *pPTPostActionMap = new ActionMap;
  ActionMap *pLEDGetActionMap = new ActionMap;
  ActionMap *pLEDPostActionMap = new ActionMap;
  ActionMap *pPIRGetActionMap = new ActionMap;
  ActionMap *pPIRPostActionMap = new ActionMap;
  ActionMap *pIRLEDGetActionMap = new ActionMap;
  ActionMap *pIRLEDPostActionMap = new ActionMap;
  ActionMap *pALSGetActionMap = new ActionMap;

  (*pPTMap)[true] = pPTGetActionMap;
  (*pPTMap)[false] = pPTPostActionMap;
  (*pLEDMap)[true] = pLEDGetActionMap;
  (*pLEDMap)[false] = pLEDPostActionMap;
  (*pPIRMap)[true] = pPIRGetActionMap;
  (*pPIRMap)[false] = pPIRPostActionMap;
  (*pIRLEDMap)[true] = pIRLEDGetActionMap;
  (*pIRLEDMap)[false] = pIRLEDPostActionMap;
  (*pALSMap)[true] = pALSGetActionMap;

  _actionMaps[QIC_CLI_IO_PT] = pPTMap;
  _actionMaps[QIC_CLI_IO_LED] = pLEDMap;
  _actionMaps[QIC_CLI_IO_PIR] = pPIRMap;
  _actionMaps[QIC_CLI_IO_IR_LED] = pIRLEDMap;
  _actionMaps[QIC_CLI_IO_ALS] = pALSMap;

  /*PT motors*/
  //PT Get API
  (*pPTGetActionMap)[QIC_CLI_IO_PT_SUPPORT] =
    reinterpret_cast<Action>(&IOActionHandler::GetPTSupportStatus);
  (*pPTGetActionMap)[QIC_CLI_IO_PT_POSITION] =
    reinterpret_cast<Action>(&IOActionHandler::GetPTPosition);
  (*pPTGetActionMap)[QIC_CLI_IO_PT_TAG_LIST] =
    reinterpret_cast<Action>(&IOActionHandler::GetPTTagList);
  //PT Post API
  //(*pPTPostActionMap)[QIC_CLI_IO_PT_ROTATE] =
  //  reinterpret_cast<Action>(&IOActionHandler::RotatePT);
  //(*pPTPostActionMap)[QIC_CLI_IO_PT_STOP] =
  //  reinterpret_cast<Action>(&IOActionHandler::StopPT);
  //TODO : open this for IJ7
  (*pPTPostActionMap)[QIC_CLI_IO_PT_ROTATE] =
    reinterpret_cast<Action>(&IOActionHandler::PostPTRotate);
  (*pPTPostActionMap)[QIC_CLI_IO_PT_STOP] =
    reinterpret_cast<Action>(&IOActionHandler::PostPTStop);
  (*pPTPostActionMap)[QIC_CLI_IO_PT_TAG] =
    reinterpret_cast<Action>(&IOActionHandler::PostPTTag);
  (*pPTPostActionMap)[QIC_CLI_IO_PT_TAG_REPLACE] =
    reinterpret_cast<Action>(&IOActionHandler::PostPTTagEdit);
  (*pPTPostActionMap)[QIC_CLI_IO_PT_TAG_RENAME] =
    reinterpret_cast<Action>(&IOActionHandler::PostPTTagRename);
  (*pPTPostActionMap)[QIC_CLI_IO_PT_TAG_DELETE] =
    reinterpret_cast<Action>(&IOActionHandler::PostPTTagDelete);
  (*pPTPostActionMap)[QIC_CLI_IO_PT_TAG_LIST_CLEAR] =
    reinterpret_cast<Action>(&IOActionHandler::PostPTTagListClear);
  (*pPTPostActionMap)[QIC_CLI_IO_PT_NAVIGATE] =
    reinterpret_cast<Action>(&IOActionHandler::PostPTNavigate);
  (*pPTPostActionMap)[QIC_CLI_IO_PT_NAVIGATE_TO] =
    reinterpret_cast<Action>(&IOActionHandler::PostPTNavigateTo);
  (*pPTPostActionMap)[QIC_CLI_IO_PT_RESET] =
    reinterpret_cast<Action>(&IOActionHandler::PostPTReset);

  /*LED*/
  //LED GET API
  (*pLEDGetActionMap)[QIC_CLI_IO_LED_ID_STATUS] =
    reinterpret_cast<Action>(&IOActionHandler::GetLEDStatus);
  //LED POST API
  (*pLEDPostActionMap)[QIC_CLI_IO_LED_ID_STATUS] =
    reinterpret_cast<Action>(&IOActionHandler::PostLEDStatus);
  (*pLEDPostActionMap)[QIC_CLI_IO_LED_ID_EVENT] =
    reinterpret_cast<Action>(&IOActionHandler::PostLEDEvent);

  //PIR
  (*pPIRGetActionMap)[QIC_CLI_IO_PIR_STATUS] =
    reinterpret_cast<Action>(&IOActionHandler::GetPIRStatus);
  (*pPIRPostActionMap)[QIC_CLI_IO_PIR_STATUS] =
    reinterpret_cast<Action>(&IOActionHandler::PostPIRStatus);

  //IR LED
  (*pIRLEDGetActionMap)[QIC_CLI_IO_IR_LED_STATUS] =
    reinterpret_cast<Action>(&IOActionHandler::GetIRLEDStatus);
  (*pIRLEDPostActionMap)[QIC_CLI_IO_IR_LED_STATUS] =
    reinterpret_cast<Action>(&IOActionHandler::PostIRLEDStatus);

  //ALS
  (*pALSGetActionMap)[QIC_CLI_IO_ALS_VALUE] =
    reinterpret_cast<Action>(&IOActionHandler::GetALS);


  return true;
}

void IOActionHandler::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                               uint32_t dataLength) {
  DEBUG ("OnCmdFinished");
}

bool IOActionHandler::IsValidAction(map<string, CollectionMap*> *map,
                                         string collection,
                                         string id,
                                         bool isReadyOnly) {
  return true;
}

RestHTTPCode IOActionHandler::DoAction(BaseProtocol *pFrom,
                                       bool isRO,
                                       vector<string> &resource,
                                       Variant &parameters,
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
  return ((this->*pAction)(parameters[REST_PARAM_STR], message));
}

// Private Functions

/*PT Motors Control*/
RestHTTPCode IOActionHandler::RotatePT(Variant &parameters, Variant &message) {
  DEBUG("RotatePT");
  if(!fileExists("/tmp/pt_motor")){
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_606_IO_PT_DEVICE_CONTROL_ERROR,
        "Not support PT motor control", message);
  }
  // Speed Control
  string cmd = "icam_motor_speed ";
  if(parameters.HasKey("panSpeed")){
    string panSpeed = (string) parameters["pan_speed"];
    if(isNumeric(panSpeed)){
      cmd = cmd + " " + panSpeed;
    }else{
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_108_INVALID_PARAMETERS,
        "Must set numeric value for panSpeed", message);
    }
  }else{
    cmd = cmd + " 0"; //0:full speed
  }

  if(parameters.HasKey("tiltSpeed")){
    string tiltSpeed = (string) parameters["tilt_speed"];
    if(isNumeric(tiltSpeed)){
      cmd = cmd + " " + tiltSpeed;
    }else{
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_108_INVALID_PARAMETERS,
        "Must set numeric value for tiltSpeed", message);
    }
  }
  else{
    cmd = cmd + " 0"; //0:full speed
  }
  DEBUG("command: %s", cmd.c_str());
  UnixDomainSocketManager::ExecuteSystemCommand(cmd);

  // Rotate Control
  cmd = "icam_motor_start ";
  if(parameters.HasKey("pan")){
    string pan = (string) parameters["pan"];
    if(isNumeric(pan)){
      cmd = cmd + " " + pan;
    }else{
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_108_INVALID_PARAMETERS,
        "Must set numeric value for pan", message);
    }
  }else{
    cmd = cmd + " 0";
  }

  if(parameters.HasKey("tilt")){
    string tilt = (string) parameters["tilt"];
    if(isNumeric(tilt)){
      cmd = cmd + " " + tilt;
    }else{
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_108_INVALID_PARAMETERS,
        "Must set numeric value for tilt", message);
    }
  }
  else{
    cmd = cmd + " 0";
  }
  DEBUG("command: %s", cmd.c_str());
  UnixDomainSocketManager::ExecuteSystemCommand(cmd);

  return HC_200_OK;
}

RestHTTPCode IOActionHandler::StopPT(Variant &parameters, Variant &message) {
  DEBUG("Stop");
  if(!fileExists("/tmp/pt_motor")){
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_606_IO_PT_DEVICE_CONTROL_ERROR,
        "Not support PT motor control", message);
  }

  string cmd = "icam_motor_stop ";
  UnixDomainSocketManager::ExecuteSystemCommand(cmd);
  return HC_200_OK;
}

RestHTTPCode IOActionHandler::GetPTSupportStatus(Variant &parameters, Variant &message){
  message["status"] = _pAVCCaptureInstance->IsPTSupported();
  return HC_200_OK;
}

RestHTTPCode IOActionHandler::GetPTPosition(Variant &parameters, Variant &message) {
  if(!_pAVCCaptureInstance->IsPTSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "PT is not supported", message);

  if(!_pAVCCaptureInstance->GetCurrentPTPosition(message))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Getting current Pan Tilt positon failed", message);
  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostPTRotate(Variant &parameters, Variant &message){
  if(!_pAVCCaptureInstance->IsPTSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "PT is not supported", message);

  if(parameters.HasKey("pan") && parameters.HasKey("tilt")){
    string pan = (string) parameters["pan"];
    string tilt = (string) parameters["tilt"];
    if(isNumeric(pan) && isNumeric(tilt)){
      int16_t panNo = (int16_t)atoi(STR(pan));
      int16_t tiltNo = (int16_t)atoi(STR(tilt));
      FATAL("Client target pan : %d, tilt : %d", panNo, tiltNo);

      int8_t isTopBoundary = -1;
      int8_t isBottomBoundary = -1;
      int8_t isLeftBoundary = -1;
      int8_t isRightBoundary = -1;
      _pAVCCaptureInstance->CheckPanTiltBoundary(panNo, tiltNo, isLeftBoundary, isRightBoundary, isTopBoundary, isBottomBoundary);

      FATAL("isLeftBoundary : %d, isRightBoundary : %d, isTopBoundary : %d, isBottomBoundary : %d", isLeftBoundary, isRightBoundary, isTopBoundary, isBottomBoundary);
      message["reachLeftBoundary"] = isLeftBoundary;
      message["reachRightBoundary"] = isRightBoundary;
      message["reachTopBoundary"] = isTopBoundary;
      message["reachBottomBoundary"] = isBottomBoundary;

      if(!_pAVCCaptureInstance->RotatePT(panNo, tiltNo))
        return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Rotating camera failed", message);
    }else{
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Numeric parameters are expected.", message);
    }
  }else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Pan/tilt is undefined", message);
  }
  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostPTTag(Variant &parameters, Variant &message){
  if(!_pAVCCaptureInstance->IsPTSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "PT is not supported", message);

  if(!parameters.HasKey("name"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Tag name is mandatory", message);

  string name = parameters["name"];
  if(name.empty())
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Empty name", message);

  uint8_t nvramIndex;
  if(!_pAVCCaptureInstance->GetAvailableTagSpace(nvramIndex))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_609_IO_PT_REACH_MAX_TAG, "Reach the limitation of maximum tags allowed. Please use replace/delete instead", message);

  if(_pAVCCaptureInstance->IsPTTagExist(name))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_604_IO_PT_ERROR, "Tag already exist", message);

  if(!_pAVCCaptureInstance->TagCurrentPTPosition(name))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Tagging current positon failed", message);

  return HC_200_OK;
}

RestHTTPCode IOActionHandler::GetPTTagList(Variant &parameters, Variant &message){
  if(!_pAVCCaptureInstance->IsPTSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "PT is not supported", message);

  if(_pAVCCaptureInstance->IsPTSupported())
    message["tagList"] = _pAVCCaptureInstance->GetPTTagList();

  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostPTTagEdit(Variant &parameters, Variant &message){
  if(!_pAVCCaptureInstance->IsPTSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "PT is not supported", message);

  if(!parameters.HasKey("oldName"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Pan/tilt is undefined", message);

  string oldName = (string) parameters["oldName"];
  string newName = "";
  if(parameters.HasKey("newName")){
    newName = (string) parameters["newName"];
  }

  if(oldName.empty())
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Old tag name is mandatory", message);

  if(!_pAVCCaptureInstance->EditTag(oldName, newName))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Replacing tag failed", message);

  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostPTTagRename(Variant &parameters, Variant &message){
  if(!_pAVCCaptureInstance->IsPTSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "PT is not supported", message);

  if(!parameters.HasKey("oldName"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"\'oldName\' is undefined", message);

  if(!parameters.HasKey("newName"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"\'newName\' is undefined", message);

  string oldName = (string) parameters["oldName"];
  string newName = (string) parameters["newName"];

  if(oldName.empty())
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Old tag name is mandatory", message);

  if(newName.empty())
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"New tag name is mandatory", message);

  if(!_pAVCCaptureInstance->RenameTag(oldName, newName))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Renaming tag failed", message);

  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostPTTagDelete(Variant &parameters, Variant &message){
  if(!_pAVCCaptureInstance->IsPTSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "PT is not supported", message);

  if(!parameters.HasKey("name"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Tag name is mandatory", message);

  string name = parameters["name"];
  if(name.empty())
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Empty name", message);

  if(!_pAVCCaptureInstance->DeleteTag(name))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Deleting tag failed", message);

  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostPTTagListClear(Variant &parameters, Variant &message){
  if(!_pAVCCaptureInstance->IsPTSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "PT is not supported", message);

  if(!_pAVCCaptureInstance->ClearPTTagList())
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Clearing tag list failed", message);

  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostPTNavigate(Variant &parameters, Variant &message){
  if(!_pAVCCaptureInstance->IsPTSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "PT is not supported", message);

  if(!_pAVCCaptureInstance->NavigatePT())
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Navigating pan/tilt through pre-defined position failed", message);

  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostPTNavigateTo(Variant &parameters, Variant &message){
  if(!_pAVCCaptureInstance->IsPTSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "PT is not supported", message);

  queue<string> tagList;
  Variant content = parameters["tagList"];
  //FATAL("Is parameter array : %d", content.IsArray());
  if(!content.IsArray())
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS,"Old tag name is mandatory", message);
  //FATAL("Array size : %d", content.MapSize());
  uint8_t index = 0;
  uint32_t PTTagsLength = content.MapSize();
  if(1 == PTTagsLength){
    string tagName = (string)content[index];
    FATAL("Tag name : %s", STR(tagName));
    if(!_pAVCCaptureInstance->RotatePTToTag(tagName))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Navigating pan/tilt to the specific positions failed", message);
  }
  else{
    for(index=0; index<PTTagsLength; index++){
      tagList.push((string)content[index]);
      //FATAL("Array content : %s", STR((string)content[index]));
    }
    if(!_pAVCCaptureInstance->NavigatePTTo(tagList))
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Navigating pan/tilt through the specific positions failed", message);
  }
  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostPTReset(Variant &parameters, Variant &message){
  if(!_pAVCCaptureInstance->IsPTSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "PT is not supported", message);

  if(!_pAVCCaptureInstance->RotatePTToPosition(0,0))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Reset pan/tilt failed", message);

  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostPTStop(Variant &parameters, Variant &message){
  if(!_pAVCCaptureInstance->IsPTSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "PT is not supported", message);

  if(!_pAVCCaptureInstance->StopPTRotate())
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Stop camera rotating failed", message);

  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostLEDStatus(Variant &parameters, Variant &message) {
  //uint32_t type;
  //jennifer

  BaseGPIODevice *pGPIOInstance = reinterpret_cast<BaseGPIODevice*>(HardwareManager::GetHardwareInstance(HT_GPIO));
  if(pGPIOInstance == NULL){
    FATAL("Unable to access GPIO interface");
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_601_IO_GPIO_ERROR,
        "Unable to access LED interface", message);
  }

  if (parameters.HasKey("enabled") && parameters["enabled"]==V_BOOL) {
    bool enabled = (bool)parameters["enabled"];
    if (enabled){
      pGPIOInstance->SetLedEvent(LED_EVENT_STATUS_SLEEP_MODE, false);
    }
    else{
      pGPIOInstance->SetLedEvent(LED_EVENT_STATUS_SLEEP_MODE, true);
    }
  }
  else {
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
                       EC_109_MISSING_MANDATORY_PARAMETERS,
                       "Invalid parameter: enabled", message);
  }
  return HC_202_ACCEPTED;
}

RestHTTPCode IOActionHandler::GetLEDStatus(Variant &parameters, Variant &message) {
  BaseGPIODevice *pGPIOInstance = reinterpret_cast<BaseGPIODevice*>(HardwareManager::GetHardwareInstance(HT_GPIO));

  if (pGPIOInstance == NULL) {
    FATAL("Unable to access GPIO interface");
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_601_IO_GPIO_ERROR,
        "Unable to access LED interface", message);
  }
   else {
    int32_t status = pGPIOInstance->CheckLedEvent(LED_EVENT_STATUS_SLEEP_MODE);
    if(status == 0){
      message["enabled"] = true; //led status enabled (sleep mode)
    }
    else if(status==1){
      message["enabled"] = false; //led status disabled (not sleep mode)
    }
    else{
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_601_IO_GPIO_ERROR,
          "Unable to access LED interface", message);
    }
  }
  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostLEDEvent(Variant &parameters, Variant &message) {
  uint32_t event;

  BaseGPIODevice *pGPIOInstance = reinterpret_cast<BaseGPIODevice*>(HardwareManager::GetHardwareInstance(HT_GPIO));
  if(pGPIOInstance == NULL){
    FATAL("Unable to access GPIO interface");
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_601_IO_GPIO_ERROR,
        "Unable to access LED interface", message);
  }

  if (parameters.HasKey("event") && parameters["event"]==V_INT64) {
    event =(uint32_t)parameters["event"];
  }
  else {
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
                       EC_109_MISSING_MANDATORY_PARAMETERS,
                       "Invalid parameter: event", message);
  }

  if (parameters.HasKey("enabled") && parameters["enabled"]==V_BOOL) {
    bool enabled = (bool)parameters["enabled"];
    pGPIOInstance->SetLedEvent((LEDEventType)event, enabled);
  }
  else {
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR,
                       EC_109_MISSING_MANDATORY_PARAMETERS,
                       "Invalid parameter: enabled", message);
  }
  return HC_202_ACCEPTED;
}

RestHTTPCode IOActionHandler::GetPIRStatus(Variant &params, Variant &message) {
  string pirStatus=SystemManager::GetNVRam(NVRAM_IO_PIR_ENABLED);
  message["enabled"]=(pirStatus!="0");

  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostPIRStatus(Variant &params, Variant &message) {
  BaseGPIODevice *pGPIO=
      reinterpret_cast<BaseGPIODevice*>(HardwareManager::GetHardwareInstance(HT_GPIO));
  if (!pGPIO) {
    return SendFailure(HC_503_SERVICE_UNAVAILABLE, EC_607_IO_PIR_DEVICE_NOT_FOUND,
         "PIR device not found", message);
  }

  if (!params.HasKey("enabled") || params["enabled"]!=V_BOOL) {
    return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
         "Invalid parameter: enabled", message);
  }

  string enabled=((bool)params["enabled"]==true)?"1":"0";
  SystemManager::SetNVRam(NVRAM_IO_PIR_ENABLED, enabled, false);
  if (enabled.compare("1")==0) {
    pGPIO->EnablePIRNotification();
  }
  else {
    pGPIO->DisablePIRNotification();
  }
  return HC_200_OK;
}

RestHTTPCode IOActionHandler::GetIRLEDStatus(Variant &params, Variant &message) {
  bool status = false;
  string mode = "holder";
  if(!_pAVCCaptureInstance->IsIRSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "IR is not supported", message);

  if(!_pAVCCaptureInstance->GetIRLEDStatus(status, mode))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Getting IR LED status failed", message);
  message["mode"] = mode;
  message["enabled"] = status;
  return HC_200_OK;
}

RestHTTPCode IOActionHandler::PostIRLEDStatus(Variant &params, Variant &message) {
  unsigned char mode;
  if(!_pAVCCaptureInstance->IsIRSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "IR is not supported", message);

  if(!params.HasKey("mode"))
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Missing parameter \'mode\'", message);

  string IRLEDMode = params["mode"];
  if(IRLEDMode.compare("manual")==0){
    if(!params.HasKey("enabled"))
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_109_MISSING_MANDATORY_PARAMETERS,"Missing parameter \'enabled\'", message);

    string status = params["enabled"];
    if( (status.compare("true")==0) || (status.compare("1")==0) )
      mode = 1;
    else if( (status.compare("false")==0) || (status.compare("0")==0) )
      mode = 0;
    else
      return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Boolean parameters is expected for \'enabled\'. Input was: %s", STR(status)), message);
  }
  else if(IRLEDMode.compare("auto")==0)
    mode = 2;
  else
    return SendFailure(HC_406_NOT_ACCEPTABLE, EC_108_INVALID_PARAMETERS, format("Available mode are manual and auto. Input was %s", STR(IRLEDMode)), message);

  if(!_pAVCCaptureInstance->SetIRLEDStatus(mode))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Setting IR LED status failed", message);

  return HC_202_ACCEPTED;
}

RestHTTPCode IOActionHandler::GetALS(Variant &params, Variant &message) {
  if(!_pAVCCaptureInstance->IsALSSupported())
    return SendFailure(HC_405_METHOD_NOT_ALLOWED, EC_103_METHOD_NOT_ALLOWED, "ALS is not supported", message);

  unsigned short lux;
  if(!_pAVCCaptureInstance->GetALS(lux))
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_308_VIDEO_IO_CONTROL_ERROR, "Getting ALS value failed", message);
  message["value"] = lux;

  return HC_200_OK;
}

#endif
#endif
