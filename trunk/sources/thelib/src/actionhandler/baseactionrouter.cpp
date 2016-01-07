/* ==================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 * (C) COPYRIGHT Quanta Computer Inc. ALL RIGHTS RESERVED
 *
 * ==================================================================
 * ------------------------------------------------------------------
 *  Date    :
 *  Version :
 *  Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 *  Purpose :
 *  =================================================================
 */

#include "actionhandler/baseactionrouter.h"
#include "actionhandler/baseactionhandler.h"
#include "protocols/cli/restclidefine.h"

BaseActionRouter::BaseActionRouter() {
}

BaseActionRouter::~BaseActionRouter() {
}

bool BaseActionRouter::RouteRequestAction(RestCLIMessage &restMessage) {
  vector<string> resource;

  //if (!IsReadyToAction(restMessage)) {
  //  return true;
  //}

  split(restMessage.request[REST_PAYLOAD_STR][REST_URI_STR], "/", resource);
  if (resource.size() < 5) {
    restMessage.response.statusCode = HC_400_BAD_REQUEST;
    restMessage.response.data[REST_ERROR_CODE] = EC_101_INVALID_API_REQUEST;
    restMessage.response.data[REST_ERROR_DESCRIPTION] =
        (string) format("Invalid API uri : %s",
                        STR(restMessage.request[REST_PAYLOAD_STR][REST_URI_STR]));
    return true;
  }

  string apiverion = resource[1];
  string category = resource[2];

  // 1. check if action category is valid
  BaseActionHandler *pHandler = GetActionHandler(category);
  if(pHandler == NULL){
    restMessage.response.statusCode = HC_400_BAD_REQUEST;
    restMessage.response.data[REST_ERROR_CODE] = EC_104_CATEGORY_NOT_FOUND;
    restMessage.response.data[REST_ERROR_DESCRIPTION] =
        (string) format("Unable to find this action path: %s", STR(category));
    return true;
  }

  // 2. check if API is valid
  if(!pHandler->IsValidAPIVersion(apiverion)){
    restMessage.response.statusCode = HC_400_BAD_REQUEST;
    restMessage.response.data[REST_ERROR_CODE] = EC_102_INCOMPATIBLE_API;
    restMessage.response.data[REST_ERROR_DESCRIPTION] =
      (string) format("Invalid API version: %s, Current API version %s",
          STR(apiverion), STR(pHandler->GetAPIVersion()));
    return true;
  }

  // 3. done. Do the action
  DEBUG ("action: %s", STR(restMessage.request[REST_PAYLOAD_STR][REST_URI_STR]));
  restMessage.response.statusCode = pHandler->DoAction(restMessage.protocol,
                                                       restMessage.request[REST_ISRO_STR],
                                                       resource,
                                                       restMessage.request[REST_PAYLOAD_STR],
                                                       restMessage.response.data);
  restMessage.cbId=pHandler->GetCallbackId();
  DEBUG ("action end");
  return true;
}

void BaseActionRouter::RegisterActionHandler(string actionType,
                                             BaseActionHandler *pActionHanlder)
{
  if(MAP_HAS1(_actionHandlers, actionType)){
    WARN("Action handler was already registered");
  }else{
    _actionHandlers[actionType] = pActionHanlder;
  }
}

void BaseActionRouter::UnRegisterActionHandler(string actionType)
{
  if(MAP_HAS1(_actionHandlers, actionType)){
    _actionHandlers.erase(actionType);
  }else{
    WARN("Action handler was not registered");
  }
}


BaseActionHandler* BaseActionRouter::GetActionHandler(string actionType)
{
  if(MAP_HAS1(_actionHandlers, actionType)){
    return (BaseActionHandler*) _actionHandlers[actionType];
  }else{
    WARN("Action handler not found");
  }
  return NULL;
}

// Parse Generic
bool BaseActionRouter::ParseURI(const string &path, URI &uri) {
  string uriString = "https://localhost" + path;
  uri.Reset();
  return URI::FromString(uriString, false, uri);
}

bool BaseActionRouter::ParseFirstLine(string &line, URI &uri, RestCLIMessage &restMessage) {

  vector<string> parts;
  split(line, " ", parts);
  if (parts.size() != 3) {
    FATAL("Incorrect first line: %s", STR(line));
    return false;
  }

  if (parts[2] != HTTP_VERSION_1_1) {
    FATAL("Http version not supported: %s", STR(parts[2]));
    return false;
  }

  if ((parts[0] != HTTP_METHOD_GET) && (parts[0] != HTTP_METHOD_POST) && (parts[0] != HTTP_METHOD_OPTIONS)) {
    FATAL("Http method not supported: %s", STR(parts[0]));
    return false;
  }

  if(!ParseURI(parts[1], uri)){
    FATAL("Invalid URI %s", STR(parts[1]));
    return false;
  }
  restMessage.request[HTTP_METHOD] = parts[0];
  restMessage.request["uri"] = uri.fullDocumentPath();
  if(restMessage.request[HTTP_METHOD] == HTTP_METHOD_GET){
    restMessage.request["param"] = (Variant) uri.parameters();
  }

  return true;

}

string BaseActionRouter::GetHTTPResponse(RestHTTPCode code) {
  string httpVersion = "HTTP/1.1 ";
  string responseString = "";
  switch(code){
    case 200:
      responseString = "200 OK";
      break;
    default:
      responseString = "500 Internal Server Error";
      break;
  }
  return (httpVersion + responseString);
}

void BaseActionRouter::ConvertToJSONMessage(RestCLIMessage &cliMsg, Variant &jsonMsg, string cuid) {
  uint64_t tId=0;

  if (cliMsg.request==V_MAP) {
    if (cliMsg.request.HasKey(REST_PAYLOAD_STR) &&
        cliMsg.request[REST_PAYLOAD_STR]==V_MAP) {
      if (cliMsg.request[REST_PAYLOAD_STR].HasKey(REST_TID_STR)) {
        tId=(uint64_t)cliMsg.request[REST_PAYLOAD_STR][REST_TID_STR];
        jsonMsg[REST_TID_STR]=tId;
      }

      jsonMsg[REST_URI_STR] = cliMsg.request[REST_PAYLOAD_STR][REST_URI_STR];
      if (cliMsg.request[REST_PAYLOAD_STR].HasKey(REST_PARAM_STR) &&
          cliMsg.request[REST_PAYLOAD_STR][REST_PARAM_STR] != V_NULL) {
        jsonMsg[REST_DATA_STR] = cliMsg.request[REST_PAYLOAD_STR][REST_PARAM_STR];
      }
    }
  }

  if (cliMsg.response.data.MapSize() != 0){
    if (cliMsg.response.data.HasKey(REST_ERROR_CODE)){
      jsonMsg[REST_ERROR_STR] = cliMsg.response.data;
      jsonMsg[REST_SUCCESS_STR] = false;
    }else{
      jsonMsg[REST_DATA_STR] = cliMsg.response.data;
      jsonMsg[REST_SUCCESS_STR] = true;
    }
  } else{
    jsonMsg[REST_SUCCESS_STR] = true;
  }
}
