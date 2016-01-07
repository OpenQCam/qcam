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
#include "actionhandler/baseactionhandler.h"
//thelib
#include "system/systemdefine.h"
#include "application/baseclientapplication.h"
#include "protocols/http/basehttpwsprotocol.h"
#include "protocols/protocolmanager.h"

BaseActionHandler::BaseActionHandler(uint64_t actionType, string apiversion,
    BaseClientApplication *pApplication)
: ISystemCommandCallback(),
  _actionHandlerType(actionType),
  _apiVersion(apiversion),
  _pApplication(pApplication),
  _pProtocol(0)
{
}


BaseActionHandler::~BaseActionHandler() {
}


uint64_t BaseActionHandler::GetType(){
  return _actionHandlerType;
}

string BaseActionHandler::GetAPIVersion() {
  return _apiVersion;
}

bool BaseActionHandler::IsValidAPIVersion(string version) {
  return (version == _apiVersion);
}

RestHTTPCode BaseActionHandler::SendFailure(RestHTTPCode statusCode,
    RestErrorCode errorCode, string errorMessage, Variant &message){
  message[REST_ERROR_CODE] = errorCode;
  message[REST_ERROR_DESCRIPTION] = (string) errorMessage;
  return statusCode;
}

RestHTTPCode BaseActionHandler::SendSystemFailure(RestHTTPCode statusCode,
    uint16_t errorCode, string errorMessage, Variant &message){
  message[REST_ERROR_CODE] = errorCode;
  message[REST_ERROR_DESCRIPTION] = (string) errorMessage;
  return statusCode;
}


RestHTTPCode BaseActionHandler::SendBlockingCmd(const string& cmd,
                                                bool sync,
                                                Variant *scriptParam,
                                                Variant &message) {
  message[REST_SYNC_STR] = sync;
  message[REST_CMD_STR] = cmd;
  if (scriptParam) {
    message[REST_SCRIPTPARAM_STR]=*scriptParam;
  }
  return HC_204_NO_CONTENT;
}

void BaseActionHandler::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                      uint32_t dataLength) {
}

void  BaseActionHandler::SetSystemParams(Variant& response, const string& cmd,
                                        bool sync, Variant* pScriptParam) {
  response[REST_CMD_STR] = cmd;
  response[REST_SYNC_STR] = sync;
  if (pScriptParam)
    response[REST_SCRIPTPARAM_STR] = (*pScriptParam);
}
