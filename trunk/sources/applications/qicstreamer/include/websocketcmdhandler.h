#ifndef _WEBSOCKETCMDHANDLER_H
#define _WEBSOCKETCMDHANDLER_H

#include "qicstreamerapplication.h"
class ActionRouter;
class BaseHTTPWSProtocol;
struct RestCLIMessage;

namespace app_qicstreamer {
  class WebSocketCmdHandler {
  private:
    ActionRouter *_pActionRouter;
    QICStreamerApplication *_pApp;

  public:
    WebSocketCmdHandler(QICStreamerApplication *pApp);
    virtual ~WebSocketCmdHandler();
    void SendCmdResponse(RestCLIMessage& msg);
    bool ParseCommand(BaseHTTPWSProtocol *pFrom, uint8_t *pData, uint32_t length);
  };
};

#endif /* _WEBSOCKETCMDHANDLER_H */
