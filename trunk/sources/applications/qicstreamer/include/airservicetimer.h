
#ifndef _AIRSERVICETIMER_H
#define _AIRSERVICETIMER_H
#include "baseservicetimer.h"

namespace app_qicstreamer {
  class AirServiceTimer
  : public BaseServiceTimer {
  private:
    bool _bTried;

  public:
    AirServiceTimer (BaseClientApplication* pClientApp, ws_param_t& customedParam);
    virtual ~AirServiceTimer ();
    virtual void OnConnectionFail();
    virtual string GetCloudServer();
    virtual bool TryConnect(string ticket);
 };
}; //namespace app_qicstreamer

#endif /* _AIRSERVICETIMER_H */
