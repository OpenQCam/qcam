#include "storage/nfsstoragedevice.h"
#include <sys/statvfs.h>

BaseHardwareAbstractDevice *NFSStorageDevice::_pDeviceInstance=NULL;

NFSStorageDevice::NFSStorageDevice(string mountPath)
: BaseStorageDevice(HT_NFS_STORAGE, mountPath, IOHT_NFS_STORAGE)
{
}

NFSStorageDevice::~NFSStorageDevice() {
}

//static member function
BaseHardwareAbstractDevice* NFSStorageDevice::GetInstance(string mountPath) {
  if(_pDeviceInstance == NULL){
   _pDeviceInstance = (BaseHardwareAbstractDevice *) new NFSStorageDevice(mountPath);
 }
  return _pDeviceInstance;
}

bool NFSStorageDevice::IsStorageAvailable(string path, uint64_t minRequiredSize) {
  uint64_t freeSpace=0;

  GetFreeSpace(freeSpace, path);
  if (freeSpace >= minRequiredSize) {
    return true;
  }
  return false;
}

void NFSStorageDevice::CheckStatus(double ts) {
//  NYI;
}

bool NFSStorageDevice::Close() {
  NYI;
  return true;
}
