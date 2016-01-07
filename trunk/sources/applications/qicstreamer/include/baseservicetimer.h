#ifndef _BASESERVICETIMER_H
#define _BASESERVICETIMER_H

#include "netio/netio.h"
#include "protocols/http/outboundhttpwsprotocol.h"

#define RECONNECT_TIME 10

class OutboundHTTPWSProtocol;
class BaseClientApplication;

enum SERVICE_TYPE {
  REG_SERVICE_TIMER,
  SERVICE_TIMER,
  AIR_SERVICE_TIMER,
};

enum ST_CONNECT_STATE {
  //WebSocket connect state
  ST_WSPROTOCOL_INIT= 0x00,
  ST_WSPROTOCOL_HTTP_CONNNECTING = 0x01,
  ST_WSPROTOCOL_HANDSHAKE = 0x02,
  ST_WSPROTOCOL_CREATED = 0x03,
  /* Camera Register State */
  ST_CR_SNO_FAILED = 0x10,
  ST_CR_WAIT_CUID = 0x11,
  ST_CR_CUID_RECEIVED = 0x12,
  /* Service State */
  ST_CS_AUTH_FAILED = 0x21,
  ST_CS_AUTH_SUCCESS = 0x22,
  /* Time out state*/
  ST_WAIT_TIMEOUT = 0xFF, /* receive pong ack, wait time tick to resend ping */
};

namespace app_qicstreamer {

class BaseServiceTimer
: public IOTimer {
private:
  ST_CONNECT_STATE _connstate;
  BaseClientApplication *_pClientApp;
  OutboundHTTPWSProtocol *_pProtocol;
  double _lastPingTime;

  void HandleTimeout();

protected:
  int32_t _timerTick;
  ws_param_t _wsParams;
  uint32_t _tcpConnectorID;
  SERVICE_TYPE _type;

public:
  BaseServiceTimer (BaseClientApplication* pClientApp,
                    ws_param_t& wsParams, SERVICE_TYPE type);
  virtual ~BaseServiceTimer();
  virtual bool OnEvent(select_event &event);
  virtual bool TryConnect(string ticket)=0;
  virtual void OnConnectionFail();
  virtual void QueryCloudServer();

  void RegisterProtocol (OutboundHTTPWSProtocol *pProtocol);
  void UnRegisterProtocol();
  bool isRegisterProtocol();
  void EnableConnection();
  void UpdatePongTime(double time);
  void DisableConnection();
  BaseClientApplication* GetClientApp();
  OutboundHTTPWSProtocol* GetWSProtocol();
  void SetConnectState(ST_CONNECT_STATE connstate);
  ST_CONNECT_STATE GetConnectState();
  string GetWSProtocolName();
  ws_param_t GetConnParameters();
  string PrintState();
  SERVICE_TYPE Type();
};
}; //namespace app_qicstreamer


#endif
