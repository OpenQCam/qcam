#include "websocketcmdhandler.h"
//thelibs
#include "protocols/http/basehttpwsprotocol.h"
#include "protocols/cli/restclidefine.h"
//hardware
#include "system/systemmanager.h"
//applications
#include "actionhandler/actionrouter.h"

using namespace app_qicstreamer;

WebSocketCmdHandler::WebSocketCmdHandler(QICStreamerApplication *pApp){
  _pActionRouter=NULL;
  _pApp=pApp;
}

WebSocketCmdHandler::~WebSocketCmdHandler() {
}

void WebSocketCmdHandler::SendCmdResponse(RestCLIMessage& msg) {
  string responseJSONString;
  Variant response;

  //DEBUG ("msg:%s", STR(msg.request.ToString()));
  BaseActionRouter::ConvertToJSONMessage(msg, response, SystemManager::GetCUID());
  //FIXME(recardo): time consuming almost 100us for x86 platform, est 100*100 = 10ms for MIPS platform
  response.SerializeToJSON(responseJSONString);
   if (_pApp) {
     uint64_t utid=0;
     if (msg.request==V_MAP && msg.request.HasKey(REST_UTID_STR)) {
       utid=msg.request[REST_UTID_STR];
     }
     _pApp->SendCmdResp(responseJSONString, utid, msg.protocol->GetId());
     if (msg.response.statusCode == HC_202_ACCEPTED) {
       msg.response.data[REST_PAYLOAD_STR]=responseJSONString;
       _pApp->SendSync(msg.response.data);
     }
   }

}

//command request and nonblocking response
bool WebSocketCmdHandler::ParseCommand(BaseHTTPWSProtocol *pProtocol,
                                       uint8_t *pData, uint32_t length) {
  uint32_t start=0;

  _pActionRouter=
      reinterpret_cast<QICStreamerApplication*>(pProtocol->GetApplication())->GetActionRouter();
  if (_pActionRouter) {
    string raw=string((char*)pData, length);

    while (start<length) {
      RestCLIMessage msg;
      msg.protocol = pProtocol;
      //1. Deserialize from JSON
      if ((!Variant::DeserializeFromJSON(raw, msg.request, start)) ) {
        msg.response.data[REST_ERROR_CODE]=EC_105_INVALID_JSON_FORMAT;
        msg.response.data[REST_ERROR_DESCRIPTION]="Invalid json format";
        SendCmdResponse(msg);
        return false;
      }
      if (msg.request!=V_MAP) {
        msg.response.data[REST_ERROR_CODE]=EC_108_INVALID_PARAMETERS;
        msg.response.data[REST_ERROR_DESCRIPTION]="Invalid request";
        SendCmdResponse(msg);
        continue;
      }
      if (!msg.request.HasKey(REST_PAYLOAD_STR)) {
        msg.response.data[REST_ERROR_CODE]=EC_108_INVALID_PARAMETERS;
        msg.response.data[REST_ERROR_DESCRIPTION]="Invalid payload";
        SendCmdResponse(msg);
        continue;
      }
      if (msg.request[REST_PAYLOAD_STR]!=V_MAP){
        msg.response.data[REST_ERROR_CODE]=EC_108_INVALID_PARAMETERS;
        msg.response.data[REST_ERROR_DESCRIPTION]="Payload must be object";
        SendCmdResponse(msg);
        continue;
      }
      if (!msg.request[REST_PAYLOAD_STR].HasKey(REST_URI_STR) ||
          !msg.request.HasKey(REST_UTID_STR) ||
          !msg.request.HasKey(REST_ISRO_STR)) {
        msg.response.data[REST_ERROR_CODE]=EC_109_MISSING_MANDATORY_PARAMETERS;
        msg.response.data[REST_ERROR_DESCRIPTION]="Missing uri/isRO/utId";
        SendCmdResponse(msg);
        continue;
      }
      if (msg.request[REST_PAYLOAD_STR].HasKey(REST_TID_STR) &&
         (msg.request[REST_PAYLOAD_STR][REST_TID_STR]!=V_INT64)) {
         msg.response.data[REST_ERROR_CODE]=EC_108_INVALID_PARAMETERS;
         msg.response.data[REST_ERROR_DESCRIPTION]="TID must be number";
         SendCmdResponse(msg);
        continue;
      }

      if(_pActionRouter->RouteRequestAction(msg)){
        if (msg.response.statusCode == HC_204_NO_CONTENT) { //blocking command
          Variant* pScriptParam=NULL;
          SystemCmdHeader cmdHeader={true, msg.response.data[REST_CMD_STR],
            msg.protocol->GetId(), msg.request[REST_UTID_STR]};

          if (msg.response.data.HasKey(REST_SCRIPTPARAM_STR)) {
            pScriptParam=&(msg.response.data[REST_SCRIPTPARAM_STR]);
          }

          if (msg.response.data[REST_SYNC_STR]) {
            UnixDomainSocketManager::ExecuteSyncSystemCmd(msg.cbId, cmdHeader,
                                                          &msg.request[REST_PAYLOAD_STR],
                                                          pScriptParam);
          }
          else {
            UnixDomainSocketManager::ExecuteSystemCommand(msg.cbId, cmdHeader,
                                                          &msg.request[REST_PAYLOAD_STR],
                                                          pScriptParam);
          }
        }
        else { //non block command
          SendCmdResponse(msg);
        }
      }//RouteRequestAction
    }//while start<length
  }//Action router
  return true;
}
