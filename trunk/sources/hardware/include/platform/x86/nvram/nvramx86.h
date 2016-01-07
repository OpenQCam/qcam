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

#include <sys/socket.h>
#include <fstream>

#include "common.h"
#include "hardwaretypes.h"
#include "nvram/basenvramdevice.h"



class DLLEXP NVRamX86 : BaseNVRamDevice {
//class NVRamX86
  private:
    static BaseHardwareAbstractDevice *_pDeviceInstance;
    string _filePath;
    Variant _infoVar;

    uint8_t _setTimes;
  private:
    NVRamX86(string filePath);
    bool WriteToNVRam();
  public:
    ~NVRamX86();
    // Device singleton Interface
    static BaseHardwareAbstractDevice* GetInstance(string filePath);
    // BaseHardwareAbstractDevice
    virtual bool InitializeDevice();
    virtual void CheckStatus(double ts);

    virtual bool Set(const string &section, const string &key, const string &value, bool bSync);
    virtual bool Set(const string &key, const string &value, bool bSync=true);
    virtual bool Get(const string &section, const string &key, string &value);
    virtual bool Get(const string &key, string &value);
    virtual bool RefreshNVRam();
    virtual void OnCmdFinished(uint32_t msgId, uint8_t *pData,
                               uint32_t dataLength);
    virtual void GetAll(Variant& value);
    virtual bool Close();
};


#endif  /* _NVRAM_H */
