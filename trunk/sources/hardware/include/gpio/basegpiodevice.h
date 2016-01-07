// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    : 2013/05/21
// // Version :
// // Author  :
// // ------------------------------------------------------------------
// // Purpose : GPIO Interface to control misc devices in different platform
// // ======================================================================
//

#ifndef _BASEGPIODEVICE_H
#define _BASEGPIODEVICE_H
#include "common.h"
#include "hardwaretypes.h"
#include "basehardwareabstractdevice.h"
#include "gpio/gpiotable.h"
#include "gpio/ledevent.h"

class BaseGPIODevice
: public BaseHardwareAbstractDevice
{
  private:
  protected:
    BaseGPIODevice(int32_t deviceFD, gpio_status defaultSetting);
    virtual ~BaseGPIODevice();

    gpio_status _currentSetting;
  public:
    gpio_status GetCurrentSettingStatus();
  // Interrupt
    virtual bool RegisterTriggerCallback(ceu32 gpioIndex){ INFO("NIY:%s", __FUNCTION__); return true;}
    virtual bool UnRegisterTriggerCallback(ceu32 gpioIndex){ INFO("NIY:%s", __FUNCTION__); return true;}
    virtual uint32_t PollSystemStatus(){ /*INFO("NIY:%s", __FUNCTION__);*/ return 0;}

  // LED
    virtual int32_t LedOn(ceu32 gpioNumber, bool enable, cei32 duration){ INFO("NIY:%s", __FUNCTION__); return 0;}
    virtual int32_t LedBlink(ceu32 gpioNumber, cei32 duration, ceu32 timeOn, ceu32 timeOff){ INFO("NIY:%s", __FUNCTION__); return 0;}
    virtual int32_t SetLedEvent(LEDEventType ledEvent, bool enable){ INFO("NIY:%s", __FUNCTION__); return 0;}
    virtual int32_t CheckLedEvent(LEDEventType ledEvent){ INFO("NIY:%s", __FUNCTION__); return 0;}

  // PIR
    virtual int32_t EnablePIRNotification(void){ INFO("NIY:%s", __FUNCTION__); return 0;}
    virtual int32_t DisablePIRNotification(void){ INFO("NIY:%s", __FUNCTION__); return 0;}
    virtual void PIRAlarm(void){ INFO("NIY:%s", __FUNCTION__); return;}

  // ALS
    virtual int32_t EnableALSNotification(void){ INFO("NIY:%s", __FUNCTION__); return 0;}
    virtual int32_t DisableALSNotification(void){ INFO("NIY:%s", __FUNCTION__); return 0;}
    virtual int32_t SetALSNotification(bool enable){ INFO("NIY:%s", __FUNCTION__); return 0;}

    virtual uint32_t GetLightLux(void){ INFO("NIY:%s", __FUNCTION__); return 0;}
    virtual void SetLightThreshold(uint32_t thresholdTurnOnIR, uint32_t thresholdTurnOffIR){ INFO("NIY:%s", __FUNCTION__); return;}
    virtual void GetLightThreshold(uint32_t &thresholdTurnOnIR, uint32_t &thresholdTurnOffIR){ INFO("NIY:%s", __FUNCTION__); return;}

    virtual bool EnableIRLED(bool enable){ INFO("NIY:%s", __FUNCTION__); return true;} // Force turn ON/OFF IR LED
    virtual void SetIRLED(void){ INFO("NIY:%s", __FUNCTION__); return;}  // detect ALS light lux to turn IR LED ON/OFF auto
};
#endif // _BASEGPIODEVICE_H
