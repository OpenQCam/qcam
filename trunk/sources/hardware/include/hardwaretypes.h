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

#ifndef _HARDWARETYPES_H
#define _HARDWARETYPES_H

#include "common.h"

enum HardwareType{
  //video
  HT_VIDEO_MJPG = 0,
  HT_VIDEO_AVC,
  HT_VIDEO_MOTION,
  //audio
  HT_MIC,
  HT_SPEAKER,
  //gpio
  HT_GPIO,
  //nvram
  HT_NVRAM,
  //Storage
  HT_STORAGE,
  //NFS Storage
  HT_NFS_STORAGE,
  //CPU
  HT_CPU,
  //Memory
  HT_MEM,
};

typedef struct _DeviceErrorStatus {
  bool bError;
  bool bSend;
} DeviceErrorStatus;

#endif
