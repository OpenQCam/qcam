#include "airservicetimer.h"
//thelib
#include "protocols/http/httpauthhelper.h"
#include "protocols/protocolmanager.h"
#include "protocols/http/outboundhttpwsprotocol.h"
#include "system/eventdefine.h"
//hardware
#include "system/systemmanager.h"
//application
#include "appprotocolhandler/wsclientappprotocolhandler.h"
#include "qicstreamerapplication.h"

using namespace app_qicstreamer;

AirServiceTimer::AirServiceTimer(BaseClientApplication* pClientApp, ws_param_t& params)
: BaseServiceTimer (pClientApp, params, AIR_SERVICE_TIMER) {
  _timerTick=0;
  _bTried=false;
}

AirServiceTimer::~AirServiceTimer() {
}

string AirServiceTimer::GetCloudServer() {
  return _wsParams.uri;
}

void AirServiceTimer::OnConnectionFail() {
  QICStreamerApplication* pApp=
      reinterpret_cast<QICStreamerApplication*>(GetClientApp());
  if (!pApp) return;

  if (GetConnectState()!=ST_CS_AUTH_SUCCESS) {
    uint64_t ts;
    GETTIMESTAMP(ts);
    pApp->SendEvent(_wsParams.eventCode, ts, "");
  }
}

bool AirServiceTimer::TryConnect(string ticket) {

  if (_bTried) return true;
  _bTried=true;
  if (!_wsParams.uri.empty()) {
    string token, urltoken;
    HTTPAuthHelper::GetSHA1Token(token, SystemManager::GetCUID(), SystemManager::GetPrivateKey(), ticket);
    //url encoding
    for (uint32_t i=0; i<token.length(); i++) {
      if (token[i]!='+')
        urltoken += token[i];
      else
        urltoken += "%2B";
    }
    _wsParams.uri+= "/?cuid="+ SystemManager::GetCUID()+ "&hv="+ urltoken;

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
        if (!OutboundHTTPWSProtocol::OpenWSConnection(_wsParams, GetClientApp(), _tcpConnectorID)){
          return false;
        }
      }
    }
    else {
      FATAL("no cuid, privatekey");
      return false;
    }
  }
  else {
    DEBUG ("uri is empty");
    return false;
  }
  return true;
}
