#include "common.h"
#include "hardwaretypes.h"
#include "basecpudevice.h"

BaseHardwareAbstractDevice *BaseCPUDevice::_pDeviceInstance=NULL;

BaseCPUDevice::BaseCPUDevice (HardwareType hwType,
                              string filePath,
                              IOHandlerType hType)
: BaseHardwareAbstractDevice(hwType, 0, hType),
  _filePath(filePath)
{
}

BaseCPUDevice::~BaseCPUDevice() {
}

//static member function
BaseHardwareAbstractDevice* BaseCPUDevice::GetInstance(string filePath) {
 if(_pDeviceInstance == NULL){
   _pDeviceInstance =
       (BaseHardwareAbstractDevice *) new BaseCPUDevice(HT_CPU, filePath, IOHT_CPU);
 }
 return _pDeviceInstance;
}

bool BaseCPUDevice::InitializeDevice() {
  CheckStatus(0);
  return true;
}

void BaseCPUDevice::CheckStatus(double ts) {
  FILE *fp=fopen(_filePath.c_str(), "r");
  uint32_t jiffies[7];

  if (fp) {
    _last = _current;
    _current.total=_current.load=0;
    fscanf (fp, "cpu %u %u %u %u %u %u %u",
            jiffies, &jiffies[1], &jiffies[2], &jiffies[3],
            &jiffies[4], &jiffies[5], &jiffies[6]);

    _current.load = jiffies[0] + jiffies[1] + jiffies[2];
    for (uint32_t i=0; i< 7; i++) {
      _current.total += jiffies[i];
    }
    fclose (fp);
    _load_per=(double)(_current.load-_last.load)/(_current.total-_last.total);
  }
  else {
    _load_per=0;
  }


  //DEBUG ("load_per:%f", _load_per);
  if (_load_per > CPU_THRESHOLD) {
    HardwareManager::SetStatus(EC_CPU_BUSY, true);
    //DEBUG ("cpu busy");
  }
  else {
    HardwareManager::ClearStatus(EC_CPU_BUSY);
    //DEBUG ("clear cpu busy");
  }
}

bool BaseCPUDevice::Close() {
  return true;
}
