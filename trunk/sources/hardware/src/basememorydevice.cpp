#include "basememorydevice.h"
#include <sys/sysinfo.h>
//hardware
#include "hardwaremanager.h"



BaseHardwareAbstractDevice *BaseMemoryDevice::_pDeviceInstance=NULL;

BaseMemoryDevice::BaseMemoryDevice(HardwareType hwType,
                                   IOHandlerType hType,
                                   uint32_t size)
: BaseHardwareAbstractDevice(hwType, 0, hType),
  _minRequiredSize(size)
{
}

BaseMemoryDevice::~BaseMemoryDevice() {
}

//static member function
BaseHardwareAbstractDevice* BaseMemoryDevice::GetInstance(uint32_t size) {
  if(_pDeviceInstance == NULL){
    _pDeviceInstance =
        (BaseHardwareAbstractDevice *) new BaseMemoryDevice(HT_MEM, IOHT_MEM, size);
  }
  return _pDeviceInstance;
}

bool BaseMemoryDevice::InitializeDevice() {
  return true;
}

bool BaseMemoryDevice::GetFreeSpace(uint32_t& free){
  struct sysinfo myinfo;
  sysinfo (&myinfo);
  free = (myinfo.mem_unit * myinfo.freeram)>>10;
  return true;
}

bool BaseMemoryDevice::IsMemoryAvailable(uint32_t minRequiredSize) {
  uint32_t free=0;

  GetFreeSpace(free);
  if (free >= minRequiredSize) {
    return true;
  }
  return false;
}

bool BaseMemoryDevice::IsMemoryAvailable() {
  uint32_t free=0;

  GetFreeSpace(free);
  if (free >= _minRequiredSize) {
    return true;
  }
  return false;

}

void BaseMemoryDevice::SetMinSize(uint32_t minSize) {
  _minRequiredSize = minSize;
}

void BaseMemoryDevice::CheckStatus(double ts) {
  if (!IsMemoryAvailable(_minRequiredSize)) {
    HardwareManager::SetStatus(EC_OUT_OF_MEMORY, true);
    //DEBUG ("out of memory");
  }
  else {
    HardwareManager::ClearStatus(EC_OUT_OF_MEMORY);
    //DEBUG ("clear oom status");
  }
}

bool BaseMemoryDevice::Close() {
  NYI;
  return true;
}
