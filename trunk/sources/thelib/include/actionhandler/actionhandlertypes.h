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

#ifndef _ACTIONHANDLERTYPES_H
#define _ACTIONHANDLERTYPES_H

#include "common.h"

//device
#define AT_DEVICE     MAKE_TAG3('D','E','V')
#define AT_DEVICE_STR "device"
//network
#define AT_NETWORK     MAKE_TAG7('N','E','T','W','O','R','K')
#define AT_NETWORK_STR "network"
//video
#define AT_VIDEO      MAKE_TAG5('V','I','D','E','O')
#define AT_VIDEO_STR  "video"
//audio
#define AT_AUDIO      MAKE_TAG5('A','U','D','I','O')
#define AT_AUDIO_STR  "audio"
//record
#define AT_RECORD     MAKE_TAG3('R','E','D')
#define AT_RECORD_STR "record"
//gpio
#define AT_IO       MAKE_TAG2('I','O')
#define AT_IO_STR   "io"

//ptz
//#define AT_PTZ        MAKE_TAG3('P','T','Z')
//#define AT_PTZ_STR    "ptz"

#define AT_SERVICE        MAKE_TAG7('S','E','R','V','I','C','E')
#define AT_SERVICE_STR    "service"

//event
#define AT_EVENT        MAKE_TAG5('E','V','E','N','T')
#define AT_EVENT_STR    "event"

//test (for production test application)
#define AT_PRODUCTION   MAKE_TAG7('P','R','O','D','U','C','T')
#define AT_PRODUCTION_STR    "production"

//iva
#define AT_IVA       MAKE_TAG3('I','V','A')
#define AT_IVA_STR   "iva"

#endif
