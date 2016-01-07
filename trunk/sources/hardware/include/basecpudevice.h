
#ifndef _BASECPUDEVICE_H
#define _BASECPUDEVICE_H

#include "common.h"
#include "hardwaretypes.h"
#include "basehardwareabstractdevice.h"

#define CPU_THRESHOLD (0.75)

typedef struct cpuInfo{
  uint32_t total;
  uint32_t load;
} cpuInfo;

class BaseCPUDevice
: public BaseHardwareAbstractDevice
{
  private:
    static BaseHardwareAbstractDevice *_pDeviceInstance;
    string _filePath;
    cpuInfo _last, _current;
    double _load_per;

  protected:
    BaseCPUDevice(HardwareType hwType, string filePath, IOHandlerType hType);
    virtual ~BaseCPUDevice();

  public:
    static BaseHardwareAbstractDevice* GetInstance(string filePath);
    virtual void CheckStatus(double ts);
    virtual bool InitializeDevice();
    virtual bool Close();
};

#endif
