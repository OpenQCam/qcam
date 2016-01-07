#include "servicetimer.h"
//thelib
#include "protocols/http/httpauthhelper.h"
#include "protocols/protocolmanager.h"
#include "protocols/http/outboundhttpwsprotocol.h"
#include "system/systemmanager.h"
#include "system/nvramdefine.h"
//application
#include "appprotocolhandler/wsclientappprotocolhandler.h"
#include "qicstreamerapplication.h"

using namespace app_qicstreamer;

ServiceTimer::ServiceTimer(BaseClientApplication* pClientApp,
                           ws_param_t& params,
                           string agentAddr)
: BaseServiceTimer (pClientApp, params, SERVICE_TIMER),
  _agentAddr(agentAddr),
  _msgId(0) {

  string token;
  string cuid=SystemManager::GetCUID();
  string privateKey=SystemManager::GetPrivateKey();

  HTTPAuthHelper::GetSHA1Token(token, cuid, privateKey, "");
  //curl no need to encoding
  _agentAddr="\""+agentAddr+"/?cuid=" + cuid + "&hv=" + token +"\"";
  //url encoding
  for (uint32_t i=0; i<token.length(); i++) {
    if (token[i]!='+')
      _token += token[i];
    else
      _token += "%2B";
  }
  if (!_wsParams.uri.empty()) {
    _wsParams.uri+="/?cuid="+cuid+"&hv="+ _token;
  }
}

ServiceTimer::~ServiceTimer() {
}

void ServiceTimer::QueryCloudServer() {
  if (_wsParams.uri.empty() && _msgId==0 ) {
    string token;
    string cuid=SystemManager::GetCUID();
    string privateKey=SystemManager::GetPrivateKey();
    HTTPAuthHelper::GetSHA1Token(token, cuid, privateKey, "");
    //url encoding
    _token.clear();
    for (uint32_t i=0; i<token.length(); i++) {
      if (token[i]!='+')
        _token += token[i];
      else
        _token += "%2B";
    }
    _agentAddr="\""+SystemManager::GetNVRam(NVRAM_NETWORK_CLOUD_SERVERAGENT)
        +"/?cuid=" + cuid + "&hv=" + _token +"\"";

    string cmd="curl -s -k -m2 " + _agentAddr;
    DEBUG ("service cmd:%s", STR(cmd));
    _msgId=UnixDomainSocketManager::ExecuteSystemCommand(cmd, GetCallbackId());
  }

}

void ServiceTimer::OnCmdFinished(uint32_t msgId, uint8_t* pData,
                                 uint32_t dataLength) {
#define CAM_AGENT "camAgent"
  string result((char*)pData, dataLength);
  Variant data;
  uint32_t start=0;
  _msgId=0;

  if (!Variant::DeserializeFromJSON(result, data, start)) {
    WARN ("invalid service json");
    return;
  }
  if (data!=V_MAP) {
    WARN ("invalid service addr");
    return;
  }

  if (data.HasKey(CAM_AGENT) && data[CAM_AGENT]==V_STRING) {
    string addr=(string)data[CAM_AGENT];
    _wsParams.uri= addr+"/?cuid="+SystemManager::GetCUID()+"&hv="+ _token;
  }
  else {
    WARN ("invalid service addr");
  }
}

bool ServiceTimer::TryConnect(string ticket) {
  ST_CONNECT_STATE curConnState=GetConnectState();

  if (curConnState!=ST_WSPROTOCOL_INIT || _timerTick>0) {
    return true;
  }
  _timerTick=10;

  if (GetWSProtocol()==NULL && !_wsParams.uri.empty()) {
    if ((!SystemManager::GetCUID().empty()) &&
        (!SystemManager::GetPrivateKey().empty())) {
      if (_wsParams.isSSL) {
        INFO("DoConnect over SSL:%s, url[%s]", STR(_wsParams.name), STR(_wsParams.uri));
        if (!OutboundHTTPWSProtocol::OpenWSSConnection(_wsParams, GetClientApp(), _tcpConnectorID))
          return false;
      }
      else {
        INFO("DoConnect:%s, url[%s]", STR(_wsParams.name), STR(_wsParams.uri));
        _wsParams.uri+= "&ts="+ticket;
        if (!OutboundHTTPWSProtocol::OpenWSConnection(_wsParams, GetClientApp(), _tcpConnectorID))
          return false;
      }
    }
    else {
      FATAL("no cuid, privatekey");
    }
  }
  return true;
}
