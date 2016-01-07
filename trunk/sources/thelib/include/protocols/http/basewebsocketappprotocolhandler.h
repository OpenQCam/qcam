
/* Base WebSocket Protocol Handler
   it passed data to protocol handler by ProcessMessage */

#ifndef _BASEWEBSOCKETPROTOCOLHANDLER_H
#define _BASEWEBSOCKETPROTOCOLHANDLER_H

#include "application/baseappprotocolhandler.h"
#include "protocols/http/websocket/basewssubprotocolfactory.h"
#include "protocols/http/websocket/basewssubprotocol.h"

class IOBuffer;

class BaseWebSocketAppProtocolHandler
: public BaseAppProtocolHandler {
  protected:
    map<string, uint64_t>_supportedProtocols;
  public:
    BaseWebSocketAppProtocolHandler (Variant &configuration);
    virtual ~BaseWebSocketAppProtocolHandler ();

    // new functions
    virtual BaseWSSubProtocol* CreateWSSubProtocol(string protocolName,
                                                   Variant &parameters,
                                                   BaseProtocol *pFar) = 0;
    //TODO(Tim) : below functions will be deprecated
    virtual void RegisterProtocol(BaseProtocol *pProtocol);
    virtual void UnRegisterProtocol(BaseProtocol *pProtocol);
    virtual bool ProcessMessage (BaseProtocol *pFrom, IOBuffer &buffer);
    virtual bool OnProtocolEvent (BaseProtocol *pFrom, int command, string param="");
    //Websocket message processing
    virtual bool ProcessWebsocketText(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length);
    virtual bool ProcessWebsocketBinary(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length);
    virtual bool ProcessWebsocketPing(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length);
    virtual bool ProcessWebsocketPong(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length);
};

#endif
