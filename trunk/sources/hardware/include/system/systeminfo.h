// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    : 2012/12/26
// // Version :
// // Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose : To calculate cpu utilization rate
// // ======================================================================

#ifndef _SYSTEMINFO_H
#define _SYSTEMINFO_H

#include "protocols/timer/basetimerprotocol.h"
#include "netio/netio.h"
#include "system/systemmanager.h"

class SystemInfo
  : public BaseTimerProtocol
{
  private:
    StreamsManager *_pStreamsManager;
    uint32_t _cmdPeriod;
    bool _bSysInfo;
    uint32_t _cbId;

  public:
    SystemInfo(uint32_t cbId, StreamsManager *pStreamsManager);
    void SetSystemInfo(bool bInfo);
    bool GetSystemInfo();
    virtual ~SystemInfo();
    virtual bool TimePeriodElapsed();
};

#endif
