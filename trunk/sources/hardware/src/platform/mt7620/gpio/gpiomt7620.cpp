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
#if 0 // disable by Raylin
// Includes
//#define CONFIG_RALINK_MT7620
#include "system/systemmanager.h"
#include "netio/select/uxdomainmanager.h"
#include "hardwaremanager.h"
#include "gpio/gpiomt7620.h"

// open and close log
#ifdef HAS_GPIO_DEBUG
#define GPIO_FINE FINE
#define GPIO_DEBUG DEBUG
#define GPIO_INFO INFO
#else
#define GPIO_FINE(...)
#define GPIO_DEBUG(...)
#define GPIO_INFO(...)
#endif

enum {
  gpio_in,
  gpio_out,
};

enum {
#if defined (CONFIG_RALINK_RT3052)
  gpio2300,
  gpio3924,
  gpio5140,
#elif defined (CONFIG_RALINK_RT3883)
  gpio2300,
  gpio3924,
  gpio7140,
  gpio9572,
#elif defined (CONFIG_RALINK_RT3352)
  gpio2300,
  gpio3924,
  gpio4540,
#elif defined (CONFIG_RALINK_RT5350)
  gpio2100,
  gpio2722,
#elif defined (CONFIG_RALINK_RT6855A)
  gpio1500,
  gpio3116,
#elif defined (CONFIG_RALINK_MT7620)
  gpio2300,
  gpio3924,
  gpio7140,
  gpio72,
#elif defined (CONFIG_RALINK_MT7621)
  gpio3100,
  gpio6332,
  gpio9564,
#else
  gpio2300,
#endif
};

// Constants
#define GPIO_DEV  "/dev/gpio"

BaseHardwareAbstractDevice *GPIOMT7620::_pDeviceInstance = NULL;

BaseHardwareAbstractDevice* GPIOMT7620::GetInstance(gpio_status defaultSetting) {
  if(_pDeviceInstance == NULL){
    int32_t fd;
    if((fd = open(GPIO_DEV, O_RDWR | O_NONBLOCK, 0)) < 0) {
      FATAL("Could not open device %s errcode = %d\n",GPIO_DEV,fd);
      return NULL;
    }

    _pDeviceInstance = new GPIOMT7620(fd, defaultSetting);
    if(!_pDeviceInstance->InitializeDevice()){
      delete _pDeviceInstance;
      return NULL;
    }
  }
  return _pDeviceInstance;
}

bool GPIOMT7620::InitializeDevice() {

  string output;
  Variant result;
  uint32_t start = 0;

  SystemManager::DoSystemCommand("icam_get_wifi_mode 5g", output);
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("mode")){
    DEBUG( "5G Wifi mode: %s", STR(result["mode"]) );

    if(strcmp("ap", STR(result["mode"])) == 0 ){
      _wifi5gMode = WIFI_AP_MODE;
    }
    else if(strcmp("sta", STR(result["mode"])) == 0 ){
      _wifi5gMode = WIFI_STA_MODE;
    }
    else{
      _wifi5gMode = WIFI_UNKNOW_MODE;
    }
  }

  start = 0;
  SystemManager::DoSystemCommand("icam_get_wifi_mode 2.4g", output);
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("mode")){
    DEBUG( "2G Wifi mode: %s", STR(result["mode"]) );

    if(strcmp("ap", STR(result["mode"])) == 0 ){
      _wifi2gMode = WIFI_AP_MODE;
    }
    else if(strcmp("sta", STR(result["mode"])) == 0 ){
      _wifi2gMode = WIFI_STA_MODE1;
    }
    else{
      _wifi2gMode = WIFI_UNKNOW_MODE;
    }
  }

  start = 0;
  SystemManager::DoSystemCommand("icam_get_buttons_config", output);
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("apmode")){
    if(result["apmode"].HasKey("index")){
      string index = (string) result["apmode"]["index"];
      if(isNumeric(index)){
        _eventNumAPMode = (uint16_t)atoi(STR(index));
        GPIO_DEBUG("_eventNumAPMode: %d", _eventNumAPMode);
      }
    }
  }
  if(result.HasKey("reset")){
    if(result["reset"].HasKey("index")){
      string index = (string) result["reset"]["index"];
      if(isNumeric(index)){
        _eventNumReset = (uint16_t)atoi(STR(index));
        GPIO_DEBUG("_eventNumReset: %d", _eventNumReset);
      }
    }
  }
  if(result.HasKey("wps")){
    if(result["wps"].HasKey("index")){
      string index = (string) result["wps"]["index"];
      if(isNumeric(index)){
        _eventNumWPS = (uint16_t)atoi(STR(index));
        GPIO_DEBUG("_eventNumWPS: %d", _eventNumWPS);
      }
    }
  }
  if(result.HasKey("pir")){
    if(result["pir"].HasKey("index")){
      string index = (string) result["pir"]["index"];
      if(isNumeric(index)){
        _eventNumPIR = (uint16_t)atoi(STR(index));
        GPIO_DEBUG("_eventNumPIR: %d", _eventNumPIR);
      }
    }
  }

  start = 0;
  SystemManager::DoSystemCommand("icam_check_operation_mode", output);
  GPIO_DEBUG( "icam_check_operation_mode: %s", output.c_str() );
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("mode")){
    DEBUG( "Device mode: %s", STR(result["mode"]) );

    if(strcmp("operation", STR(result["mode"])) == 0 ){
      ENABLE_DEVICE_READ_DATA;
      OnReadEvent(); //check all events while boot-up first time
    }
    else if(strcmp("production", STR(result["mode"])) == 0 ){
      DISABLE_DEVICE_READ_DATA;
    }
    else{
      FATAL("Device mode is unknow....");
      ENABLE_DEVICE_READ_DATA;
      OnReadEvent(); //check all events while boot-up first time
    }
  }
  else{
    FATAL("cannot get device mode");
  }

  GPIO_FINE("GPIOMT7620 InitializeDevice done");
  return true;
}

GPIOMT7620::GPIOMT7620(int32_t deviceFD, gpio_status defaultSetting)
: BaseGPIODevice(deviceFD, defaultSetting)
{
  _wifi5gMode = WIFI_UNKNOW_MODE;
  _wifi2gMode = WIFI_UNKNOW_MODE;
  _wpsPairing = false;
  _rebooting = false;

  _eventNumReset = 0xFFFF;
  _eventNumAPMode = 0xFFFF;
  _eventNumWPS = 0xFFFF;
  _eventNumPIR = 0xFFFF;
}

GPIOMT7620::~GPIOMT7620()
{
  if(_deviceFD > 0){  // close gpio device
    close(_deviceFD);
  }
}

bool GPIOMT7620::OnReadEvent() {
  FATAL("trigger!");
  ralink_gpio_interrupt_record gpio_record[RALINK_GPIO_NUMBER];
  GpioGetAllInterrupts(gpio_record);

  string output;
  uint32_t buttonIndex = 0;
  uint8_t buttonPressed = 0;
  uint32_t lastPressDuration = 0;

  for(buttonIndex = 0 ; buttonIndex < RALINK_GPIO_NUMBER ; buttonIndex++){
    buttonPressed = gpio_record[buttonIndex].current_status;
    lastPressDuration = gpio_record[buttonIndex].triggered_duration;

    if(lastPressDuration > 0){
      if(buttonIndex == _eventNumReset){
        GPIO_DEBUG("GPIO_EVENT_RESET_BUTTON: pressed %d, duration: %d", buttonPressed, lastPressDuration);
        if(!_rebooting){
          if(buttonPressed){
            if(lastPressDuration >= 3000){
              INFO("GPIO_EVENT_RESET_BUTTON: factory reset");
              SystemManager::DoSystemCommand("icam_factory_reset", output);
              _rebooting = true;
            }
            else{
              INFO("GPIO_EVENT_RESET_BUTTON: pressed %d, duration: %d", buttonPressed, lastPressDuration);
            }
          }
          else{
            if(lastPressDuration >= 3000){
              INFO("GPIO_EVENT_RESET_BUTTON: factory reset");
              SystemManager::DoSystemCommand("icam_factory_reset", output);
              _rebooting = true;
            }
            else if(lastPressDuration >= 50){
              INFO("GPIO_EVENT_RESET_BUTTON: reboot");
              SystemManager::DoSystemCommand("reboot", output);
              _rebooting = true;
            }
          }
        }
      }
      else if(buttonIndex == _eventNumAPMode){
        GPIO_DEBUG("GPIO_EVENT_AP_STA_SWITCH: pressed %d, duration: %d", buttonPressed, lastPressDuration);
        if(!_rebooting){
          if(buttonPressed){
            if(_wifi5gMode == WIFI_AP_MODE){
              INFO("GPIO_EVENT_AP_STA_SWITCH: change to STA mode");
              SystemManager::DoSystemCommand("icam_set_wifi_mode 5g sta", output);
              _rebooting = true;
            }
            else{
              GPIO_INFO("current is STA mode, no change...");
            }
          }
          else{
            if(_wifi5gMode == WIFI_STA_MODE){
              INFO("GPIO_EVENT_AP_STA_SWITCH: change to AP mode");
              SystemManager::DoSystemCommand("icam_set_wifi_mode 5g ap", output);
              _rebooting = true;
            }
            else{
              GPIO_INFO("current is AP mode, no change...");
            }
          }
        }
      }
      else if(buttonIndex == _eventNumWPS){
        GPIO_DEBUG("GPIO_EVENT_WPS_BUTTON: pressed %d, duration: %d", buttonPressed, lastPressDuration);
        if(_wifi2gMode != WIFI_UNKNOW_MODE && _wifi5gMode != WIFI_UNKNOW_MODE){ // dual band (5g, 2.4g)
          if(buttonPressed){
            if(lastPressDuration >= 3000){
              INFO("GPIO_EVENT_WPS_BUTTON: 5G wifi WPS");
              if(!fileExists("/tmp/wps_pairing")){
                UnixDomainSocketManager::ExecuteSystemCommand("icam_wps_pairing 5g");
              }
              else{
                INFO("WPS is pairing, ignore this button event");
              }
            }
            else{
              INFO("GPIO_EVENT_WPS_BUTTON: pressed %d, duration: %d", buttonPressed, lastPressDuration);
            }
          }
          else{
            if(lastPressDuration >= 3000){
              INFO("GPIO_EVENT_WPS_BUTTON: 5G wifi WPS");
              if(!fileExists("/tmp/wps_pairing")){
                UnixDomainSocketManager::ExecuteSystemCommand("icam_wps_pairing 5g");
              }
              else{
                INFO("WPS is pairing, ignore this button event");
              }
            }
            else if(lastPressDuration >= 50){
              INFO("GPIO_EVENT_WPS_BUTTON: 2.4G wifi WPS");
              if(!fileExists("/tmp/wps_pairing")){
                UnixDomainSocketManager::ExecuteSystemCommand("icam_wps_pairing 2.4g");
              }
              else{
                INFO("WPS is pairing, ignore this button event");
              }
            }
          }
        }
        else if(_wifi2gMode == WIFI_UNKNOW_MODE && _wifi5gMode != WIFI_UNKNOW_MODE){ // 5g only
          if(buttonPressed || (lastPressDuration >= 50) ){
            INFO("GPIO_EVENT_WPS_BUTTON: 5G wifi WPS");
            if(!fileExists("/tmp/wps_pairing")){
              UnixDomainSocketManager::ExecuteSystemCommand("icam_wps_pairing 5g");
            }
            else{
              INFO("WPS is pairing, ignore this button event");
            }
          }
        }
        else if(_wifi2gMode != WIFI_UNKNOW_MODE && _wifi5gMode == WIFI_UNKNOW_MODE){ // 2.4g only
          if(buttonPressed || (lastPressDuration >= 50) ){
            INFO("GPIO_EVENT_WPS_BUTTON: 2.4G wifi WPS");
            if(!fileExists("/tmp/wps_pairing")){
              UnixDomainSocketManager::ExecuteSystemCommand("icam_wps_pairing 2.4g");
            }
            else{
              INFO("WPS is pairing, ignore this button event");
            }
          }
        }
      }
      else if(buttonIndex == _eventNumPIR){
        GPIO_DEBUG("GPIO_EVENT_PIR_ALARM: pressed %d, duration: %d", buttonPressed, lastPressDuration);
        if(lastPressDuration >= 30){
          if(fileExists("/tmp/stop_pir")){
            INFO("GPIO_EVENT_PIR_ALARM: disabled");
          }
          else{
            INFO("GPIO_EVENT_PIR_ALARM: PIR Trigger");
            NotifyObservers(GPIO_PIRALRAM, "PIR Trigger");
          }
        }
      }
      else{
        FATAL("unknow GPIO event:%d, please check it...", buttonIndex);
      }
    }
  }
  return true;
}

int GPIOMT7620::GpioSetDirectionGroup(int r, int dir)
{
  int req;

  if (_deviceFD < 0) {
    FATAL("gpio: Device is not open yet");
    return -1;
  }
  if (dir == gpio_in) {
#if defined (CONFIG_RALINK_RT3052)
    if (r == gpio5140)
      req = RALINK_GPIO5140_SET_DIR_IN;
    else if (r == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_IN;
    else
#elif defined (CONFIG_RALINK_RT3883)
    if (r == gpio9572)
      req = RALINK_GPIO9572_SET_DIR_IN;
    else if (r == gpio7140)
      req = RALINK_GPIO7140_SET_DIR_IN;
    else if (r == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_IN;
    else
#elif defined (CONFIG_RALINK_RT3352)
    if (r == gpio4540)
      req = RALINK_GPIO4540_SET_DIR_IN;
    else if (r == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_IN;
    else
#elif defined (CONFIG_RALINK_RT5350)
    if (r == gpio2722)
      req = RALINK_GPIO2722_SET_DIR_IN;
    else
#elif defined (CONFIG_RALINK_RT6855A)
    if (r == gpio3116)
      req = RALINK_GPIO3116_SET_DIR_IN;
    else
#elif defined (CONFIG_RALINK_MT7620)
    if (r == gpio72)
      req = RALINK_GPIO72_SET_DIR_IN;
    else if (r == gpio7140)
      req = RALINK_GPIO7140_SET_DIR_IN;
    else if (r == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_IN;
    else
#elif defined (CONFIG_RALINK_MT7621)
    if (r == gpio9564)
      req = RALINK_GPIO9564_SET_DIR_IN;
    else if (r == gpio6332)
      req = RALINK_GPIO6332_SET_DIR_IN;
    else
#endif
      req = RALINK_GPIO_SET_DIR_IN;
  }
  else {
#if defined (CONFIG_RALINK_RT3052)
    if (r == gpio5140)
      req = RALINK_GPIO5140_SET_DIR_OUT;
    else if (r == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_OUT;
    else
#elif defined (CONFIG_RALINK_RT3883)
    if (r == gpio9572)
      req = RALINK_GPIO9572_SET_DIR_OUT;
    else if (r == gpio7140)
      req = RALINK_GPIO7140_SET_DIR_OUT;
    else if (r == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_OUT;
#elif defined (CONFIG_RALINK_RT3352)
    if (r == gpio4540)
      req = RALINK_GPIO4540_SET_DIR_OUT;
    else if (r == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_OUT;
    else
#elif defined (CONFIG_RALINK_RT5350)
    if (r == gpio2722)
      req = RALINK_GPIO2722_SET_DIR_OUT;
    else
#elif defined (CONFIG_RALINK_RT6855A)
    if (r == gpio3116)
      req = RALINK_GPIO3116_SET_DIR_OUT;
    else
#elif defined (CONFIG_RALINK_MT7620)
    if (r == gpio72)
      req = RALINK_GPIO72_SET_DIR_OUT;
    else if (r == gpio7140)
      req = RALINK_GPIO7140_SET_DIR_OUT;
    else if (r == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_OUT;
#elif defined (CONFIG_RALINK_MT7621)
    if (r == gpio9564)
      req = RALINK_GPIO9564_SET_DIR_OUT;
    else if (r == gpio6332)
      req = RALINK_GPIO6332_SET_DIR_OUT;
    else
#endif
      req = RALINK_GPIO_SET_DIR_OUT;
  }
  if (ioctl(_deviceFD, req, 0xffffffff) < 0) {
    FATAL("gpio ioctl failed");
    return -1;
  }
  return 0;
}

int GPIOMT7620::GpioReadGroup(int r, int *value)
{
  int req;

  *value = 0;
  if (_deviceFD < 0) {
    FATAL("gpio: Device is not open yet");
    return -1;
  }

#if defined (CONFIG_RALINK_RT3052)
  if (r == gpio5140)
    req = RALINK_GPIO5140_READ;
  else if (r == gpio3924)
    req = RALINK_GPIO3924_READ;
  else
#elif defined (CONFIG_RALINK_RT3883)
  if (r == gpio9572)
    req = RALINK_GPIO9572_READ;
  else if (r == gpio7140)
    req = RALINK_GPIO7140_READ;
  else if (r == gpio3924)
    req = RALINK_GPIO3924_READ;
  else
#elif defined (CONFIG_RALINK_RT3352)
  if (r == gpio4540)
    req = RALINK_GPIO4540_READ;
  else if (r == gpio3924)
    req = RALINK_GPIO3924_READ;
  else
#elif defined (CONFIG_RALINK_RT5350)
  if (r == gpio2722)
    req = RALINK_GPIO2722_READ;
  else
#elif defined (CONFIG_RALINK_RT6855A)
  if (r == gpio3116)
    req = RALINK_GPIO3116_READ;
  else
#elif defined (CONFIG_RALINK_MT7620)
  if (r == gpio72)
    req = RALINK_GPIO72_READ;
  else if (r == gpio7140)
    req = RALINK_GPIO7140_READ;
  else if (r == gpio3924)
    req = RALINK_GPIO3924_READ;
  else
#elif defined (CONFIG_RALINK_MT7621)
  if (r == gpio9564)
    req = RALINK_GPIO9564_READ;
  else if (r == gpio6332)
    req = RALINK_GPIO6332_READ;
  else
#endif
    req = RALINK_GPIO_READ;
  if (ioctl(_deviceFD, req, value) < 0) {
    FATAL("gpio ioctl failed");
    return -1;
  }
  return 0;
}

int GPIOMT7620::GpioWriteGroup(int r, int value)
{
  int req;

  if (_deviceFD < 0) {
    FATAL("gpio: Device is not open yet");
    return -1;
  }
#if defined (CONFIG_RALINK_RT3052)
  if (r == gpio5140)
    req = RALINK_GPIO5140_WRITE;
  else if (r == gpio3924)
    req = RALINK_GPIO3924_WRITE;
  else
#elif defined (CONFIG_RALINK_RT3883)
  if (r == gpio9572)
    req = RALINK_GPIO9572_WRITE;
  else if (r == gpio7140)
    req = RALINK_GPIO7140_WRITE;
  else if (r == gpio3924)
    req = RALINK_GPIO3924_WRITE;
  else
#elif defined (CONFIG_RALINK_RT3352)
  if (r == gpio4540)
    req = RALINK_GPIO4540_WRITE;
  else if (r == gpio3924)
    req = RALINK_GPIO3924_WRITE;
  else
#elif defined (CONFIG_RALINK_RT5350)
  if (r == gpio2722)
    req = RALINK_GPIO2722_WRITE;
  else
#elif defined (CONFIG_RALINK_RT6855A)
  if (r == gpio3116)
    req = RALINK_GPIO3116_WRITE;
  else
#elif defined (CONFIG_RALINK_MT7620)
  if (r == gpio72)
    req = RALINK_GPIO72_WRITE;
  else if (r == gpio7140)
    req = RALINK_GPIO7140_WRITE;
  else if (r == gpio3924)
    req = RALINK_GPIO3924_WRITE;
  else
#elif defined (CONFIG_RALINK_MT7621)
  if (r == gpio9564)
    req = RALINK_GPIO9564_WRITE;
  else if (r == gpio6332)
    req = RALINK_GPIO6332_WRITE;
  else
#endif
    req = RALINK_GPIO_WRITE;
  if (ioctl(_deviceFD, req, value) < 0) {
    FATAL("gpio ioctl failed");
    return -1;
  }
  return 0;
}

void (*GpioTriggerFunctions[RALINK_GPIO_NUMBER])();

void GPIOMT7620::SignalHandler(int signum)
{
  //int GpioInterrputNum = -1;
  GPIO_DEBUG("SIGNAL %d received\n", signum);
  if (signum == SIGUSR1 || signum == SIGUSR2){

   // GpioGetInterruptNum(&GpioInterrputNum);
   // if(GpioInterrputNum >=0 && GpioInterrputNum <= RALINK_GPIO_NUMBER){
   //   if(NULL != GpioTriggerFunctions[GpioInterrputNum]){
   //     GpioTriggerFunctions[GpioInterrputNum]();
   //   }
   // }
  }
  else{
    FATAL("The signal is not SIGUSR1 or SIGUSR2!");
  }

}

int GPIOMT7620::GpioEnableIrq(void)
{
  if (_deviceFD < 0) {
    FATAL("gpio: Device is not open yet");
    return -1;
  }
  if (ioctl(_deviceFD, RALINK_GPIO_ENABLE_INTP) < 0) {
    FATAL("gpio ioctl failed");
    return -1;
  }
  return 0;
}

int GPIOMT7620::GpioDisableIrq(void)
{
  if (_deviceFD < 0) {
    FATAL("gpio: Device is not open yet");
    return -1;
  }
  if (ioctl(_deviceFD, RALINK_GPIO_DISABLE_INTP) < 0) {
    FATAL("gpio ioctl failed");
    return -1;
  }
  return 0;
}

int GPIOMT7620::GpioRegisterIrqInfo(int gpio_num)
{
  ralink_gpio_reg_info info;

  if (_deviceFD < 0) {
    FATAL("gpio: Device is not open yet");
    return -1;
  }
  info.pid = getpid();
  info.irq = gpio_num;
  if (ioctl(_deviceFD, RALINK_GPIO_REG_IRQ, &info) < 0) {
    FATAL("gpio ioctl failed");
    return -1;
  }
  return 0;
}

int GPIOMT7620::GpioSetDirection(uint8_t gpioNum, int dir)
{
  int req;
  int gpioGroup;
  uint8_t numInGroup = 0;
  int gpioDirctionMask = 0;

  if (_deviceFD < 0) {
    FATAL("gpio: Device is not open yet");
    return -1;
  }

  // get gpio group
#if defined (CONFIG_RALINK_RT3052)
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
  }
  else if(gpioNum >= 24 && gpioNum <= 39){
    gpioGroup = gpio3924;
  }
  else if(gpioNum >= 40 && gpioNum <= 51){
    gpioGroup = gpio4540;
  }
#elif defined (CONFIG_RALINK_RT3883)
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
  }
  else if(gpioNum >= 24 && gpioNum <= 39){
    gpioGroup = gpio3924;
  }
  else if(gpioNum >= 40 && gpioNum <= 71){
    gpioGroup = gpio7140;
  }
  else if(gpioNum >= 72 && gpioNum <= 95){
    gpioGroup = gpio9572;
  }
#elif defined (CONFIG_RALINK_RT3352)
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
    numInGroup = gpioNum;
  }
  else if(gpioNum >= 24 && gpioNum <= 39){
    gpioGroup = gpio3924;
    numInGroup = gpioNum - 24;
  }
  else if(gpioNum >= 40 && gpioNum <= 45){
    gpioGroup = gpio4540;
    numInGroup = gpioNum - 40;
  }
#elif defined (CONFIG_RALINK_RT5350)
  if(gpioNum >= 0 && gpioNum <= 21){
    gpioGroup = gpio2100;
  }
  else if(gpioNum >= 22 && gpioNum <= 27){
    gpioGroup = gpio2722;
  }
#elif defined (CONFIG_RALINK_RT6855A)
  if(gpioNum >= 0 && gpioNum <= 15){
    gpioGroup = gpio1500;
  }
  else if(gpioNum >= 16 && gpioNum <= 31){
    gpioGroup = gpio3116;
  }
#elif defined (CONFIG_RALINK_MT7620)
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
  }
  else if(gpioNum >= 24 && gpioNum <= 39){
    gpioGroup = gpio3924;
  }
  else if(gpioNum >= 40 && gpioNum <= 71){
    gpioGroup = gpio7140;
  }
  else if(gpioNum >= 72){
    gpioGroup = gpio72;
  }
#elif defined (CONFIG_RALINK_MT7621)
  if(gpioNum >= 0 && gpioNum <= 31){
    gpioGroup = gpio3100;
  }
  else if(gpioNum >= 32 && gpioNum <= 63){
    gpioGroup = gpio6332;
  }
  else if(gpioNum >= 64 && gpioNum <= 95){
    gpioGroup = gpio9564;
  }
#else
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
  }
#endif
  else{
    FATAL("GpioRead the GPIO number is out of range!");
  }

  //get set gpio in/out command
  if (dir == gpio_in) {
#if defined (CONFIG_RALINK_RT3052)
    if (gpioGroup == gpio5140)
      req = RALINK_GPIO5140_SET_DIR_IN;
    else if (gpioGroup == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_IN;
    else
#elif defined (CONFIG_RALINK_RT3883)
    if (gpioGroup == gpio9572)
      req = RALINK_GPIO9572_SET_DIR_IN;
    else if (gpioGroup == gpio7140)
      req = RALINK_GPIO7140_SET_DIR_IN;
    else if (gpioGroup == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_IN;
    else
#elif defined (CONFIG_RALINK_RT3352)
    if (gpioGroup == gpio4540)
      req = RALINK_GPIO4540_SET_DIR_IN;
    else if (gpioGroup == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_IN;
    else
#elif defined (CONFIG_RALINK_RT5350)
    if (gpioGroup == gpio2722)
      req = RALINK_GPIO2722_SET_DIR_IN;
    else
#elif defined (CONFIG_RALINK_RT6855A)
    if (gpioGroup == gpio3116)
      req = RALINK_GPIO3116_SET_DIR_IN;
    else if (gpioGroup == gpio1500)
      req = RALINK_GPIO1500_SET_DIR_IN;
    else
#elif defined (CONFIG_RALINK_MT7620)
    if (gpioGroup == gpio72)
      req = RALINK_GPIO72_SET_DIR_IN;
    else if (gpioGroup == gpio7140)
      req = RALINK_GPIO7140_SET_DIR_IN;
    else if (gpioGroup == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_IN;
    //else if (gpioGroup == gpio2300)
    //  req = RALINK_GPIO2300_SET_DIR_IN;
    else
#elif defined (CONFIG_RALINK_MT7621)
    if (gpioGroup == gpio9564)
      req = RALINK_GPIO9564_SET_DIR_IN;
    else if (gpioGroup == gpio6332)
      req = RALINK_GPIO6332_SET_DIR_IN;
    //else if (gpioGroup == gpio3100)
    //  req = RALINK_GPIO3100_SET_DIR_IN;
    else
#endif
      req = RALINK_GPIO_SET_DIR_IN;
  }
  else {
#if defined (CONFIG_RALINK_RT3052)
    if (gpioGroup == gpio5140)
      req = RALINK_GPIO5140_SET_DIR_OUT;
    else if (gpioGroup == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_OUT;
    else
#elif defined (CONFIG_RALINK_RT3883)
    if (gpioGroup == gpio9572)
      req = RALINK_GPIO9572_SET_DIR_OUT;
    else if (gpioGroup == gpio7140)
      req = RALINK_GPIO7140_SET_DIR_OUT;
    else if (gpioGroup == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_OUT;
#elif defined (CONFIG_RALINK_RT3352)
    if (gpioGroup == gpio4540)
      req = RALINK_GPIO4540_SET_DIR_OUT;
    else if (gpioGroup == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_OUT;
    else
#elif defined (CONFIG_RALINK_RT5350)
    if (gpioGroup == gpio2722)
      req = RALINK_GPIO2722_SET_DIR_OUT;
    else
#elif defined (CONFIG_RALINK_RT6855A)
    if (gpioGroup == gpio3116)
      req = RALINK_GPIO3116_SET_DIR_OUT;
    else
#elif defined (CONFIG_RALINK_MT7620)
    if (gpioGroup == gpio72)
      req = RALINK_GPIO72_SET_DIR_OUT;
    else if (gpioGroup == gpio7140)
      req = RALINK_GPIO7140_SET_DIR_OUT;
    else if (gpioGroup == gpio3924)
      req = RALINK_GPIO3924_SET_DIR_OUT;
    else
#elif defined (CONFIG_RALINK_MT7621)
    if (gpioGroup == gpio9564)
      req = RALINK_GPIO9564_SET_DIR_OUT;
    else if (gpioGroup == gpio6332)
      req = RALINK_GPIO6332_SET_DIR_OUT;
    //else if (gpioGroup == gpio3100)
    //  req = RALINK_GPIO3100_SET_DIR_OUT;
#endif
      req = RALINK_GPIO_SET_DIR_OUT;
  }

  gpioDirctionMask = (1 << numInGroup);


  //set the direction
  if (ioctl(_deviceFD, req, gpioDirctionMask) < 0) {
    FATAL("gpio ioctl failed");
    return -1;
  }
  return 0;
}

void GPIOMT7620::GpioWrite(uint8_t gpioNum, bool gpioValue)
{
  int gpioGroup;
  int groupValue = 0;
  uint8_t numInGroup = 0;

#if defined (CONFIG_RALINK_RT3052)
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
  }
  else if(gpioNum >= 24 && gpioNum <= 39){
    gpioGroup = gpio3924;
  }
  else if(gpioNum >= 40 && gpioNum <= 51){
    gpioGroup = gpio4540;
  }
#elif defined (CONFIG_RALINK_RT3883)
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
  }
  else if(gpioNum >= 24 && gpioNum <= 39){
    gpioGroup = gpio3924;
  }
  else if(gpioNum >= 40 && gpioNum <= 71){
    gpioGroup = gpio7140;
  }
  else if(gpioNum >= 72 && gpioNum <= 95){
    gpioGroup = gpio9572;
  }
#elif defined (CONFIG_RALINK_RT3352)
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
    numInGroup = gpioNum;
  }
  else if(gpioNum >= 24 && gpioNum <= 39){
    gpioGroup = gpio3924;
    numInGroup = gpioNum - 24;
  }
  else if(gpioNum >= 40 && gpioNum <= 45){
    gpioGroup = gpio4540;
    numInGroup = gpioNum - 40;
  }
#elif defined (CONFIG_RALINK_RT5350)
  if(gpioNum >= 0 && gpioNum <= 21){
    gpioGroup = gpio2100;
  }
  else if(gpioNum >= 22 && gpioNum <= 27){
    gpioGroup = gpio2722;
  }
#elif defined (CONFIG_RALINK_RT6855A)
  if(gpioNum >= 0 && gpioNum <= 15){
    gpioGroup = gpio1500;
  }
  else if(gpioNum >= 16 && gpioNum <= 31){
    gpioGroup = gpio3116;
  }
#elif defined (CONFIG_RALINK_MT7620)
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
  }
  else if(gpioNum >= 24 && gpioNum <= 39){
    gpioGroup = gpio3924;
  }
  else if(gpioNum >= 40 && gpioNum <= 71){
    gpioGroup = gpio7140;
  }
  else if(gpioNum >= 72){
    gpioGroup = gpio72;
  }
#elif defined (CONFIG_RALINK_MT7621)
  if(gpioNum >= 0 && gpioNum <= 31){
    gpioGroup = gpio3100;
  }
  else if(gpioNum >= 32 && gpioNum <= 63){
    gpioGroup = gpio6332;
  }
  else if(gpioNum >= 64 && gpioNum <= 95){
    gpioGroup = gpio9564;
  }
#else
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
  }
#endif
  else{
    FATAL("GpioRead the GPIO number is out of range!");
  }

  //set gpio direction to input
  GpioSetDirection(gpioNum, gpio_in);
  //read gpio Group
  GpioReadGroup(gpioGroup, &groupValue);

  //modify the value of the dedicated gpio
  if(1 == gpioValue){
    groupValue = groupValue | (1 << numInGroup);
  }
  else if(0 == gpioValue){
    groupValue = groupValue & (~(1 << numInGroup));
  }
  else{
    FATAL("Bad value to set");
    return;
  }

  //set gpio direction to output
  GpioSetDirection(gpioNum, gpio_out);
  //write gpio
  GpioWriteGroup(gpioGroup, groupValue);
}

bool GPIOMT7620::GpioRead(uint8_t gpioNum)
{
  int gpioGroup;
  int groupValue = 0;
  bool gpioValue = 0;
  uint8_t numInGroup = 0;

#if defined (CONFIG_RALINK_RT3052)
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
  }
  else if(gpioNum >= 24 && gpioNum <= 39){
    gpioGroup = gpio3924;
  }
  else if(gpioNum >= 40 && gpioNum <= 51){
    gpioGroup = gpio4540;
  }
#elif defined (CONFIG_RALINK_RT3883)
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
  }
  else if(gpioNum >= 24 && gpioNum <= 39){
    gpioGroup = gpio3924;
  }
  else if(gpioNum >= 40 && gpioNum <= 71){
    gpioGroup = gpio7140;
  }
  else if(gpioNum >= 72 && gpioNum <= 95){
    gpioGroup = gpio9572;
  }
#elif defined (CONFIG_RALINK_RT3352)
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
    numInGroup = gpioNum;
  }
  else if(gpioNum >= 24 && gpioNum <= 39){
    gpioGroup = gpio3924;
    numInGroup = gpioNum - 24;
  }
  else if(gpioNum >= 40 && gpioNum <= 45){
    gpioGroup = gpio4540;
    numInGroup = gpioNum - 40;
  }
#elif defined (CONFIG_RALINK_RT5350)
  if(gpioNum >= 0 && gpioNum <= 21){
    gpioGroup = gpio2100;
  }
  else if(gpioNum >= 22 && gpioNum <= 27){
    gpioGroup = gpio2722;
  }
#elif defined (CONFIG_RALINK_RT6855A)
  if(gpioNum >= 0 && gpioNum <= 15){
    gpioGroup = gpio1500;
  }
  else if(gpioNum >= 16 && gpioNum <= 31){
    gpioGroup = gpio3116;
  }
#elif defined (CONFIG_RALINK_MT7620)
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
  }
  else if(gpioNum >= 24 && gpioNum <= 39){
    gpioGroup = gpio3924;
  }
  else if(gpioNum >= 40 && gpioNum <= 71){
    gpioGroup = gpio7140;
  }
  else if(gpioNum >= 72){
    gpioGroup = gpio72;
  }
#elif defined (CONFIG_RALINK_MT7621)
  if(gpioNum >= 0 && gpioNum <= 31){
    gpioGroup = gpio3100;
  }
  else if(gpioNum >= 32 && gpioNum <= 63){
    gpioGroup = gpio6332;
  }
  else if(gpioNum >= 64 && gpioNum <= 95){
    gpioGroup = gpio9564;
  }
#else
  if(gpioNum >= 0 && gpioNum <= 23){
    gpioGroup = gpio2300;
  }
#endif
  else{
    FATAL("GpioRead the GPIO number is out of range!");
  }


  GpioSetDirection(gpioNum, gpio_in);
  GpioReadGroup(gpioGroup, &groupValue);
  gpioValue = (groupValue & (1 << gpioNum)) > 0;  //Get the value of the gpio

  GPIO_DEBUG("gpioGroup = 0x%x\n", groupValue);
  GPIO_DEBUG("The GPIO#%d = 0x%d\n", gpioNum, gpioValue);
}

void GPIOMT7620::GpioSetInterrupt(int gpio_num, void (*NotifyFunction)(void))
{

  if(NULL != NotifyFunction){
    GpioTriggerFunctions[gpio_num] = NotifyFunction;
    GPIO_DEBUG("Register gpio#%d interrupt trigger function", gpio_num);
  }
  else{
    FATAL("The Registered trigger function is NULL!");
  }

  //set gpio direction to input
  GpioSetDirection(gpio_num, gpio_in);

  //enable gpio interrupt
  GpioEnableIrq();

  //register my information
  GpioRegisterIrqInfo(gpio_num);

  //issue a handler to handle SIGUSR1
  signal(SIGUSR1, SignalHandler);
  signal(SIGUSR2, SignalHandler);

  //wait for signal
  //pause();

  //disable gpio interrupt
  //GpioDisableIrq();
}

int GPIOMT7620::GpioGetInterruptNum(int *value)
{
  *value = -1;

  if (_deviceFD < 0) {
    FATAL("gpio: Device is not open yet");
    return -1;
  }

  if (ioctl(_deviceFD, RALINK_GPIO_GET_INTERRUPT_NUMBER, value) < 0) {
    FATAL("gpio ioctl failed");
    return -1;
  }
  return 0;
}

int32_t GPIOMT7620::SetLedEvent(LEDEventType ledEvent, bool enable)
{
  if(!fileExists("/sbin/icam_set_led_status")){
    DEBUG("Cannot find icam_set_led_status command");
    return -1;
  }

  string cmd = "icam_set_led_status ";
  char led_event[128];
  sprintf(led_event, "%010u", (uint32_t)ledEvent);

  if(false == enable){
    cmd = cmd + " " + led_event + " 0";
  }else{
    cmd = cmd + " " + led_event + " 1";
  }

  DEBUG("cmd:%s", cmd.c_str());
  UnixDomainSocketManager::ExecuteSystemCommand(cmd);
  return 0;
}

void GPIOMT7620::GpioSetLed(uint8_t gpio, uint16_t on, uint16_t off, uint16_t blinks, uint16_t resets, uint16_t times)
{
  ralink_gpio_led_info led;

  led.gpio = gpio;
  if (led.gpio < 0 || led.gpio >= RALINK_GPIO_NUMBER) {
    FATAL("gpio number %d out of range (should be 0 ~ %d)\n", led.gpio, RALINK_GPIO_NUMBER);
    return;
  }
  led.on = on;
  if (led.on > RALINK_GPIO_LED_INFINITY) {
    FATAL("on interval %d out of range (should be 0 ~ %d)\n", led.on, RALINK_GPIO_LED_INFINITY);
    return;
  }
  led.off = off;
  if (led.off > RALINK_GPIO_LED_INFINITY) {
    FATAL("off interval %d out of range (should be 0 ~ %d)\n", led.off, RALINK_GPIO_LED_INFINITY);
    return;
  }
  led.blinks = blinks;
  if (led.blinks > RALINK_GPIO_LED_INFINITY) {
    FATAL("number of blinking cycles %d out of range (should be 0 ~ %d)\n", led.blinks, RALINK_GPIO_LED_INFINITY);
    return;
  }
  led.rests = resets;
  if (led.rests > RALINK_GPIO_LED_INFINITY) {
    FATAL("number of resting cycles %d out of range (should be 0 ~ %d)\n", led.rests, RALINK_GPIO_LED_INFINITY);
    return;
  }
  led.times = times;
  if (led.times > RALINK_GPIO_LED_INFINITY) {
    FATAL("times of blinking %d out of range (should be 0 ~ %d)\n", led.times, RALINK_GPIO_LED_INFINITY);
    return;
  }

  if (_deviceFD < 0) {
    FATAL("gpio: Device is not open yet");
    return;
  }
  if (ioctl(_deviceFD, RALINK_GPIO_LED_SET, &led) < 0) {
    FATAL("gpio ioctl failed");
    return;
  }
}

int GPIOMT7620::GpioGetAllInterrupts(ralink_gpio_interrupt_record (&gpio_record)[RALINK_GPIO_NUMBER])
{
  if (_deviceFD < 0) {
    FATAL("gpio: Device is not open yet");
    return -1;
  }

  if (ioctl(_deviceFD, RALINK_GPIO_GET_INTERRUPTS, &gpio_record) < 0) {
    FATAL("gpio ioctl failed");
    return 0;
  }

#ifdef HAS_GPIO_DEBUG
  for(uint8_t i = 0 ; i < RALINK_GPIO_NUMBER ; i++){
    if(gpio_record[i].triggered_duration > 0)
      FATAL("Index %ld pressed? %d period(miliseconds) %ld\n",
                                i,
                                gpio_record[i].current_status,
                                gpio_record[i].triggered_duration);
  }
#endif
  return 0;
}

//------------------------only for test------------------------
void GPIOMT7620::GpioTestWrite(void)
{
  int i = 0;

  //set gpio direction to output
#if defined (CONFIG_RALINK_RT3052)
  GpioSetDirectionGroup(gpio5140, gpio_out);
  GpioSetDirectionGroup(gpio3924, gpio_out);
  GpioSetDirectionGroup(gpio2300, gpio_out);
#elif defined (CONFIG_RALINK_RT3883)
  GpioSetDirectionGroup(gpio9572, gpio_out);
  GpioSetDirectionGroup(gpio7140, gpio_out);
  GpioSetDirectionGroup(gpio3924, gpio_out);
  GpioSetDirectionGroup(gpio2300, gpio_out);
#elif defined (CONFIG_RALINK_RT3352)
  GpioSetDirectionGroup(gpio4540, gpio_out);
  GpioSetDirectionGroup(gpio3924, gpio_out);
  GpioSetDirectionGroup(gpio2300, gpio_out);
#elif defined (CONFIG_RALINK_RT5350)
  GpioSetDirectionGroup(gpio2722, gpio_out);
  GpioSetDirectionGroup(gpio2100, gpio_out);
#elif defined (CONFIG_RALINK_RT6855A)
  GpioSetDirectionGroup(gpio3116, gpio_out);
  GpioSetDirectionGroup(gpio1500, gpio_out);
#elif defined (CONFIG_RALINK_MT7620)
  GpioSetDirectionGroup(gpio72, gpio_out);
  GpioSetDirectionGroup(gpio7140, gpio_out);
  GpioSetDirectionGroup(gpio3924, gpio_out);
  GpioSetDirectionGroup(gpio2300, gpio_out);
#elif defined (CONFIG_RALINK_MT7621)
  GpioSetDirectionGroup(gpio9564, gpio_out);
  GpioSetDirectionGroup(gpio6332, gpio_out);
  GpioSetDirectionGroup(gpio3100, gpio_out);
#else
  GpioSetDirectionGroup(gpio2300, gpio_out);
#endif

  //turn off LEDs
#if defined (CONFIG_RALINK_RT3052)
  GpioWriteGroup(gpio5140, 0xffffffff);
  GpioWriteGroup(gpio3924, 0xffffffff);
  GpioWriteGroup(gpio2300, 0xffffffff);
#elif defined (CONFIG_RALINK_RT3883)
  GpioWriteGroup(gpio9572, 0xffffffff);
  GpioWriteGroup(gpio7140, 0xffffffff);
  GpioWriteGroup(gpio3924, 0xffffffff);
  GpioWriteGroup(gpio2300, 0xffffffff);
#elif defined (CONFIG_RALINK_RT3352)
  GpioWriteGroup(gpio4540, 0xffffffff);
  GpioWriteGroup(gpio3924, 0xffffffff);
  GpioWriteGroup(gpio2300, 0xffffffff);
#elif defined (CONFIG_RALINK_RT5350)
  GpioWriteGroup(gpio2722, 0xffffffff);
  GpioWriteGroup(gpio2100, 0xffffffff);
#elif defined (CONFIG_RALINK_RT6855A)
  GpioWriteGroup(gpio3116, 0x0000ffff);
  GpioWriteGroup(gpio1500, 0xffff0000);
#elif defined (CONFIG_RALINK_MT7620)
  GpioWriteGroup(gpio72, 0xffffffff);
  GpioWriteGroup(gpio7140, 0xffffffff);
  GpioWriteGroup(gpio3924, 0xffffffff);
  GpioWriteGroup(gpio2300, 0xffffffff);
#elif defined (CONFIG_RALINK_MT7621)
  GpioWriteGroup(gpio9564, 0xffffffff);
  GpioWriteGroup(gpio6332, 0xffffffff);
  GpioWriteGroup(gpio3100, 0xffffffff);
#else
  GpioWriteGroup(gpio2300, 0xffffffff);
#endif
  sleep(3);

  //turn on all LEDs
#if defined (CONFIG_RALINK_RT3052)
  GpioWriteGroup(gpio5140, 0);
  GpioWriteGroup(gpio3924, 0);
  GpioWriteGroup(gpio2300, 0);
#elif defined (CONFIG_RALINK_RT3883)
  GpioWriteGroup(gpio9572, 0);
  GpioWriteGroup(gpio7140, 0);
  GpioWriteGroup(gpio3924, 0);
  GpioWriteGroup(gpio2300, 0);
#elif defined (RALINK_GPIO_HAS_3352)
  GpioWriteGroup(gpio4540, 0);
  GpioWriteGroup(gpio3924, 0);
  GpioWriteGroup(gpio2300, 0);
#elif defined (CONFIG_RALINK_RT5350)
  GpioWriteGroup(gpio2722, 0);
  GpioWriteGroup(gpio2100, 0);
#elif defined (CONFIG_RALINK_RT6855A)
  GpioWriteGroup(gpio3116, 0);
  GpioWriteGroup(gpio1500, 0);
#elif defined (CONFIG_RALINK_MT7620)
  GpioWriteGroup(gpio72, 0);
  GpioWriteGroup(gpio7140, 0);
  GpioWriteGroup(gpio3924, 0);
  GpioWriteGroup(gpio2300, 0);
#elif defined (CONFIG_RALINK_MT7621)
  GpioWriteGroup(gpio9564, 0);
  GpioWriteGroup(gpio6332, 0);
  GpioWriteGroup(gpio3100, 0);
#else
  GpioWriteGroup(gpio2300, 0);
#endif
}

void GPIOMT7620::GpioTestRead(void)
{
  int i, d;

#if defined (CONFIG_RALINK_RT3052)
  GpioSetDirectionGroup(gpio5140, gpio_in);
  GpioReadGroup(gpio5140, &d);
  GPIO_DEBUG("gpio 51~40 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio3924, gpio_in);
  GpioReadGroup(gpio3924, &d);
  GPIO_DEBUG("gpio 39~24 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio2300, gpio_in);
  GpioReadGroup(gpio2300, &d);
  GPIO_DEBUG("gpio 23~00 = 0x%x\n", d);
#elif defined (CONFIG_RALINK_RT3883)
  GpioSetDirectionGroup(gpio9572, gpio_in);
  GpioReadGroup(gpio9572, &d);
  GPIO_DEBUG("gpio 95~72 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio7140, gpio_in);
  GpioReadGroup(gpio7140, &d);
  GPIO_DEBUG("gpio 71~40 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio3924, gpio_in);
  GpioReadGroup(gpio3924, &d);
  GPIO_DEBUG("gpio 39~24 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio2300, gpio_in);
  GpioReadGroup(gpio2300, &d);
  GPIO_DEBUG("gpio 23~00 = 0x%x\n", d);
#elif defined (CONFIG_RALINK_RT3352)
  GpioSetDirectionGroup(gpio4540, gpio_in);
  GpioReadGroup(gpio4540, &d);
  GPIO_DEBUG("gpio 45~40 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio3924, gpio_in);
  GpioReadGroup(gpio3924, &d);
  GPIO_DEBUG("gpio 39~24 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio2300, gpio_in);
  GpioReadGroup(gpio2300, &d);
  GPIO_DEBUG("gpio 23~00 = 0x%x\n", d);
#elif defined (CONFIG_RALINK_RT5350)
  GpioSetDirectionGroup(gpio2722, gpio_in);
  GpioReadGroup(gpio2722, &d);
  GPIO_DEBUG("gpio 27~22 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio2100, gpio_in);
  GpioReadGroup(gpio2100, &d);
  GPIO_DEBUG("gpio 21~00 = 0x%x\n", d);
#elif defined (CONFIG_RALINK_RT6855A)
  GpioSetDirectionGroup(gpio3116, gpio_in);
  GpioReadGroup(gpio3116, &d);
  GPIO_DEBUG("gpio 31~16 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio1500, gpio_in);
  GpioReadGroup(gpio1500, &d);
  GPIO_DEBUG("gpio 15~00 = 0x%x\n", d);
#elif defined (CONFIG_RALINK_MT7620)
  GpioSetDirectionGroup(gpio72, gpio_in);
  GpioReadGroup(gpio72, &d);
  GPIO_DEBUG("gpio 72 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio7140, gpio_in);
  GpioReadGroup(gpio7140, &d);
  GPIO_DEBUG("gpio 71~40 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio3924, gpio_in);
  GpioReadGroup(gpio3924, &d);
  GPIO_DEBUG("gpio 39~24 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio2300, gpio_in);
  GpioReadGroup(gpio2300, &d);
  GPIO_DEBUG("gpio 23~00 = 0x%x\n", d);
#elif defined (CONFIG_RALINK_MT7621)
  GpioSetDirectionGroup(gpio9564, gpio_in);
  GpioReadGroup(gpio9564, &d);
  GPIO_DEBUG("gpio 95~64 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio6332, gpio_in);
  GpioReadGroup(gpio6332, &d);
  GPIO_DEBUG("gpio 63~32 = 0x%x\n", d);

  GpioSetDirectionGroup(gpio3100, gpio_in);
  GpioReadGroup(gpio3100, &d);
  GPIO_DEBUG("gpio 31~00 = 0x%x\n", d);
#else
  GpioSetDirectionGroup(gpio2300, gpio_in);
  GpioReadGroup(gpio2300, &d);
  GPIO_DEBUG("gpio 23~00 = 0x%x\n", d);
#endif
}

void GPIOMT7620::GpioTestInterrupt(int gpio_num)
{
  //set gpio direction to input
#if defined (CONFIG_RALINK_RT3052)
  GpioSetDirectionGroup(gpio5140, gpio_in);
  GpioSetDirectionGroup(gpio3924, gpio_in);
  GpioSetDirectionGroup(gpio2300, gpio_in);
#elif defined (CONFIG_RALINK_RT3352)
  GpioSetDirectionGroup(gpio4540, gpio_in);
  GpioSetDirectionGroup(gpio3924, gpio_in);
  GpioSetDirectionGroup(gpio2300, gpio_in);
#elif defined (CONFIG_RALINK_RT3883)
  GpioSetDirectionGroup(gpio9572, gpio_in);
  GpioSetDirectionGroup(gpio7140, gpio_in);
  GpioSetDirectionGroup(gpio3924, gpio_in);
  GpioSetDirectionGroup(gpio2300, gpio_in);
#elif defined (CONFIG_RALINK_RT5350)
  GpioSetDirectionGroup(gpio2722, gpio_in);
  GpioSetDirectionGroup(gpio2100, gpio_in);
#elif defined (CONFIG_RALINK_RT6855A)
  GpioSetDirectionGroup(gpio3116, gpio_in);
  GpioSetDirectionGroup(gpio1500, gpio_in);
#elif defined (CONFIG_RALINK_MT7620)
  GpioSetDirectionGroup(gpio72, gpio_in);
  GpioSetDirectionGroup(gpio7140, gpio_in);
  GpioSetDirectionGroup(gpio3924, gpio_in);
  GpioSetDirectionGroup(gpio2300, gpio_in);
#elif defined (CONFIG_RALINK_MT7621)
  GpioSetDirectionGroup(gpio9564, gpio_in);
  GpioSetDirectionGroup(gpio6332, gpio_in);
  GpioSetDirectionGroup(gpio3100, gpio_in);
#else
  GpioSetDirectionGroup(gpio2300, gpio_in);
#endif

  //enable gpio interrupt
  GpioEnableIrq();

  //register my information
  GpioRegisterIrqInfo(gpio_num);

  //issue a handler to handle SIGUSR1
  signal(SIGUSR1, SignalHandler);
  signal(SIGUSR2, SignalHandler);

  //wait for signal
  pause();

  //disable gpio interrupt
  GpioDisableIrq();
}



void GPIOMT7620::CheckStatus(double ts) {
//  NYI;
}

bool GPIOMT7620::Close() {
}

// PIR
int32_t GPIOMT7620::EnablePIRNotification()
{
  _currentSetting.enablePIR = true;
}

int32_t GPIOMT7620::DisablePIRNotification()
{
  _currentSetting.enablePIR = false;
}

void GPIOMT7620::PIRAlarm()
{
  DEBUG("PIR Alarm !!");
  NotifyObservers(GPIO_PIRALRAM, "PIR Trigger");
}
#endif
