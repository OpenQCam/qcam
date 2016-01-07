#ifndef _NFSSTORAGEDEVICE_H
#define _NFSSTORAGEDEVICE_H

#include "common.h"
#include "hardwaretypes.h"
#include "basestoragedevice.h"

class BaseStorageDevice;

class NFSStorageDevice
: public BaseStorageDevice
{
  private:
    static BaseHardwareAbstractDevice *_pDeviceInstance;

  protected:
    NFSStorageDevice(string mountPath);
    virtual ~NFSStorageDevice();

  public:
    static BaseHardwareAbstractDevice* GetInstance(string mountPath);
    virtual bool IsStorageAvailable(string path, uint64_t reqSize);
    virtual void CheckStatus(double ts);
    virtual bool Close();
};

#endif
