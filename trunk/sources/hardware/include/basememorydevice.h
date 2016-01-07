#ifndef _BASEMEMORYDEVICE_H
#define _BASEMEMORYDEVICE_H

#include "common.h"
#include "hardwaretypes.h"
#include "basehardwareabstractdevice.h"

class BaseHardwareAbstractDevice;

class BaseMemoryDevice
: public BaseHardwareAbstractDevice
{
  private:
    static BaseHardwareAbstractDevice *_pDeviceInstance;
    uint32_t _minRequiredSize;

  protected:
    BaseMemoryDevice(HardwareType hwType, IOHandlerType hType, uint32_t size);
    virtual ~BaseMemoryDevice();

  public:
    static BaseHardwareAbstractDevice* GetInstance(uint32_t size);
    virtual bool InitializeDevice();

    void SetMinSize(uint32_t size);
    virtual bool GetFreeSpace(uint32_t &free);
    virtual bool IsMemoryAvailable(uint32_t reqSize);
    virtual bool IsMemoryAvailable();
    virtual void CheckStatus(double ts);
    virtual bool Close();
};

#endif
