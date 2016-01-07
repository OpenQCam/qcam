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

#ifdef HAS_PROTOCOL_CLI

#include "appprotocolhandler/restcliappprotocolhandler.h"
//thelib
#include "clidefine.h"
#include "actionhandler/actionrouter.h"
#include "protocols/cli/statuscode.h"
#include "protocols/cli/restclidefine.h"
#include "protocols/cli/inboundjsonrestcliprotocol.h"
//hardware
#include "system/systemmanager.h"
//application
#include "qicstreamerapplication.h"

using namespace app_qicstreamer;


RestCLIAppProtocolHandler::RestCLIAppProtocolHandler(Variant &configuration)
: BaseRestCLIAppProtocolHandler(configuration)
{
  Variant acceptors=configuration[CONF_ACCEPTORS];
  FOR_MAP(acceptors, string, Variant, i) {
    if (MAP_VAL(i) == V_MAP) {
      Variant acceptConfig=MAP_VAL(i);
      if (acceptConfig.HasKey(CONF_PROTOCOL) && acceptConfig.HasKey(CONF_API) &&
          acceptConfig.HasKey(CONF_PORT)) {
        if (acceptConfig[CONF_PROTOCOL] == CONF_PROTOCOL_INBOUND_HTTP_REST_CLI) {
          uint32_t port=(uint32_t)acceptConfig[CONF_PORT];
          _allowedAPI[port] = (string)acceptConfig[CONF_API];
        }
      }
    }
  }
}

RestCLIAppProtocolHandler::~RestCLIAppProtocolHandler() {
}

void RestCLIAppProtocolHandler::SetApplication(BaseClientApplication *pApplication) {

  BaseAppProtocolHandler::SetApplication(pApplication);

}

bool RestCLIAppProtocolHandler::ProcessMessage(BaseProtocol *pFrom, Variant &message){
  bool ret=true;
  string host = message[HTTP_HEADERS][HTTP_HEADERS_HOST];
  vector<string> hostSplit;
  RestCLIMessage restMessage;
  Variant request = (Variant) message[HTTP_FIRST_LINE];
  Variant response;
  InboundJSONRestCLIProtocol *pRestProtocol = (InboundJSONRestCLIProtocol *) pFrom;

  split(host, ":", hostSplit);
  uint32_t port=stringToNum<uint32_t>(hostSplit[1]);
  QICStreamerApplication *pApp = (QICStreamerApplication *) GetApplication();
  ActionRouter *pActionRouter = pApp->GetActionRouter();
  restMessage.response.statusCode = HC_400_BAD_REQUEST;

  if (pActionRouter != NULL) {
    URI uri;
    restMessage.protocol = pFrom;
    // 1. parse uri
    BaseActionRouter::ParseURI(request[HTTP_URL], uri);
    restMessage.request[REST_PAYLOAD_STR][REST_URI_STR] = uri.fullDocumentPath();

    // 2. check the api is allowed with specified port
    if (MAP_HAS1(_allowedAPI, port)) {
      if (_allowedAPI[port] != uri.fullDocumentPath()) {
        restMessage.response.data[REST_ERROR_CODE] = EC_101_INVALID_API_REQUEST;
        restMessage.response.data[REST_ERROR_DESCRIPTION] = "Invalid REST request";
        restMessage.request[REST_PAYLOAD_STR][REST_URI_STR] = request[HTTP_URL];
        goto response;
      }
    }

    // 3. get paramters
    if (request[HTTP_METHOD] == HTTP_METHOD_GET) { //get method
      restMessage.request[REST_PAYLOAD_STR][REST_PARAM_STR] = uri.parameters();
      restMessage.request[REST_ISRO_STR] = true;
    } else { //post method
      IOBuffer *pBuf = pRestProtocol->GetInputBuffer();
      if (pBuf != NULL) {
        string postContent((char *)GETIBPOINTER(*pBuf), GETAVAILABLEBYTESCOUNT(*pBuf));
        uint32_t start = 0;
        restMessage.request[REST_ISRO_STR] = false;
        /*if (Variant::DeserializeFromJSON(postContent, restMessage.request[REST_PAYLOAD_STR], start)) {
          //check the parameters
          if (restMessage.request[REST_PAYLOAD_STR].HasKey(REST_PARAM_STR)) {
            if (restMessage.request[REST_PAYLOAD_STR][REST_PARAM_STR]!=V_MAP) {
              restMessage.response.data[REST_ERROR_CODE] = EC_108_INVALID_PARAMETERS;
              restMessage.response.data[REST_ERROR_DESCRIPTION] = "Invalid parameters";
              goto response;
            }
          }
          //check uri parameter for http post method
          if (!(restMessage.request[REST_PAYLOAD_STR]).HasKey(REST_URI_STR)) {
            restMessage.response.data[REST_ERROR_CODE] = EC_108_INVALID_PARAMETERS;
            restMessage.response.data[REST_ERROR_DESCRIPTION] = "Invalid parameters";
            goto response;
          }
        }
        else {*/
        if (!Variant::DeserializeFromJSON(postContent, restMessage.request[REST_PAYLOAD_STR][REST_PARAM_STR], start)) {
          restMessage.response.statusCode = HC_400_BAD_REQUEST;
          restMessage.response.data[REST_ERROR_CODE] = EC_105_INVALID_JSON_FORMAT;
          restMessage.response.data[REST_ERROR_DESCRIPTION] = "Invalid JSON format";
          goto response;
        }
      }
      pBuf->IgnoreAll();
    } //post method

    if (!pActionRouter->RouteRequestAction(restMessage)) {
      restMessage.response.data[REST_ERROR_CODE] = EC_101_INVALID_API_REQUEST;
      restMessage.response.data[REST_ERROR_DESCRIPTION] = "Invalid REST request";
    }
  } else { //Action Router == null
    return false;
  }

response:
  BaseActionRouter::ConvertToJSONMessage(restMessage, response, SystemManager::GetCUID());

  if (restMessage.request[REST_PARAM_STR].HasKey(QIC_CLI_JSONP_CALLBACK)) {
    return pRestProtocol->SendJsonpMessage(restMessage.response.statusCode,
                                           restMessage.request[REST_PARAM_STR][QIC_CLI_JSONP_CALLBACK],
                                           response);
  } else {
    //Send Command resposne
    ret=pRestProtocol->SendMessage(restMessage.response.statusCode, response);
    //Send Sync
    if (restMessage.request[REST_PAYLOAD_STR].HasKey(REST_PARAM_STR)) {
      restMessage.response.data[REST_DATA_STR] = restMessage.request[REST_PAYLOAD_STR][REST_PARAM_STR];
    }
    //DEBUG ("sync data:%s", STR(restMessage.response.data.ToString()));
    if (restMessage.response.statusCode==HC_202_ACCEPTED) {
      string respBinary;
      response.SerializeToJSON(respBinary);
      restMessage.response.data[REST_PAYLOAD_STR]=respBinary;
      QICStreamerApplication *pApp=reinterpret_cast<QICStreamerApplication*>(GetApplication());
      pApp->SendSync(restMessage.response.data);
    }
  }
  return ret;
}
#endif  /* HAS_PROTOCOL_CLI */
