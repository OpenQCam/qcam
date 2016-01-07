#include "audio/baseaudiodevice.h"

BaseAudioDevice::BaseAudioDevice(HardwareType hwType, int32_t fd)
  : BaseHardwareAbstractDevice(hwType, fd, IOHT_QIC_AUDIO),
    _volume(0),
    _isStart(false)
{
}

BaseAudioDevice::~BaseAudioDevice()
{
}

const device_parameters& BaseAudioDevice::GetDeviceParameters()
{
  return _deviceParams;
}

uint32_t BaseAudioDevice::GetAudioFrameSize() {
  NYI;
  return 0;
}

int32_t BaseAudioDevice::PutData(uint8_t *pData, uint32_t size) {
  NYI;
  return -1;
}
