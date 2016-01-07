#include "storage/basestoragedevice.h"
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <linux/magic.h>
//hardware
#include "system/systemmanager.h"
#include "hardwaremanager.h"


// FIXME(recardo):
// celeno platform hack a different RAMFS_MAGIC number in fs/ramfs/inode.c
// where ramfs.h already define it
#if (defined(__x86__))
#define ROMFS_MAGIC 0x7275
#else
#define ROMFS_MAGIC 0x858458f6
#endif


BaseHardwareAbstractDevice *BaseStorageDevice::_pDeviceInstance=NULL;

BaseStorageDevice::BaseStorageDevice(HardwareType hwType,
                                     string mountPath,
                                     IOHandlerType hType)
: BaseHardwareAbstractDevice(hwType, 0, hType),
  _mountPath(mountPath),
  _type(MSDOS_SUPER_MAGIC),
  _isMounted(false),
  _lastSendTime(0)
{
}

BaseStorageDevice::~BaseStorageDevice() {
}

//static member function
BaseHardwareAbstractDevice* BaseStorageDevice::GetInstance(string mountPath) {
  if(_pDeviceInstance == NULL){
   _pDeviceInstance =
       (BaseHardwareAbstractDevice *) new BaseStorageDevice(HT_STORAGE,
                                                            mountPath,
                                                            IOHT_STORAGE);
 }
  return _pDeviceInstance;
}

bool BaseStorageDevice::InitializeDevice() {
  string cmdOutput;
  Variant result;
  uint32_t start=0;
  SystemManager::DoSystemCommand("icam_check_sdmmc", cmdOutput);

  if (Variant::DeserializeFromJSON(cmdOutput, result, start)) {
    if (result.HasKey("status") && result["status"]==V_BOOL) {
      if ((bool)result["status"]==true) {
        _isMounted=true;
        return true;
      }
    }
  }
  else {
    DEBUG ("Unable to deserialize json");
  }

  //Set storage to error status
  FATAL ("Unable to mount SD/MMC. Device error.");
  HardwareManager::SetStatus(EC_STORAGE, true);
  return true;
}


bool BaseStorageDevice::GetFreeSpace(uint64_t& freeSpace,
                                     string path){
  struct statfs buf;
  uint32_t storageType;

  freeSpace=0;
  statfs (STR(path), &buf);
  storageType = buf.f_type;

  if (storageType!=_type) {
    freeSpace=0;
    //FATAL ("unsupport storage type:%x", storageType);
    return false;
  }


  if (storageType == NFS_SUPER_MAGIC){
    WARN ("nfs not support");
    return false;
  }
  else if (storageType == EXT2_SUPER_MAGIC ||
           storageType == MSDOS_SUPER_MAGIC ) { //ext filesystem
    struct statvfs vfsInfo;
    if (-1 == statvfs(path.c_str(), &vfsInfo)) {
      return false;
    }

    freeSpace= (vfsInfo.f_bsize>>10)*(vfsInfo.f_bfree);
  }
  else {
    WARN ("not supported filesystem:%04x", storageType);
    return false;
  }
  return true;
}

bool BaseStorageDevice::IsStorageAvailable(string path, uint64_t minRequiredSize) {
  uint64_t freeSpace=0;

  GetFreeSpace(freeSpace, path);
  if (freeSpace >= minRequiredSize) {
    return true;
  }

  //DEBUG ("free:%llu, reqSize:%llu", freeSpace, minRequiredSize);
  return false;
}

void BaseStorageDevice::SetMinSize(uint64_t minSize) {
  _minRequiredSize = minSize;
}

void BaseStorageDevice::CheckStatus(double ts) {
  if (!IsStorageAvailable(_mountPath, _minRequiredSize)) {
    HardwareManager::SetStatus(EC_OUT_OF_STORAGE, true);
    //DEBUG ("out of storage");
  }
  else {
    HardwareManager::ClearStatus(EC_OUT_OF_STORAGE);
    //DEBUG ("clear out of storage");
  }

  //Reset send status, 4 hours
  if ((ts - _lastSendTime) > (3600000*4)) {
    HardwareManager::ClearSendStatus(EC_OUT_OF_STORAGE);
    _lastSendTime = ts;
  }
}

bool BaseStorageDevice::Close() {
  NYI;
  return true;
}
