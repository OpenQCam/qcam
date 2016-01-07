// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    :
// // Version :
// // Author  :
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//

// Includes
#include "hardwaremanager.h"
#include "gpio/gpiox86.h"

// Constants

// open and close log
//#define HAS_GPIO_DEBUG

#ifdef HAS_GPIO_DEBUG
#define GPIO_FINE FINE
#define GPIO_DEBUG DEBUG
#define GPIO_INFO INFO
#else
#define GPIO_FINE(...)
#define GPIO_DEBUG(...)
#define GPIO_INFO(...)
#endif

BaseHardwareAbstractDevice *GPIOX86::_pDeviceInstance = NULL;

BaseHardwareAbstractDevice* GPIOX86::GetInstance(gpio_status defaultSetting) {
  if(_pDeviceInstance == NULL){
    _pDeviceInstance = new GPIOX86(defaultSetting);
    if(!_pDeviceInstance->InitializeDevice()){
      delete _pDeviceInstance;
      return NULL;
    }
  }
  return _pDeviceInstance;
}

bool GPIOX86::InitializeDevice() {
  FATAL("NOT support GPIO in X86 platform!");
  return true;
}

GPIOX86::GPIOX86(gpio_status defaultSetting)
: BaseGPIODevice(0, defaultSetting)
{
  FATAL("NOT support GPIO in X86 platform!");
}

GPIOX86::~GPIOX86()
{
  FATAL("NOT support GPIO in X86 platform!");
}

void GPIOX86::PIRAlarm()
{
  DEBUG("Trigger PIR Alarm in X86 Platform (simulated event)");
  NotifyObservers (GPIO_PIRALRAM , "PIR Alarm");
  return;
}

void GPIOX86::CheckStatus(double ts) {
  NYI;
}

bool GPIOX86::Close() {
  NYI;
  return false;
}
