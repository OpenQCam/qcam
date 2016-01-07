
#ifndef _BASEWEBSOCKETPROTOCOL_H
#define _BASEWEBSOCKETPROTOCOL_H
#include "common.h"

#define FIN 0x80
#define RSV1 0x40
#define RSV3 0x20
#define RSV2 0x10
#define OPCODE 0x0F
#define MASK   0x80
#define PAYLOAD 0x7F
#define PAYLOAD16BIT 0x7E //126
#define PAYLOAD64BIT 0x7F //127
#define MAX_HEADER_LENGTH 14 // 14 bytes
#define MIN_HEADER_LENGTH 2 // 2 bytes

#define MAX_WS_PAYLOADLENGTH (10*1024*1024)

#define WSHEADER(wsHeader, _isMask, _fin, _opCode, _mask, _length)\
{\
  wsHeader.isMask=_isMask;\
  wsHeader.fin=_fin;\
  wsHeader.opCode=_opCode;\
  wsHeader.mask=_mask;\
  wsHeader.payloadLength=_length;\
}

#define ISCONTROLFRAME(opCode) (((opCode)==WS_OPCODE_PING)||((opCode)==WS_OPCODE_PONG)||((opCode)==WS_OPCODE_CLOSE))

class BaseStream;
class BaseInNetStream;
class BaseOutNetStream;
class IOBuffer;
class WSSubProtocolHandler;

typedef enum _WS_OPCODES_TYPE{
  WS_OPCODE_CONTINUATION = 0,
  WS_OPCODE_TEXT_FRAME = 1,
  WS_OPCODE_BINARY_FRAME = 2,
  /* control extensions 8+ */
  WS_OPCODE_CLOSE = 8,
  WS_OPCODE_PING = 9,
  WS_OPCODE_PONG = 0xa,
} WS_OPCODES_TYPE;

typedef enum _WS_CONNECT_TYPE {
  HTTPWS_UNCONNECTED=0,
  HTTPWS_CONNECT_WAIT_RESPONSE=1,
  HTTPWS_CONNECT_ALIVE=2,
  HTTPWS_CONNECT_REJECT=0xff,
} WS_CONNECT_STATE;

typedef struct _WSHeader {
  bool isMask;
  bool fin;
  WS_OPCODES_TYPE opCode;
  uint32_t mask; //the mask is key is rotatable
  uint64_t payloadLength;
} WSHeader;

class WebSocketDataFrame {
public:
  WSHeader _header;
  uint32_t _payloadOffset;
};

class BaseWSSubProtocol;

class BaseWebSocketProtocol {

private:
  double _lastPongTime;

protected:
  WS_CONNECT_STATE _wsState;
  BaseWSSubProtocol* _pSubProtocol;
  string _subProtocolName;
  WSHeader _wsOutHeader;
  queue<WebSocketDataFrame*> _wsDataFrame;

private:

protected:
  static bool ConstructDataFrameHeader (WSHeader& _wsHeader, IOBuffer& _headerBuffer);
  virtual bool SignalInboundWebSocketPing(IOBuffer &buffer, WebSocketDataFrame &dataframe);
  virtual bool SignalInboundWebSocketClose()=0;
  virtual bool ProcessWSDataFrame(IOBuffer &buffer);
  //virtual void RegisterSubProtocol(BaseWSSubProtocol *pSubProtocol);

  static bool ParseDataFrame (queue<WebSocketDataFrame*>& wsDataFrame, IOBuffer& buffer);

public:
  BaseWebSocketProtocol();
  virtual ~BaseWebSocketProtocol();

  virtual bool EnqueueForWSOutbound(uint8_t *pData, uint32_t len, bool fin, WS_OPCODES_TYPE opCode)=0;


  virtual void Print();

  string GetWSProtocol() { return _subProtocolName;}
  void SetWSProtocol(string subProtocolName) { _subProtocolName = subProtocolName;}
  WS_CONNECT_STATE GetWSState() { return _wsState;}
  void SetWSState(WS_CONNECT_STATE wsState) {_wsState=wsState;}
  double GetLastPongTime() {return _lastPongTime;}
  void UpdatePongTime(double ts);
};

#endif
