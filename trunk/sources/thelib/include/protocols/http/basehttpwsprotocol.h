#ifndef _BASEHTTPWSPROTOCOL_H
#define _BASEHTTPWSPROTOCOL_H

#include "protocols/http/basehttpprotocol.h"
#include "protocols/http/basewebsocketprotocol.h"

#define MAXAUDIOBUFSIZE (128*1024)

class BaseHTTPWSProtocol
: public BaseHTTPProtocol, public BaseWebSocketProtocol {
  public:
    BaseHTTPWSProtocol(uint64_t protocolType);
    virtual ~BaseHTTPWSProtocol();

    virtual bool IsBufferFull() { return false;}
    virtual IOBuffer* ForceGetOutputBuffer();
    virtual bool EnqueueForWSOutbound(uint8_t *pData, uint32_t len, bool fin, WS_OPCODES_TYPE opCode);
};

#endif
