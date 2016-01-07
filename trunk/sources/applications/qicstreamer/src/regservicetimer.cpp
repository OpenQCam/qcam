#include "regservicetimer.h"
//thelib
#include "protocols/protocolmanager.h"
#include "protocols/http/outboundhttpwsprotocol.h"
#include "system/nvramdefine.h"
#include "system/eventdefine.h"
//hardware
#include "system/systemmanager.h"
//application
#include "qicstreamerapplication.h"
#include "appprotocolhandler/wsclientappprotocolhandler.h"

using namespace app_qicstreamer;

RegServiceTimer::RegServiceTimer(BaseClientApplication* pClientApp,
                                 ws_param_t& params,
                                 string agentAddr)
: BaseServiceTimer (pClientApp, params, REG_SERVICE_TIMER),
  _agentAddr(agentAddr),
  _msgId(0) {

  if (!_wsParams.uri.empty()) {
    string ip=SystemManager::GetExternalIP();
    string hashCode=SystemManager::GetNVRam(NVRAM_HASHCODE);
    _wsParams.uri+="/?ac=" + SystemManager::GetDeviceSN()+"&ip="+ip+"&hv="+hashCode;
  }
}

RegServiceTimer::~RegServiceTimer() {
}

void RegServiceTimer::QueryCloudServer() {
  if (_wsParams.uri.empty() && _msgId==0 ) {
    string cmd="curl -s -k -m2 " + _agentAddr;
    DEBUG ("reg cmd:%s", STR(cmd));
    _msgId=UnixDomainSocketManager::ExecuteSystemCommand(cmd, GetCallbackId());
  }
}

void RegServiceTimer::OnCmdFinished(uint32_t msgId, uint8_t* pData,
                                 uint32_t dataLength) {
#define REG_AGENT "regAgent"
  string result((char*)pData, dataLength);
  Variant data;
  uint32_t start=0;
  _msgId=0;

  if (!Variant::DeserializeFromJSON(result, data, start)) {
    WARN ("invalid camReg json");
    return;
  }
  if (data!=V_MAP) {
    WARN ("invalid camReg addr");
    return;
  }

  if (data.HasKey(REG_AGENT) && data[REG_AGENT]==V_STRING) {
    string ip=SystemManager::GetExternalIP();
    string hashCode=SystemManager::GetNVRam(NVRAM_HASHCODE);
    string addr=(string)data[REG_AGENT];
    _wsParams.uri=addr+"/?ac=" + SystemManager::GetDeviceSN()+"&ip="+ip+"&hv="+hashCode;
  }
  else {
    WARN ("invalid camReg addr");
  }
}

bool RegServiceTimer::TryConnect(string ticket) {
  ST_CONNECT_STATE curConnState=GetConnectState();

  if (curConnState!=ST_WSPROTOCOL_INIT || _timerTick>0) {
    return true;
  }
  _timerTick=10;

  if (GetWSProtocol()==NULL && !_wsParams.uri.empty()) {
    if ((SystemManager::GetCUID().empty()) &&
        (SystemManager::GetPrivateKey().empty()) &&
        !(SystemManager::GetDeviceSN().empty())) {
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
      FATAL("cuid, privatekey, ac is invalid");
    }
  }
  return true;
}
