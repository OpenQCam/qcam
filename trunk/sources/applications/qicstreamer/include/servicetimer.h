
#ifndef _SERVICETIMER_H
#define _SERVICETIMER_H
#include "common.h"
#include "baseservicetimer.h"

namespace app_qicstreamer {
  class ServiceTimer
  : public BaseServiceTimer,
    public ISystemCommandCallback {
  private:
    string _agentAddr;
    uint32_t _msgId;
    string _token;

  public:
    ServiceTimer(BaseClientApplication* pClientApp,
                 ws_param_t& customedParam,
                 string agentAddr);
    virtual ~ServiceTimer ();
    virtual void QueryCloudServer();
    virtual void OnCmdFinished(uint32_t msgId, uint8_t* pData,
                               uint32_t dataLength);
    virtual bool TryConnect(string ticket);
 };
}; //namespace app_qicstreamer

#endif /* _SERVICETIMER_H */
