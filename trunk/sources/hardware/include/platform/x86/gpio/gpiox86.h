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

#ifndef _GPIOX86_H
#define _GPIOX86_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <fstream>

#include "common.h"
#include "hardwaretypes.h"
#include "gpio/basegpiodevice.h"




class DLLEXP GPIOX86 : public BaseGPIODevice {
//class GPIOX86
  private:
  // GPIO
    static BaseHardwareAbstractDevice *_pDeviceInstance;
  private:
  // GPIO
    GPIOX86(gpio_status defaultSetting);
  public:
  // GPIO
    ~GPIOX86();

    // Device singleton Interface
    static BaseHardwareAbstractDevice* GetInstance(gpio_status defaultSetting);
    // BaseHardwareAbstractDevice
    virtual bool InitializeDevice();

    // Interrupt not support in X86 platform
    // LED not support in X86 platform
    // PIR not support in X86 platform
    void PIRAlarm(void);  // just for simulation PIR signal in X86 platform

    virtual void CheckStatus(double ts);
    // ALS not support in X86 platform
    virtual bool Close();
};


#endif  /* _GPIOX86_H */
