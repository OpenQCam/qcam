#ifndef _BASESTORAGEDEVICE_H
#define _BASESTORAGEDEVICE_H

#include "common.h"
#include "hardwaretypes.h"
#include "basehardwareabstractdevice.h"

class BaseHardwareAbstractDevice;

class BaseStorageDevice
: public BaseHardwareAbstractDevice
{
private:
    static BaseHardwareAbstractDevice *_pDeviceInstance;

  protected:
    string _mountPath;
    uint64_t _minRequiredSize;
    uint32_t _type;
    bool _isMounted;
    double _lastSendTime;

  protected:
    BaseStorageDevice(HardwareType hwType,
                      string mountPath,
                      IOHandlerType hType);
    virtual ~BaseStorageDevice();

  public:
    static BaseHardwareAbstractDevice* GetInstance(string mountPath);
    virtual bool InitializeDevice();

    void SetMinSize(uint64_t size);
    virtual bool GetFreeSpace(uint64_t &freeSpace, string path);
    virtual bool IsStorageAvailable(string path, uint64_t reqSize);
    virtual void CheckStatus(double ts);
    virtual bool Close();
};

#endif
