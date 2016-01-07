#include "protocols/http/basehttpwsprotocol.h"

BaseHTTPWSProtocol::BaseHTTPWSProtocol(uint64_t protocolType)
  :BaseHTTPProtocol(protocolType),
   BaseWebSocketProtocol() {
}

BaseHTTPWSProtocol::~BaseHTTPWSProtocol() {

}

IOBuffer* BaseHTTPWSProtocol::ForceGetOutputBuffer() {
  return &_outputBuffer;
}

bool BaseHTTPWSProtocol::EnqueueForWSOutbound(uint8_t *pData, uint32_t len, bool fin, WS_OPCODES_TYPE opCode)
{
  WSHeader headerWS;
  IOBuffer headerBuf;
  bool isMask=false;
  uint32_t mask=0;

  if (!IsBufferFull()) {
    WSHEADER(headerWS, isMask, fin, opCode, mask, len);
    if (ConstructDataFrameHeader (headerWS, headerBuf)) {
      _outputBuffer.ReadFromInputBuffer(headerBuf, GETAVAILABLEBYTESCOUNT(headerBuf));
      _outputBuffer.ReadFromBuffer(pData, len);
    }
    else {
      return false;
    }
    return BaseProtocol::EnqueueForOutbound();
  }
  return true;
}
