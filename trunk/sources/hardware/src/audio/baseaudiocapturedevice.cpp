#include "audio/baseaudiocapturedevice.h"
#include <cmath>

BaseAudioCaptureDevice::BaseAudioCaptureDevice(HardwareType hwType, int32_t fd)
  : BaseAudioDevice(hwType, fd),
    _rmsTh(-1),
    _rmsEnable(false),
    _alarmMask(false),
    _alarmEnable(true)
{
}

BaseAudioCaptureDevice::~BaseAudioCaptureDevice()
{
}

bool BaseAudioCaptureDevice::GetAlarmStatus() {
  return _alarmEnable;
}

void BaseAudioCaptureDevice::EnableAlarm() {
  _alarmEnable = true;
}

void BaseAudioCaptureDevice::DisableAlarm() {
  _alarmEnable = false;
}

void BaseAudioCaptureDevice::SetAudioRMS(double rmsTh) {
  _rmsTh=rmsTh;
}

double BaseAudioCaptureDevice::GetAudioRMS() {
  return _rmsTh;
}

double BaseAudioCaptureDevice::DetectSoundEvent(uint8_t *buf, uint32_t size)
{
  double curEnergy=0;
  uint32_t offset=6;
  static double rms=0;

  for (uint32_t i=0; i < size; i+=(1<<offset)) {
    int16_t absValue=*((int16_t*)(buf+i));
    curEnergy += (absValue*absValue);
  }

  size = size>>offset;
  curEnergy = sqrt(curEnergy/size);
  rms = 20*log10(curEnergy) - 96.329; //20*log10(1/65536)

  if (rms>_rmsTh && !_alarmMask && _alarmEnable) {
    string rmsStr=  "rms:";
    rmsStr += numToString<double>(rms, 3);
    AudioAlarm(rmsStr);
    _alarmMask=true;
  }

  return rms;
}

void BaseAudioCaptureDevice::AudioAlarm(string& alarmMSG)
{
  NotifyObservers(ACM_DETECTED, alarmMSG.c_str());
}

void BaseAudioCaptureDevice::EnableRMS() {
  _rmsEnable=true;
}

void BaseAudioCaptureDevice::DisableRMS() {
  _rmsEnable=false;
}

bool BaseAudioCaptureDevice::IsRMSEnabled() {
  return _rmsEnable;
}
