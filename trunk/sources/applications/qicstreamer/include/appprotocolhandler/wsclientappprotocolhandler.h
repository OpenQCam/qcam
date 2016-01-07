#ifndef _WSCLIENTAPPPROTOCOLHANDLER_H
#define _WSCLIENTAPPPROTOCOLHANDLER_H

#include "protocols/http/basewebsocketappprotocolhandler.h"
#include "baseservicetimer.h"

namespace app_qicstreamer {
class ServiceTimer;
class WebSocketCmdHandler;
class WSClientSubProtocolFactory;

class WSClientAppProtocolHandler
: public BaseWebSocketAppProtocolHandler {

  private:
    map <string, BaseServiceTimer*> _mapServiceTimer;
    uint32_t _regProtId;
    WSClientSubProtocolFactory *_pProtocolFactory;
    string _clientId;
    WebSocketCmdHandler *_pCmdHandler;

  protected:
  public:
    WSClientAppProtocolHandler (Variant &configuration);
    virtual ~WSClientAppProtocolHandler();

    bool Initialize();
    bool InitiateConnection(ws_param_t& params, string ticket);
    bool RegisterServiceTimer(string mapKey, BaseServiceTimer* pServiceTimer);
    bool IsServiceTimerExists(const string& name);
    void EnableCloudConnection();
    void DisableCloudConnection();

    string ListAllServiceTimerState();
    void UpdatePongTime(double time);

    virtual BaseWSSubProtocol* CreateWSSubProtocol(string protocolName,
                                                   Variant &parameters,
                                                   BaseProtocol *pProtocol);

    //Message Processing
    virtual bool OnProtocolEvent(BaseProtocol* pFrom, int command, string param="");
    //Websocket message processing
    virtual bool ProcessWebsocketText(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length);
    virtual bool ProcessWebsocketBinary(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length);
    virtual bool ProcessWebsocketPing(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length);
    virtual bool ProcessWebsocketPong(BaseProtocol *pFrom, uint8_t* pBuf, uint32_t length);

};
}; //namespace app_qicstreamer
#endif
