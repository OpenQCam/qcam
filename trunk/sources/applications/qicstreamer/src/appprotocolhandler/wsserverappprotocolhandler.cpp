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
//thelib
#include "streaming/streamstypes.h"
#include "application/baseclientapplication.h"
#include "protocols/http/inboundhttpwsprotocol.h"

//applications
#include "websocketcmdhandler.h"
#include "websocket/wsserversubprotocolfactory.h"
#include "appprotocolhandler/wsserverappprotocolhandler.h"

using namespace app_qicstreamer;
WSServerAppProtocolHandler::WSServerAppProtocolHandler(Variant &configuration)
: BaseWebSocketAppProtocolHandler(configuration) {

  //create subprotocol factory
  _pProtocolFactory = new WSServerSubProtocolFactory();
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

WSServerAppProtocolHandler::~WSServerAppProtocolHandler() {
  if (_pCmdHandler) {
    delete _pCmdHandler;
  }
  if (_pProtocolFactory) {
    delete _pProtocolFactory;
  }
}

bool WSServerAppProtocolHandler::Initialize() {

  _pCmdHandler=
      new WebSocketCmdHandler(static_cast<QICStreamerApplication*>(GetApplication()));
  return true;

//
//  if (!_configuration.HasKeyChain(V_MAP, true, 2, CONF_LUA_DEFAULT,
//                                  CONF_WEBSOCKET_INBOUND_CONFIG)) {
//    return false;
//  }
//  Variant wsInConfig=_configuration[CONF_LUA_DEFAULT][CONF_WEBSOCKET_INBOUND_CONFIG];
//  FOR_MAP(wsInConfig, string, Variant, i) {
//    if (MAP_VAL(i)!=V_MAP ||
//        !MAP_VAL(i).HasKey(CONF_WEBSOCKET_PROTOCOL_NAME) ||
//        !MAP_VAL(i).HasKey(CONF_WEBSOCKET_STREAM_NAME) ||
//        !MAP_VAL(i).HasKey(CONF_WEBSOCKET_OUTSTREAM_TYPE)) {
//      return false;
//    }
//
//    Variant subConfig=(MAP_VAL(i));
//    string subProtocolName = (string)subConfig[CONF_WEBSOCKET_PROTOCOL_NAME];
//    _allowProtocol[subProtocolName]=subConfig;
//  }
//  return true;
}


//bool WSServerAppProtocolHandler::ProcessMessage(BaseProtocol *pFrom, IOBuffer &buffer) {
//  DEBUG ("processMessage:%s", STR(buffer.ToString()));
//  return true;
//}
//
//bool WSServerAppProtocolHandler::OnProtocolEvent(BaseProtocol *pFrom, int event, string param) {
//  return true;
//}
//

//
//bool WSServerAppProtocolHandler::ProcessWebsocketBinary(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length) {
//  NYI;
//  return true;
//}
//
//bool WSServerAppProtocolHandler::ProcessWebsocketPing(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length) {
//  return true;
//}
//
//bool WSServerAppProtocolHandler::ProcessWebsocketPong(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length) {
//  return true;
//}

//BaseWSSubProtocol* WSServerAppProtocolHandler::CreateWSSubProtocol(BaseProtocol* pFrom, string name) {
//  BaseWSSubProtocol *pResult=NULL;
//  if (!MAP_HAS1(_allowProtocol, name)) {
//    DEBUG ("Unsupported websocket sub protocol:%s", STR(name));
//    return NULL;
//  }
//  Variant configuration=_allowProtocol[name];
//  if (configuration!=V_MAP)
//    return NULL;
//
//  uint64_t type=stringToUint64((string)configuration[CONF_WEBSOCKET_OUTSTREAM_TYPE]);
//  pResult=factory.SpawnProtocol(type, GetApplication(), (BaseHTTPWSProtocol*)pFrom, configuration);
//  return pResult;
//}

bool WSServerAppProtocolHandler::ProcessWebsocketText(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length) {
  if (_pCmdHandler) {
    _pCmdHandler->ParseCommand(reinterpret_cast<BaseHTTPWSProtocol*>(pFrom),
                               pBuf, length);
  }
  else
    WARN ("no cmd handler");
  return true;
}

void WSServerAppProtocolHandler::RegisterProtocol(BaseProtocol *pProtocol) {
}


void WSServerAppProtocolHandler::UnRegisterProtocol(BaseProtocol *pProtocol) {
}

BaseWSSubProtocol* WSServerAppProtocolHandler::CreateWSSubProtocol(string protocolName,
                                                                   Variant &parameters,
                                                                   BaseProtocol *pFar)
{
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
