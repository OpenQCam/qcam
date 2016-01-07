#ifndef _BASEAUDIOCAPTUREDEVICE_H
#define _BASEAUDIOCAPTUREDEVICE_H

#include "baseaudiodevice.h"

class BaseAudioCaptureDevice
: public BaseAudioDevice
{
  private:
    double _rmsTh;
    bool _rmsEnable;

  protected:
    BaseAudioCaptureDevice(HardwareType hwType, int32_t fd);
    virtual ~BaseAudioCaptureDevice();
    virtual double DetectSoundEvent(uint8_t *buf, uint32_t size);
    virtual void AudioAlarm(string& rmsStr);
    bool _alarmMask;
    bool _alarmEnable;

  public:
    void SetAudioRMS(double rmsTh);
    double GetAudioRMS();
    void ResetAlarmMask() {_alarmMask=false;}
    void EnableAlarm();
    void DisableAlarm();
    void EnableRMS();
    void DisableRMS();
    bool IsRMSEnabled();
    bool GetAlarmStatus();
    virtual bool IsStart()=0;
};


#endif
