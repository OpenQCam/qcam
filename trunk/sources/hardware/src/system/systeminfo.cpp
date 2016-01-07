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
// // Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//

#include "system/systeminfo.h"
//thelib
#include "protocols/avhttpstream/avstreamdefine.h"
#include "protocols/cli/restclidefine.h"
#include "system/eventdefine.h"
#include "system/systemdefine.h"
#include "streaming/streamstypes.h"
#include "streaming/inmsgstream.h"
#include "streaming/streamsmanager.h"
//hardware
#include "hardwaremanager.h"
#include "protocols/http/httpauthhelper.h"
#include "video/basev4l2videocapture.h"
//application
#include "application/baseclientapplication.h"

#define SYSINFO_PERIOD 6
#define EXPIRE_PERIOD 3

SystemInfo::SystemInfo(uint32_t cbId, StreamsManager *pStreamsManager)
  : _pStreamsManager(pStreamsManager),
    _cmdPeriod(0),
    _bSysInfo(false),
    _cbId(cbId) {
}

SystemInfo::~SystemInfo() {
}

void SystemInfo::SetSystemInfo(bool bInfo) {
  _bSysInfo=bInfo;
}

bool SystemInfo::GetSystemInfo() {
  return _bSysInfo;
}

bool SystemInfo::TimePeriodElapsed() {
  Variant params;
  BaseClientApplication *pApp=
      const_cast<BaseClientApplication*>(_pStreamsManager->GetApplication());

  if (_cmdPeriod%SYSINFO_PERIOD==0) {
    double curTime=0;
    GETTIMESTAMP(curTime);

    HardwareManager::CheckStatus(pApp, curTime);

    if (HardwareManager::IsErrorStatus(EC_VIDEO_AVC)) {
      if (HardwareManager::GetSendStatus(EC_VIDEO_AVC)) {
        pApp->OnNotifyEvent(CLOUD_MSG_ERROR, EC_VIDEO_AVC, 0, "");
        HardwareManager::ClearSendStatus(EC_VIDEO_AVC);
      }
    }

    if (_bSysInfo) {
      static double lastTime=0;

      string timeDiff=numToString<double>(curTime-lastTime, 3);
      //cbId is reserved for future usage
      UnixDomainSocketManager::SendCmdToInfoThread("icam_get_sysinfo "+timeDiff, INFO_OP_SYSTEMINFO, _cbId);
      lastTime=curTime;
    }
  }

  _cmdPeriod++;
  return true;
}
