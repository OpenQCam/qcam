#include "protocols/http/basewebsocketappprotocolhandler.h"

BaseWebSocketAppProtocolHandler::BaseWebSocketAppProtocolHandler(Variant &configuration)
: BaseAppProtocolHandler(configuration) {
}

BaseWebSocketAppProtocolHandler::~BaseWebSocketAppProtocolHandler() {
}

void BaseWebSocketAppProtocolHandler::RegisterProtocol(BaseProtocol* pProtocol) {
  NYI;
}

void BaseWebSocketAppProtocolHandler::UnRegisterProtocol(BaseProtocol *pProtocol) {
  NYI;
}

bool BaseWebSocketAppProtocolHandler::ProcessMessage(BaseProtocol *pFrom,
                                                     IOBuffer &buffer) {
  NYI;
  return true;
}

bool BaseWebSocketAppProtocolHandler::OnProtocolEvent(BaseProtocol *pFrom,
                                                      int cmd,
                                                      string param) {
  NYI;
  return true;
}

bool BaseWebSocketAppProtocolHandler::ProcessWebsocketText(BaseProtocol *pFrom,
                                                           uint8_t* pBuf,
                                                           uint32_t length) {
  NYI;
  return true;
}

bool BaseWebSocketAppProtocolHandler::ProcessWebsocketBinary(BaseProtocol *pFrom,
                                                             uint8_t* pBuf,
                                                             uint32_t length) {
  NYI;
  return true;
}

bool BaseWebSocketAppProtocolHandler::ProcessWebsocketPing(BaseProtocol *pFrom,
                                                           uint8_t* pBuf,
                                                           uint32_t length) {
  NYI;
  return true;
}

bool BaseWebSocketAppProtocolHandler::ProcessWebsocketPong(BaseProtocol *pFrom,
                                                           uint8_t* pBuf,
                                                           uint32_t length) {
  NYI;
  return true;
}
