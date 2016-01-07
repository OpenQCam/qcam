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
// // Author  :
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================

/*
Usage:
GpioTestWrite - writing test (output).
GpioTestRead- reading test (input).
GpioTestInterrupt - interrupt test for gpio number.
GpioSetLed <gpio> <on> <off> <blinks> <rests> <times> - set led <gpio> on/off interval,
                                                        noumber of blinking/resting cycles,
                                                        times of blinking.
*/


#ifndef _MISCSTATUS_H
#define _MISCSTATUS_H

#include <signal.h>
#include "common.h"


// open and close log
//#define HAS_CEGPIO_LOG

#ifdef HAS_CEGPIO_LOG
#define MISC_DEBUG printf
#else
#define MISC_DEBUG(...)
#endif

class DLLEXP MISCStatus{
  private:
  public:
    static bool DoSystemCommand(char* cmd, string &output);
};


#endif  /* _MISCSTATUS_H */
