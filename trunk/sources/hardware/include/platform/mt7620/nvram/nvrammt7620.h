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

#ifndef _NVRAM_H
#define _NVRAM_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <fstream>

#include "common.h"
#include "hardwaretypes.h"
#include "nvram/basenvramdevice.h"


class DLLEXP NVRamCL1830
  : public BaseNVRamDevice {
//class NVRamCL1830
  private:
    static BaseHardwareAbstractDevice *_pDeviceInstance;
    Variant _nvramSetting;
    map<uint32_t, string> _cmdTable;

  private:
    NVRamCL1830();
  public:
    ~NVRamCL1830();
    // Device singleton Interface
    static BaseHardwareAbstractDevice* GetInstance();
    // BaseHardwareAbstractDevice
    virtual bool InitializeDevice();

    virtual void CheckStatus(double ts);
    virtual bool Set(const string &section, const string &key,
                     const string &value, bool bSync);
    virtual bool Set(const string &key, const string &value, bool bSync=true);
    virtual bool Get(const string &section, const string &key, string &value);
    virtual bool Get(const string &key, string &value);
    virtual bool RefreshNVRam();
    virtual void GetAll(Variant& value);
    virtual void OnCmdFinished(uint32_t msgId, uint8_t *pData,
                               uint32_t dataLength);

    virtual bool Close();
};


#endif  /* _NVRAM_H */
