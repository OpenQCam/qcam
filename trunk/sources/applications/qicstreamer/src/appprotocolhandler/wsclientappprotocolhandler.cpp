//thelib
#include "streaming/streamstypes.h"
#include "streaming/baseinnetstream.h"
#include "streaming/baseoutstream.h"
#include "application/baseclientapplication.h"
#include "protocols/http/outboundhttpwsprotocol.h"
#include "protocols/http/httpauthhelper.h"
//hardware
#include "system/systemmanager.h"
#include "system/nvramdefine.h"
#include "gpio/basegpiodevice.h"
#include "hardwaremanager.h"
//applications
#include "airservicetimer.h"
#include "servicetimer.h"
#include "websocketcmdhandler.h"
#include "websocket/wsclientsubprotocolfactory.h"
#include "appprotocolhandler/wsclientappprotocolhandler.h"

using namespace app_qicstreamer;

WSClientAppProtocolHandler::WSClientAppProtocolHandler(Variant &configuration)
: BaseWebSocketAppProtocolHandler (configuration),
  _regProtId(0) {

  //create subprotocol factory
  _pProtocolFactory = new WSClientSubProtocolFactory();
  if(!_pProtocolFactory) {
    ASSERT("Unable to create websocket subprotocol factory");
  }

  //create supported subprotocol map
  //INFO("%s", STR(configuration.ToString()));
  vector<uint64_t>allProtocols = _pProtocolFactory->HandledProtocols();
  FOR_VECTOR(allProtocols, i){
    _supportedProtocols[tagToString(allProtocols[i])] = allProtocols[i];
  }

}

WSClientAppProtocolHandler::~WSClientAppProtocolHandler() {
  if (_pCmdHandler) {
    delete _pCmdHandler;
  }
  if (_pProtocolFactory) {
    delete _pProtocolFactory;
  }
}

bool WSClientAppProtocolHandler::RegisterServiceTimer(string mapKey, BaseServiceTimer* pServiceTimer) {
  if (!pServiceTimer) {
    FATAL ("RegisterServiceTimer failed");
    return false;
  }
  else {
    _mapServiceTimer.insert(pair<string, BaseServiceTimer*>(mapKey, pServiceTimer));
  }

  return true;
}

bool WSClientAppProtocolHandler::Initialize() {
  _pCmdHandler=
      new WebSocketCmdHandler(static_cast<QICStreamerApplication*>(GetApplication()));

  return true;
}

bool WSClientAppProtocolHandler::IsServiceTimerExists(const string& name) {
  if (MAP_HAS1(_mapServiceTimer, name)) {
    return true;
  }
  return false;
}

bool WSClientAppProtocolHandler::InitiateConnection(ws_param_t& params, string ticket) {
  AirServiceTimer* pServiceTimer=NULL;

  if (IsServiceTimerExists(params.name)) {
    return false;
  }

  pServiceTimer = new AirServiceTimer (GetApplication(), params);
  if (pServiceTimer->TryConnect(ticket)) {
    pServiceTimer->EnqueueForTimeEvent(1);
    RegisterServiceTimer(params.name, static_cast<AirServiceTimer*>(pServiceTimer));
    return true;
  }
  else {
    delete pServiceTimer;
  }
  return false;
}

void WSClientAppProtocolHandler::EnableCloudConnection() {
  FOR_MAP (_mapServiceTimer, string, BaseServiceTimer*,it) {
    if (!MAP_VAL(it)->isRegisterProtocol())
      MAP_VAL(it)->EnableConnection();
  }
}

void WSClientAppProtocolHandler::DisableCloudConnection() {
  FOR_MAP (_mapServiceTimer, string, BaseServiceTimer*,it) {
    if (!MAP_VAL(it)->isRegisterProtocol())
      MAP_VAL(it)->DisableConnection();
  }
}

string WSClientAppProtocolHandler::ListAllServiceTimerState() {
  string result;
  FOR_MAP (_mapServiceTimer, string, BaseServiceTimer*, it) {
    if (MAP_VAL(it)) {
      result += MAP_VAL(it)->PrintState();
    }
    else {
      result += format ("result: [%s] is nil\n",  STR(MAP_KEY(it)));
    }
  }
  return result;
}

bool WSClientAppProtocolHandler::OnProtocolEvent(BaseProtocol* pFrom , int event, string name) {
  OutboundHTTPWSProtocol *pWSProtocol=NULL;
  string mapKey=name;
  ST_CONNECT_STATE timerState;
  BaseGPIODevice *pGPIODev=
    reinterpret_cast<BaseGPIODevice*>(HardwareManager::GetHardwareInstance(HT_GPIO));

  //protocol may be NULL
  if (pFrom) {
    pWSProtocol=static_cast<OutboundHTTPWSProtocol*>(pFrom);
    mapKey=pWSProtocol->GetWSProtocol();
  }

  if (!MAP_HAS1(_mapServiceTimer, mapKey) || !_mapServiceTimer[mapKey]) {
    FATAL ("serviceTimer no key or NULL:%s", STR(mapKey));
    return false;
  }

  switch (event) {
    case (WS_EVT_HTTP_CONNECTTING):
      timerState = ST_WSPROTOCOL_HTTP_CONNNECTING;
      break;
    case (WS_HTTPWSCONNECTION_DTOR):
      if (_mapServiceTimer[mapKey]->Type()==SERVICE_TIMER) {
        HTTPAuthHelper::ClearAllClientId();
        HTTPAuthHelper::SetAuthKey(SystemManager::GetNVRam("Login"),
                                   SystemManager::GetNVRam("Password"));
        _clientId.clear();
        if (pGPIODev) {
          pGPIODev->SetLedEvent(LED_EVENT_STATUS_CONNECTED_SERVER, false);
        }
      }
      _mapServiceTimer[mapKey]->OnConnectionFail();
      _mapServiceTimer[mapKey]->SetConnectState(ST_WAIT_TIMEOUT);
      _mapServiceTimer[mapKey]->UnRegisterProtocol();
      if (_mapServiceTimer[mapKey]->Type()==AIR_SERVICE_TIMER) {
        delete _mapServiceTimer[mapKey];
        _mapServiceTimer.erase(mapKey);
      }
      return true;
    case (WS_HTTP_PARSE_STATUS_ERROR): //server reject connection, no more trying connect
      if (_mapServiceTimer[mapKey]->isRegisterProtocol()) {
        _mapServiceTimer[mapKey]->SetConnectState(ST_CR_SNO_FAILED);
      }
      else
        _mapServiceTimer[mapKey]->SetConnectState(ST_CS_AUTH_FAILED);

      if (pWSProtocol)
        pWSProtocol->GracefullyEnqueueForDelete();
      break;
    case (WS_HTTP_PROTOCOL_CREATED):
      timerState = ST_WSPROTOCOL_HANDSHAKE;
      _mapServiceTimer[mapKey]->RegisterProtocol(pWSProtocol);
      break;
    case (WS_HANDSHAKE_SUCCESS):
      if (pGPIODev) {
        pGPIODev->SetLedEvent(LED_EVENT_STATUS_CONNECTED_SERVER, true);
      }
      if (_mapServiceTimer[mapKey]->isRegisterProtocol()) {
        timerState = ST_CR_WAIT_CUID;
        _regProtId=pFrom->GetId();
      }
      else {
        timerState = ST_CS_AUTH_SUCCESS;
      }
      break;
  }

   _mapServiceTimer[mapKey]->SetConnectState(timerState);
  return true;
}

bool WSClientAppProtocolHandler::ProcessWebsocketText(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length) {
  Variant result;
  uint32_t start=0;
  string raw = string((char *) pBuf, length);

  if (pFrom->GetId() == _regProtId) { //register protocol
    bool ret = Variant::DeserializeFromJSON(raw, result, start);
    if (ret) {
      if (GetApplication()->OnServerRegisterAuthed(result)) {
        DEBUG ("================================================================");
        DEBUG ("Registering to cloud server success");
        DEBUG ("================================================================");
        string mapKey=reinterpret_cast<OutboundHTTPWSProtocol*>(pFrom)->GetWSProtocol();
        delete _mapServiceTimer[mapKey];
        _mapServiceTimer.erase(mapKey);
      }
    }
    else {
      INFO ("DeserializeFromJSON fail");
      return false;
    }
  }
  else {
    if (_pCmdHandler) {
      /*FIXME(Recardo): handle the case multiple command in one websocket */
      _pCmdHandler->ParseCommand(static_cast<BaseHTTPWSProtocol*>(pFrom),
                                 pBuf,
                                 length);
    }
    else
      DEBUG ("no cmd handler");
  }
  return true;
}

bool WSClientAppProtocolHandler::ProcessWebsocketBinary(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length) {
  return true;
}

bool WSClientAppProtocolHandler::ProcessWebsocketPing(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length) {
  return true;
}

bool WSClientAppProtocolHandler::ProcessWebsocketPong(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length) {
  return true;
}


void WSClientAppProtocolHandler::UpdatePongTime(double time) {
  FOR_MAP (_mapServiceTimer, string, BaseServiceTimer*, it) {
    if (MAP_VAL(it)) {
      MAP_VAL(it)->UpdatePongTime(time);
    }
  }
}

BaseWSSubProtocol* WSClientAppProtocolHandler::CreateWSSubProtocol(string protocolName,
                                                                   Variant &parameters,
                                                                   BaseProtocol *pFarProtocol) {
  BaseProtocol *pSubProtocol = NULL;
  // check if protocol is supported
  if(!MAP_HAS1(_supportedProtocols, protocolName)){
    WARN("WebSocket subprotocol(%s) is not supported", STR(protocolName));
    return NULL;
  }

  //INFO("%s", STR(parameters.ToString()));
  pSubProtocol = _pProtocolFactory->SpawnProtocol(_supportedProtocols[protocolName], parameters);
  if(pSubProtocol == NULL) {
    WARN("Spawn protocol failed - %s", STR(protocolName));
    return NULL;
  }

  // Must set application before invoking subprotocol initialize()
  // Subprotocol initialize will use application instance
  pSubProtocol->SetApplication(GetApplication());
  pSubProtocol->SetFarProtocol(pFarProtocol);

  // Create stream
  if(!pSubProtocol->Initialize(parameters)) {
    FATAL("Protocol(%s) initialization failed", STR(tagToString(pSubProtocol->GetType())));
    delete pSubProtocol;
    return NULL;
  }

  if(pSubProtocol){
    INFO("ws %s subprotocol(%d) was created", STR(protocolName), pSubProtocol->GetId());
    return reinterpret_cast<BaseWSSubProtocol*>(pSubProtocol);
  }

  return NULL;

}
