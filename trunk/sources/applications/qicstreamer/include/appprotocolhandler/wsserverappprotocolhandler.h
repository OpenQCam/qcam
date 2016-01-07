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
// // Author  : Recardo.Cheng (recardo.cheng@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//

#ifndef _WSSERVERAPPPROTOCOLHANLDER_H
#define _WSSERVERAPPPROTOCOLHANLDER_H

#include "protocols/http/basewebsocketappprotocolhandler.h"

class WSServerSubProtocolFactory;

namespace app_qicstreamer {
  class WebSocketCmdHandler;

	class WSServerAppProtocolHandler
	: public BaseWebSocketAppProtocolHandler {
  private:
    WebSocketCmdHandler *_pCmdHandler;
    WSServerSubProtocolFactory *_pProtocolFactory;

  protected:
  public:
	  WSServerAppProtocolHandler(Variant &configuration);
	  virtual ~WSServerAppProtocolHandler();

    bool Initialize();
    //virtual bool ProcessMessage(BaseProtocol *pFrom, IOBuffer &buffer);
    //virtual bool OnProtocolEvent(BaseProtocol *pFrom, int command, string param="");
    //Websocket message processing
    //virtual bool ProcessWebsocketBinary(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length);
    //virtual bool ProcessWebsocketPing(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length);
    //virtual bool ProcessWebsocketPong(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length);
    virtual bool ProcessWebsocketText(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length);

    virtual BaseWSSubProtocol* CreateWSSubProtocol(string protocolName,
                                                   Variant &parameters,
                                                   BaseProtocol *pFar);
    virtual void RegisterProtocol(BaseProtocol *pProtocol);
    virtual void UnRegisterProtocol(BaseProtocol *pProtocol);
	};
}

#endif	/* _WSINPROTOCOLHANLDER_H */
