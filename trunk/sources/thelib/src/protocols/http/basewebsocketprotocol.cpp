#include "streaming/baseinnetstream.h"
#include "streaming/baseoutnetstream.h"
#include "protocols/http/basewebsocketprotocol.h"
#include "protocols/http/websocket/basewssubprotocol.h"

BaseWebSocketProtocol::BaseWebSocketProtocol()
: _wsState (HTTPWS_UNCONNECTED),
  _pSubProtocol(NULL) {
  GETTIMESTAMP(_lastPongTime);
}

BaseWebSocketProtocol::~BaseWebSocketProtocol()
{
  //1. no need to delete it, protocol manager can do it
  //  if (_pSubProtocol) {
  //    delete _pSubProtocol;
  //    _pSubProtocol=NULL;
  //  }

  //delte all data frame created
  while (!_wsDataFrame.empty()) {
    WebSocketDataFrame* pDataframe = _wsDataFrame.front();
    delete pDataframe;
    _wsDataFrame.pop();
  }
}

void BaseWebSocketProtocol::Print()
{
  DEBUG ("into:%s, %d", STR(_subProtocolName), _wsState);
}

bool BaseWebSocketProtocol::ParseDataFrame(queue<WebSocketDataFrame*>& wsDataFrame,
                                           IOBuffer& buffer) {

  uint32_t avail=GETAVAILABLEBYTESCOUNT(buffer);
  uint32_t bufOffset=0;
  uint8_t* pBuf=GETIBPOINTER(buffer);

  while (avail>=MIN_HEADER_LENGTH) {
    uint32_t reqLen=MIN_HEADER_LENGTH;
    bool fin=((pBuf[0]&FIN)==FIN);
    uint8_t opCode=pBuf[0]&OPCODE;
    uint32_t maskKey=0;
    uint8_t extLenBytes=0;

    //1. check RSV1,2,3 to be 0
    if (!((pBuf[0]&0x70)==0x00)) {
      WARN ("invalid rsv");
      return false;
    }

    //2. check 2nd byte is valid
    bool isMask=((pBuf[1]&MASK)==MASK);
    uint64_t length=(pBuf[1]&PAYLOAD);
    reqLen+=(isMask<<2);
    if (length==0x7E) {
      extLenBytes=2;
    }
    else if (length==0x7F){
      extLenBytes=8;
    }
    reqLen+=extLenBytes;
    //3. check header data is available
    if (avail<reqLen) {
      WARN ("ws header not available");
      return true;
    }

    if (length==0x7E) {
      length=ENTOHSP(pBuf+2);
    }
    else if (length==0x7F){
      length=ENTOHLLP(pBuf+2);
    }
    if (isMask) {
      maskKey=ENTOHLP(pBuf+2+extLenBytes);
    }
    if (length>MAX_WS_PAYLOADLENGTH) {
      WARN ("too large payload length:%llu", length);
      return false;
    }
    reqLen+=length;
    if (avail<reqLen) {
      WARN ("ws payload not available ");
      return true;
    }

    //4. unmask Payload data
    if (isMask) {
      uint8_t tempMaskey;
      uint8_t *buf;
      for(uint32_t i=0; i<length; i++) {
        tempMaskey=maskKey>>(CHAR_BIT*(3-(i&0x3)));
        buf=static_cast<uint8_t*>(pBuf+(reqLen-length)+i);
        *(buf)^=tempMaskey;
      }
    }

    //5. parse
    WebSocketDataFrame *pWSData=new WebSocketDataFrame();
    pWSData->_header.fin=fin;
    pWSData->_header.isMask=isMask;
    pWSData->_header.opCode=(WS_OPCODES_TYPE)opCode;
    pWSData->_header.mask=maskKey;
    pWSData->_header.payloadLength=length;
    pWSData->_payloadOffset=bufOffset+(reqLen-length);
    wsDataFrame.push(pWSData);

    //6. update buffer status
    avail-=reqLen;
    bufOffset+=reqLen;
    pBuf=GETIBPOINTER(buffer)+bufOffset;
  }
  return true;
}

bool BaseWebSocketProtocol::ProcessWSDataFrame(IOBuffer &buffer)
{
  WebSocketDataFrame *pWSData=NULL;
  uint32_t lastPayloadOffset=0;
  uint64_t lastPayloadLength=0;

  if (!_pSubProtocol) {
    DEBUG ("no subprotocol");
    return false;
  }
  /* FIXME(Recardo): handle fin and continuation websocket command */
  /* FIXME(Recardo): check largest data frame can be sent by device without continuation */
  if (!BaseWebSocketProtocol::ParseDataFrame(_wsDataFrame, buffer)) {
    WARN ("error parse data frame");
    return false;
  }

  while (!_wsDataFrame.empty()) {
    pWSData = _wsDataFrame.front();
    switch (pWSData->_header.opCode) {
      case (WS_OPCODE_TEXT_FRAME):
        _pSubProtocol->SignalInputTextFrame(buffer, *pWSData);
        break;
      case (WS_OPCODE_BINARY_FRAME):
        _pSubProtocol->SignalInputBinaryFrame(buffer, *pWSData);
        break;
      case (WS_OPCODE_CLOSE):
        SignalInboundWebSocketClose();
        break;
      case (WS_OPCODE_PING):
        SignalInboundWebSocketPing(buffer, *pWSData);
        break;
      case (WS_OPCODE_PONG):
        GETTIMESTAMP(_lastPongTime);
        break;
      case (WS_OPCODE_CONTINUATION):
        WARN ("ws continuation ignored");
        break;
      default:
        break;
    }
    _wsDataFrame.pop();
    lastPayloadOffset = pWSData->_payloadOffset;
    lastPayloadLength = pWSData->_header.payloadLength;
    delete pWSData;
  }

  //Ignore parsed data
  buffer.Ignore(lastPayloadOffset+lastPayloadLength);
  return true;
}

bool BaseWebSocketProtocol::ConstructDataFrameHeader(WSHeader& wsHeader, IOBuffer& headerBuffer)
{
  uint64_t payloadLength = wsHeader.payloadLength;
  uint8_t firstByte = 0; // FIN + OPCODE
  uint8_t secondByte = 0; // Mask + basePayLoadLength

  // Check operation code
  switch(wsHeader.opCode){
    case WS_OPCODE_CONTINUATION:
    case WS_OPCODE_TEXT_FRAME:
    case WS_OPCODE_BINARY_FRAME:

    // control extensions 8+
    case WS_OPCODE_CLOSE:
    case WS_OPCODE_PING:
    case WS_OPCODE_PONG:
      break;

    // others are reserverd: not support yet!
    default:
      FATAL("Websockets - Not support this kind of opcode");
      return false;
  }

  // first byte
  // FIN + OPCODE, FIXME:support fragment type message
  firstByte = ((wsHeader.fin<<7) | wsHeader.opCode);
  headerBuffer.ReadFromByte(firstByte);

  // second byte & extPayloadLength
  if(payloadLength < 126){
    secondByte = payloadLength;
    headerBuffer.ReadFromByte(secondByte);
  }
  else if(payloadLength < 65536) {
    if (wsHeader.opCode & 8) // control frames are not allowed to have big lengths
      return false; // illegal ctl length;
    secondByte = 126;
    headerBuffer.ReadFromByte(secondByte);
    // add 2 bytes for extPayloadLength
    headerBuffer.ReadFromByte((uint8_t)((((uint16_t)payloadLength & 0xFF00) >> 8)) );
    headerBuffer.ReadFromByte((uint8_t)(((uint16_t)payloadLength) & 0x00FF));
  }
  else {
    if (wsHeader.opCode & 8) // control frames are not allowed to have big lengths
      return false; // illegal ctl length;

    secondByte = 127;
    headerBuffer.ReadFromByte(secondByte);

    // add 8 bytes for extPayloadLength
    headerBuffer.ReadFromByte((uint8_t)((((uint64_t)payloadLength) & 0xFF00000000000000LL) >> 56) );
    headerBuffer.ReadFromByte((uint8_t)((((uint64_t)payloadLength) & 0x00FF000000000000LL) >> 48) );
    headerBuffer.ReadFromByte((uint8_t)((((uint64_t)payloadLength) & 0x0000FF0000000000LL) >> 40) );
    headerBuffer.ReadFromByte((uint8_t)((((uint64_t)payloadLength) & 0x000000FF00000000LL) >> 32) );
    headerBuffer.ReadFromByte((uint8_t)((((uint64_t)payloadLength) & 0x00000000FF000000LL) >> 24) );
    headerBuffer.ReadFromByte((uint8_t)((((uint64_t)payloadLength) & 0x0000000000FF0000LL) >> 16) );
    headerBuffer.ReadFromByte((uint8_t)((((uint64_t)payloadLength) & 0x000000000000FF00LL) >>  8) );
    headerBuffer.ReadFromByte((uint8_t)(((uint64_t)payloadLength) & 0x00000000000000FFLL) );
  }
  return true;
}

bool BaseWebSocketProtocol::SignalInboundWebSocketPing(IOBuffer &buffer, WebSocketDataFrame &dataframe)
{
  uint32_t offset = dataframe._payloadOffset;
  uint8_t *pBuf = (uint8_t*)(GETIBPOINTER(buffer)+offset);

  EnqueueForWSOutbound (pBuf, dataframe._header.payloadLength, true, WS_OPCODE_PONG);
  return true;
}

void BaseWebSocketProtocol::UpdatePongTime(double ts) {
  _lastPongTime=ts;
}

//void BaseWebSocketProtocol::RegisterSubProtocol(BaseWSSubProtocol *pSubProtocol) {
//  _pSubProtocol=pSubProtocol;
//}
