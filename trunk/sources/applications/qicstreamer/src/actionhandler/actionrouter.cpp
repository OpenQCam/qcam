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

#include "actionhandler/actionrouter.h"
//thelib
#include "system/errordefine.h"
//hardware
#include "hardwaremanager.h"
//application
#include "clidefine.h"
#include "actionhandler/baseactionhandler.h"
#include "actionhandler/deviceactionhandler.h"
#include "actionhandler/videoactionhandler.h"
#include "actionhandler/audioactionhandler.h"
#include "actionhandler/recordactionhandler.h"
#include "actionhandler/ioactionhandler.h"
#include "actionhandler/eventactionhandler.h"
#include "actionhandler/serviceactionhandler.h"
#include "actionhandler/networkactionhandler.h"
#include "actionhandler/productionactionhandler.h"

#include "application/baseclientapplication.h"


using namespace app_qicstreamer;

ActionRouter::ActionRouter()
{
}

ActionRouter::~ActionRouter() {
}

bool ActionRouter::CreateProductionActionHandlers(BaseClientApplication *pBaseApp) {

  INFO("Init production action handler!");
  ProductionActionHandler *pProductionActionHandler = new ProductionActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  if (pProductionActionHandler->Initialize()) {
    RegisterActionHandler(AT_PRODUCTION_STR, reinterpret_cast<BaseActionHandler *>(pProductionActionHandler));
  }
  else {
    FATAL("Initial Production Action Handler failed");
    //return false;
  }

  //1. Initial Video Action Handler
  VideoActionHandler *pVideoActionHandler = new VideoActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  if(pVideoActionHandler->Initialize()){
    INFO("Initial video action handler!");
  }else{
    FATAL("Initial video action handler failed");
  }
  RegisterActionHandler(AT_VIDEO_STR, reinterpret_cast<BaseActionHandler *>(pVideoActionHandler));

  //2. Initial Audio Action Handler
  AudioActionHandler *pAudioActionHandler = new AudioActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  if(pAudioActionHandler->Initialize()){
    INFO("Initial audio action handler!");
  }else{
    FATAL("Initial audio action handler failed");
  }
  RegisterActionHandler(AT_AUDIO_STR, reinterpret_cast<BaseActionHandler *>(pAudioActionHandler));

  //3. Initial Record Action Handler
  RecordActionHandler *pRecordActionHandler = new RecordActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  //_pRecordActionHandler->SetCameraInstance(_pCameraInstance);
  if(pRecordActionHandler->Initialize()){
    INFO("Initial record action handler!");
  }else{
    FATAL("Initial record action handler failed");
  }
  RegisterActionHandler(AT_RECORD_STR, reinterpret_cast<BaseActionHandler *>(pRecordActionHandler));

  //4. Initial IO Action Handler
  //IOActionHandler *pIOActionHandler = new IOActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  //if(pIOActionHandler->Initialize()){
  //  INFO("Initial io action handler!");
  //}else{
  //  FATAL("Initial io action handler failed");
  //}
  //RegisterActionHandler(AT_IO_STR, reinterpret_cast<BaseActionHandler *>(pIOActionHandler));

  //5. Initial Event Action Handler
  EventActionHandler *pEventActionHandler = new EventActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  if(pEventActionHandler->Initialize() ){
    INFO("Initial event action handler!");
  }else{
    FATAL("Initial event action handler failed");
  }
  RegisterActionHandler(AT_EVENT_STR, reinterpret_cast<BaseActionHandler *>(pEventActionHandler));


  //6. Initial Network Action Handler
  NetworkActionHandler *pNetworkActionHandler = new NetworkActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  if(pNetworkActionHandler->Initialize()){
    INFO("Initial Network action handler!");
  }else{
    FATAL("Initial Network action handler failed");
  }
  RegisterActionHandler(AT_NETWORK_STR, reinterpret_cast<BaseActionHandler *>(pNetworkActionHandler));

  //7. Initial Network Action Handler
  DeviceActionHandler *pDeviceActionHandler = new DeviceActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  if(pDeviceActionHandler->Initialize()){
    INFO("Initial device action handler!");
  }else{
    FATAL("Initial device action handler failed");
  }
  RegisterActionHandler(AT_DEVICE_STR, reinterpret_cast<BaseActionHandler *>(pDeviceActionHandler));

  //8. Initial Service Action Handler
  //ServiceActionHandler *pServiceActionHandler = new ServiceActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  //if(pServiceActionHandler->Initialize()){
  //  INFO("Initial service action handler!");
  //}else{
  //  FATAL("Initial service action handler failed");
  //  return false;
  //}
  //RegisterActionHandler(AT_SERVICE_STR, reinterpret_cast<BaseActionHandler *>(pServiceActionHandler));

  return true;
}

bool ActionRouter::CreateActionHandlers(BaseClientApplication *pBaseApp){

  //1. Initial Video Action Handler
  VideoActionHandler *pVideoActionHandler = new VideoActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  if(pVideoActionHandler->Initialize()){
    INFO("Initial video action handler!");
  }else{
    FATAL("Initial video action handler failed");
    return false;
  }
  RegisterActionHandler(AT_VIDEO_STR, reinterpret_cast<BaseActionHandler *>(pVideoActionHandler));

  //2. Initial Audio Action Handler
  AudioActionHandler *pAudioActionHandler = new AudioActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  if(pAudioActionHandler->Initialize()){
    INFO("Initial audio action handler!");
  }else{
    FATAL("Initial audio action handler failed");
    return false;
  }
  RegisterActionHandler(AT_AUDIO_STR, reinterpret_cast<BaseActionHandler *>(pAudioActionHandler));

  //3. Initial Record Action Handler
  RecordActionHandler *pRecordActionHandler = new RecordActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  //_pRecordActionHandler->SetCameraInstance(_pCameraInstance);
  if(pRecordActionHandler->Initialize()){
    INFO("Initial record action handler!");
  }else{
    FATAL("Initial record action handler failed");
    return false;
  }
  RegisterActionHandler(AT_RECORD_STR, reinterpret_cast<BaseActionHandler *>(pRecordActionHandler));

  //4. Initial IO Action Handler
  //IOActionHandler *pIOActionHandler = new IOActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  //if(pIOActionHandler->Initialize()){
  //  INFO("Initial io action handler!");
  //}else{
  //  FATAL("Initial io action handler failed");
  //  return false;
  //}
  //RegisterActionHandler(AT_IO_STR, reinterpret_cast<BaseActionHandler *>(pIOActionHandler));

  //5. Initial Event Action Handler
  EventActionHandler *pEventActionHandler = new EventActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  if(pEventActionHandler->Initialize() ){
    INFO("Initial event action handler!");
  }else{
    FATAL("Initial event action handler failed");
    return false;
  }
  RegisterActionHandler(AT_EVENT_STR, reinterpret_cast<BaseActionHandler *>(pEventActionHandler));


  //5. Initial Network Action Handler
  NetworkActionHandler *pNetworkActionHandler = new NetworkActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  if(pNetworkActionHandler->Initialize()){
    INFO("Initial Network action handler!");
  }else{
    FATAL("Initial Network action handler failed");
    return false;
  }
  RegisterActionHandler(AT_NETWORK_STR, reinterpret_cast<BaseActionHandler *>(pNetworkActionHandler));

  //6. Initial ONVIF Action Handler

  //7. Initial Extension Action Handler

  //8. Initial Device Action Handler

  DeviceActionHandler *pDeviceActionHandler = new DeviceActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  if(pDeviceActionHandler->Initialize()){
    INFO("Initial device action handler!");
  }else{
    FATAL("Initial device action handler failed");
    return false;
  }
  RegisterActionHandler(AT_DEVICE_STR, reinterpret_cast<BaseActionHandler *>(pDeviceActionHandler));

  //9. Initial Service Action Handler

  ServiceActionHandler *pServiceActionHandler = new ServiceActionHandler(QIC_CLI_API_VERSION_V2, pBaseApp);
  if(pServiceActionHandler->Initialize()){
    INFO("Initial service action handler!");
  }else{
    FATAL("Initial service action handler failed");
    return false;
  }
  RegisterActionHandler(AT_SERVICE_STR, reinterpret_cast<BaseActionHandler *>(pServiceActionHandler));

  return true;
}


bool ActionRouter::IsReadyToAction(RestCLIMessage &restMessage) {
  //Block action if cpu is busy
  uint32_t errorCheck[] = {EC_CPU_BUSY};

  for (uint32_t i=0; i<sizeof(errorCheck); i++) {
    if (HardwareManager::IsErrorStatus(errorCheck[i])) {
      restMessage.response.statusCode = HC_500_INTERNAL_SERVER_ERROR;
      restMessage.response.data[REST_ERROR_CODE] = EC_CPU_BUSY;
      restMessage.response.data[REST_ERROR_DESCRIPTION] = ERROR_DESCRIPTION(EC_CPU_BUSY);
      return false;
    }
  }
  return true;
}
