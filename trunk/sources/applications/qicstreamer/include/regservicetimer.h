#ifndef _REGSERVICETIMER_H
#define _REGSERVICETIMER_H
#include "baseservicetimer.h"

namespace app_qicstreamer {
  class RegServiceTimer
  : public BaseServiceTimer,
    public ISystemCommandCallback{
  private:
    string _agentAddr;
    uint32_t _msgId;

  public:
    RegServiceTimer (BaseClientApplication* pClientApp,
                     ws_param_t& customedParam,
                     string agentAddr);
    virtual ~RegServiceTimer();
    virtual void QueryCloudServer();
    virtual void OnCmdFinished(uint32_t msgId, uint8_t* pData,
                               uint32_t dataLength);
    virtual bool TryConnect(string ticket);
 };
}; //namespace app_qicstreamer

#endif /* _REGSERVICETIMER_H */
