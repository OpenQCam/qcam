
#include "baseservicetimer.h"
#include "appprotocolhandler/wsclientappprotocolhandler.h"
#include "protocols/protocolmanager.h"
#include "protocols/http/outboundhttpwsprotocol.h"
#include "system/systemmanager.h"
#include "system/nvramdefine.h"
#include "network/networkmanager.h"
#include "netio/netio.h"
#include "qicstreamerapplication.h"

using namespace app_qicstreamer;

BaseServiceTimer::BaseServiceTimer(BaseClientApplication* pClientApp,
                                   ws_param_t& params,
                                   SERVICE_TYPE type)
: IOTimer(),
  _pClientApp(pClientApp),
  _pProtocol(NULL),
  _lastPingTime(0),
  _wsParams(params),
  _tcpConnectorID(0),
  _type(type) {
  SetConnectState(ST_WSPROTOCOL_INIT);
}

BaseServiceTimer::~BaseServiceTimer() {
  if (_pClientApp != NULL) {
    _pClientApp=NULL;
  }
  if (_pProtocol != NULL) {
    if (!_pProtocol->IsEnqueueForDelete()) {
      _pProtocol->EnqueueForDelete();
      DEBUG ("BaseServiceTimer EnqueueForDelete");
    }
  }
}



ws_param_t BaseServiceTimer::GetConnParameters() {
  return _wsParams;
}

void BaseServiceTimer::SetConnectState(ST_CONNECT_STATE connstate) {

  switch (connstate) {
    case (ST_WSPROTOCOL_INIT):
      INFO ("[SetConnectState]ST_WSPROTOCOL_INIT");
      _timerTick = 10;
      break;
    case (ST_WSPROTOCOL_HTTP_CONNNECTING):
      INFO ("[SetConnectState]ST_WSPROTOCOL_HTTP_CONNNECTING");
      _timerTick = 5;
      break;
    case (ST_WSPROTOCOL_HANDSHAKE):
      INFO ("[SetConnectState]ST_WSPROTOCOL_HANDSHAKE");
      _timerTick = 5;
      break;
    case (ST_WSPROTOCOL_CREATED):
      INFO ("[SetConnectState]ST_WSPROTOCOL_CREATED");
      _timerTick = 5;
      break;
    case (ST_CR_SNO_FAILED):
      INFO ("[SetConnectState]ST_CR_SNO_FAILED");
      break;
    case (ST_CR_WAIT_CUID):
      INFO ("[SetConnectState]ST_CR_WAIT_CUID");
      _timerTick = 0x7fffffff;
      break;
    case (ST_CR_CUID_RECEIVED):
      INFO ("[SetConnectState]ST_CR_CUID_RECEIVED");
      _timerTick = 0x7fffffff;
      break;
   case (ST_CS_AUTH_FAILED):
      INFO ("[SetConnectState]ST_CS_AUTH_FAILED");
      _timerTick = 0x7fffffff;
      break;
    case (ST_CS_AUTH_SUCCESS):
      //FIXME(Tim): Ask recardo where is good place to init network service
      ((QICStreamerApplication *)_pClientApp)->InitAllNetworkServices();
      INFO ("[SetConnectState]ST_CS_AUTH_SUCCESS");
      _timerTick = 0x7fffffff;
      break;
    case (ST_WAIT_TIMEOUT):
      _timerTick = RECONNECT_TIME;
      break;
    default:
      return;
  }
  _connstate = connstate;
}

void BaseServiceTimer::RegisterProtocol (OutboundHTTPWSProtocol *pProtocol) {
  if (pProtocol) {
    _pProtocol = pProtocol;
  }
}

void BaseServiceTimer::UnRegisterProtocol() {
  _pProtocol = NULL;
}

string BaseServiceTimer::GetWSProtocolName() {
  return _wsParams.name;
}

void BaseServiceTimer::QueryCloudServer() {
}

bool BaseServiceTimer::OnEvent (select_event &event) {

  _timerTick --;
  if (!_wsParams.enabled) {
    return true;
  }

  HandleTimeout();
  if (_connstate == ST_WAIT_TIMEOUT) {
    if (_pProtocol != NULL) { //application level connection timeout
      _pProtocol->EnqueueForDelete();
      _pProtocol = NULL;
    }
    else if (_tcpConnectorID) { //socket level connection timeout
      map<uint32_t, IOHandler*> handlerMap= IOHandlerManager::GetActiveHandlers();
      if (MAP_HAS1(handlerMap, _tcpConnectorID)) {
        IOHandler *pHandler= handlerMap[_tcpConnectorID];
        if (pHandler && pHandler->GetType()==IOHT_TCP_CONNECTOR) {
          TCPConnector<OutboundHTTPWSProtocol> *pTCPConnector=reinterpret_cast<TCPConnector<OutboundHTTPWSProtocol>*>(pHandler);
          pTCPConnector->SetCloseSocket();
          IOHandlerManager::EnqueueForDelete(pHandler);
        }
      }
      _tcpConnectorID=0;
    }
    SetConnectState(ST_WSPROTOCOL_INIT);
  }
  //Update cloud timestamp
  if (_connstate==ST_WSPROTOCOL_INIT && _timerTick==2) {
    QueryCloudServer();
  }
  TryConnect("");
  //always return true or timer will be deleted
  return true;
}

bool BaseServiceTimer::isRegisterProtocol() {
  return _wsParams.isReg;
}

ST_CONNECT_STATE BaseServiceTimer::GetConnectState() {
  return _connstate;
}

BaseClientApplication* BaseServiceTimer::GetClientApp() {
  return _pClientApp;
}

OutboundHTTPWSProtocol* BaseServiceTimer::GetWSProtocol() {
  return _pProtocol;
}


SERVICE_TYPE BaseServiceTimer::Type() {
  return _type;
}

void BaseServiceTimer::EnableConnection() {
  _wsParams.enabled=true;
}

void BaseServiceTimer::DisableConnection() {
  _wsParams.enabled=false;
}

string BaseServiceTimer::PrintState() {
  string result="";
  result = format ("name:%s, enabled:%d, _connstate:%d, timerTick:%d",
      STR(_wsParams.name), _wsParams.enabled, _connstate, _timerTick);
  return result+"\n";
}

void BaseServiceTimer::OnConnectionFail() {
  NYI;
}

void BaseServiceTimer::UpdatePongTime(double time) {
  if (_pProtocol) {
    _pProtocol->UpdatePongTime(time);
  }
}

void BaseServiceTimer::HandleTimeout() {
  double currentTime=0;

  if (_pProtocol && (_connstate==ST_CS_AUTH_SUCCESS || _connstate==ST_CR_WAIT_CUID)) {
    GETTIMESTAMP(currentTime);
    //Send ping to server every 10s
    if ((currentTime - _lastPingTime) > 10000) {
      _pProtocol->EnqueueForPing();
      _lastPingTime = currentTime;
    }
    double lastPongTime=_pProtocol->GetLastPongTime();
    if ( (currentTime-lastPongTime)>_wsParams.expire) {
      WARN ("no pong received for %d, timeout:%s", _wsParams.expire, STR(_wsParams.name));
      _connstate = ST_WAIT_TIMEOUT;
    }
  }
  if (_timerTick <= 0 && _connstate != ST_WSPROTOCOL_INIT) {
    _connstate = ST_WAIT_TIMEOUT;
  }
}
