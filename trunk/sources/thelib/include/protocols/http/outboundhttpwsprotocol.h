#if (defined(HAS_PROTOCOL_HTTP) && defined(HAS_PROTOCOL_HTTPWSCONNECTION))

#ifndef _HTTPWSCONNECTPROTOCOL_H
#define _HTTPWSCONNECTPROTOCOL_H


#include "basehttpwsprotocol.h"

#define WS_CREATE_SERVICE_TIMER 0
#define WS_CONNECTION_SUCCESS 1
#define WS_HTTPWSCONNECTION_DTOR 2
#define WS_HTTP_PARSE_STATUS_ERROR 3
#define WS_HTTP_PROTOCOL_CREATED 4
#define WS_HANDSHAKE_SUCCESS 5
#define WS_EVT_CR_CUID_RECEIVED 6
#define WS_EVT_CS_AUTH_SUCCESS 7
#define WS_EVT_CS_AUTH_FAIL 8
#define WS_EVT_HTTP_CONNECTTING 9

typedef struct _ws_param_t {
  bool enabled;
  bool isReg;
  bool isSSL;
  uint16_t errorCode; //unable to push to airport error
  uint16_t eventCode; //stream connected to disconnected event code
  uint32_t expire;
  string uri;
  string name;
} ws_param_t ;

class IOBuffer;
class BaseClientApplication;
class BaseWebSocketAppProtocolHandler;
class BaseInNetStream;
class BaseOutNetStream;

class OutboundHTTPWSProtocol
: public BaseHTTPWSProtocol {
  private:
    BaseWebSocketAppProtocolHandler* _pWSHandler;
    Variant _httpHeaders;
    string _url;
    string _mapId;
    string _method;
    string _host;
    string _document;


  protected:
    //http function
    virtual string GetOutputFirstLine();
    virtual bool ParseFirstLine(string &line, Variant &firstLineHeader);
    virtual bool Authenticate() {return true;}

    //websocket function
    //virtual bool SignalInboundWebSocketText(IOBuffer &buffer, WebSocketDataFrame &dataFrame);
    //virtual bool SignalInboundWebSocketBinary(IOBuffer &buffer, WebSocketDataFrame &dataFrame);
    virtual bool SignalInboundWebSocketClose();
    virtual bool SignalDataProcessed();

  public:

    OutboundHTTPWSProtocol();
    virtual ~OutboundHTTPWSProtocol();

    virtual void SetApplication (BaseClientApplication *pApplication);
    virtual bool Initialize(Variant &parameters);
    virtual IOBuffer *GetOutputBuffer();
    virtual bool AllowFarProtocol(uint64_t type);
    virtual bool AllowNearProtocol(uint64_t type);
    virtual bool SignalInputData (IOBuffer &buffer);
    virtual bool EnqueueForOutbound();
//    virtual bool EnqueueForAuthOutbound(string result);
    bool EnqueueForPing();
    virtual bool IsBufferFull();

    void SetUrl(string url);
    string& GetUrl();
    BaseWebSocketAppProtocolHandler* GetProtocolHandler();
    void SetProtocolHandler(string handlerAddr);
    bool DoWSHandshake(Variant &parameters);

    /*static */
    static bool OpenWSConnection(ws_param_t wsParam,
                                 BaseClientApplication *pClientApp,
                                 uint32_t &tcpConnectID);

    static bool OpenWSSConnection(ws_param_t wsParam,
                                  BaseClientApplication *pClientApp,
                                  uint32_t &tcpConnectID);

    static bool SignalProtocolCreated(BaseProtocol *pProtocol, Variant &parameters);
};

#endif
#endif /* HAS_PROTOCOL_HTTP && HAS_PROTOCOL_HTTPWSCONNECTION */
