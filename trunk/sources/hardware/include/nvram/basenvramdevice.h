// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    : 2013/05/21
// // Version :
// // Author  :
// // ------------------------------------------------------------------
// // Purpose : NVram Interface to Set/Get section value in different platform
// // ======================================================================
//

#ifndef _BASENVRAMDEVICE_H
#define _BASENVRAMDEVICE_H
#include "common.h"
#include "netio/netio.h"
//hardware
#include "hardwaretypes.h"
#include "basehardwareabstractdevice.h"

class BaseNVRamDevice
: public BaseHardwareAbstractDevice,
  public ISystemCommandCallback
{
  private:
  protected:
    BaseNVRamDevice();
    virtual ~BaseNVRamDevice();

  public:
    virtual bool Set(const string &section, const string &key, const string &value, bool bSync) = 0;
    virtual bool Set(const string &key, const string &value, bool bSync=true) = 0;
    virtual bool Get(const string &section, const string &key, string &value) = 0;
    virtual bool Get(const string &key, string &value) = 0;
    virtual bool RefreshNVRam() = 0;

    virtual void OnCmdFinished(uint32_t msgId, uint8_t *pData,
                               uint32_t dataLength)=0;

    virtual void GetAll(Variant& value)=0;

};
#endif // _BASENVRAMDEVICE_H
