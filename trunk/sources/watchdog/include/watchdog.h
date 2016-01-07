/* ============================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 *            (C) COPYRIGHT Quanta Computer Inc.
 *                   ALL RIGHTS RESERVED
 *
 * ============================================================================
 * ----------------------------------------------------------------------------
 * Date    : 2013/03/19
 * Version : 0.1
 * Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 * ----------------------------------------------------------------------------
 * Purpose : watchdog for qcam
 * ============================================================================
 */

#ifndef _WATCHDOG_H
#define _WATCHDOG_H

#include "common.h"
#include "netio/select/unixdomainsocketclient.h"

class WatchDog : public IMessageCallBack
{
  private:
    uint8_t _interval2check;
    string _pidPath;
    string _restartScriptPath;
    int8_t _counter;
    UnixDomainSocketClient *_pUXClient;
    bool CheckDaemon();
  public:
    WatchDog(const char* pidPath, const char* scriptPath, uint8_t interval);
    virtual ~WatchDog();
    void Run();
    virtual void OnMessage(IOBuffer &buffer);
  private:
    void ResetCounter();
    bool RestartDaemon();
};
#endif /* _WATCHDOG_H */

