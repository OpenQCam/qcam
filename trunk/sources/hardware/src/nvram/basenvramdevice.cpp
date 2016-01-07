#include "nvram/basenvramdevice.h"

BaseNVRamDevice::BaseNVRamDevice()
: BaseHardwareAbstractDevice(HT_NVRAM, IOHT_NVRAM)
{
}

BaseNVRamDevice::~BaseNVRamDevice()
{
}

