#include "gpio/basegpiodevice.h"

BaseGPIODevice::BaseGPIODevice(int32_t deviceFD, gpio_status defaultSetting)
  : BaseHardwareAbstractDevice(HT_GPIO, deviceFD, IOHT_GPIO)
{
  _currentSetting = defaultSetting;
}

BaseGPIODevice::~BaseGPIODevice()
{
}

gpio_status BaseGPIODevice::GetCurrentSettingStatus()
{
  DEBUG("_currentSetting.enablePIR:%d", _currentSetting.enablePIR);
  DEBUG("_currentSetting.enableALS:%d", _currentSetting.enableALS);
  return _currentSetting;
}
