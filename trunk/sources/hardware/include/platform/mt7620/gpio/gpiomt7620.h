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

#ifndef _GPIOMT7620_H
#define _GPIOMT7620_H

#include <sys/ioctl.h>
//#include <linux/autoconf.h>
//#include <char/ralink_gpio.h>

#include "common.h"
#include "hardwaretypes.h"
#include "gpio/basegpiodevice.h"

typedef enum _WifiModeType {
  WIFI_AP_MODE = 0,
  WIFI_STA_MODE,
  WIFI_UNKNOW_MODE,
} WifiModeType;

class DLLEXP GPIOMT7620 : public BaseGPIODevice {
  private:
    static BaseHardwareAbstractDevice *_pDeviceInstance;
    static void SignalHandler(int signum);
    //void (*GpioTriggerFunctions[RALINK_GPIO_NUMBER])();
    // Button event (read define from nvram)
    uint32_t _eventNumReset;
    uint32_t _eventNumAPMode;
    uint32_t _eventNumWPS;
    uint32_t _eventNumPIR;

    WifiModeType _wifi5gMode;
    WifiModeType _wifi2gMode;
    bool _rebooting;
    bool _wpsPairing;

  private:
    GPIOMT7620(int32_t deviceFD, gpio_status defaultSetting);

    int GpioSetDirectionGroup(int r, int dir);
    int GpioReadGroup(int r, int *value);
    int GpioWriteGroup(int r, int value);

    int GpioEnableIrq(void);
    int GpioDisableIrq(void);
    int GpioRegisterIrqInfo(int gpio_num);

    int GpioSetDirection(uint8_t gpioNum, int dir);
    void GpioWrite(uint8_t gpioNum, bool gpioValue);
    bool GpioRead(uint8_t gpioNum);

    void GpioSetInterrupt(int gpio_num, void (*NotifyFunction)(void));
    int GpioGetInterruptNum(int *value);
    //int GpioGetAllInterrupts(ralink_gpio_interrupt_record (&gpio_record)[RALINK_GPIO_NUMBER]);

    void GpioSetLed(uint8_t gpio, uint16_t on, uint16_t off, uint16_t blinks, uint16_t resets, uint16_t times);
    int32_t SetLedEvent(LEDEventType ledEvent, bool enable);

    //------------------------only for test------------------------
    void GpioTestWrite(void);
    void GpioTestRead(void);
    void GpioTestInterrupt(int gpio_num);

    int32_t EnablePIRNotification(void);
    int32_t DisablePIRNotification(void);
    void PIRAlarm(void);

  public:
    ~GPIOMT7620();

    // Device singleton Interface
    static BaseHardwareAbstractDevice* GetInstance(gpio_status defaultSetting);
    virtual bool InitializeDevice();
    virtual bool OnReadEvent();
    virtual void CheckStatus(double ts);
    virtual bool Close();
};

#endif  /* _GPIOMT7620_H */
