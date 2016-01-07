#include "protocols/http/websocket/basewssubprotocol.h"
#include "protocols/http/basehttpwsprotocol.h"
#include "protocols/protocoltypes.h"

BaseWSSubProtocol::BaseWSSubProtocol()
  : BaseProtocol(PT_WEBSOCKET_SUBPROTOCOL)
{
}
BaseWSSubProtocol::~BaseWSSubProtocol()
{
  INFO("delete sub protocol, id:(%d), type:(%s)",
       GetId(), STR(tagToString(GetType())));
}

bool BaseWSSubProtocol::AllowFarProtocol(uint64_t type)
{
  return type == PT_INBOUND_HTTP_WSCONNECTION ||
         type == PT_OUTBOUND_HTTP_WSCONNECTION;
}
bool BaseWSSubProtocol::AllowNearProtocol(uint64_t type)
{
  return false;
}
bool BaseWSSubProtocol::SignalInputData(int32_t recvAmount)
{
  NYI;
  return true;
}
bool BaseWSSubProtocol::SignalInputData(IOBuffer &buffer)
{
  NYI;
  return true;
}


bool BaseWSSubProtocol::EnqueueForWSOutbound(uint8_t *pData, uint32_t len,
                                             bool fin, WS_OPCODES_TYPE opCode)
{
  if (_pFarProtocol) {
    return reinterpret_cast<BaseHTTPWSProtocol*>(_pFarProtocol)->EnqueueForWSOutbound(pData, len, fin, opCode);
  }
  return false;
}

string BaseWSSubProtocol::GetProtocolName()
{
  return _protocolName;
}
