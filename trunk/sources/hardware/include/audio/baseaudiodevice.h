// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    : 2012/11/07
// // Version :
// // Author  : Recardo Cheng(recardo.cheng@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose : Audio Capture Interface to adapt different audio devices in different platform
// // ======================================================================
//

#ifndef _BASEAUDIODEVICE_H
#define _BASEAUDIODEVICE_H
#include "common.h"
#include "hardwaretypes.h"
#include "basehardwareabstractdevice.h"


//a general device parameters applied to all kind of audio device (I2S/USB audio etc...)
typedef struct _device_parameters {
    uint32_t buffer_time;           //length of the circular buffer in usec
    uint32_t period_time;           //length of one period in usec
    uint32_t n_channels;            //number of channels
    uint32_t sample_rate;           //frame rate
    uint32_t bitwidth;              //bit depth per sample in bits
} device_parameters;

class BaseAudioDevice
: public BaseHardwareAbstractDevice
{
  private:
  protected:
    device_parameters _deviceParams;
    int32_t _volume;
    bool _isStart;
    BaseAudioDevice(HardwareType hwType, int32_t fd);
    virtual ~BaseAudioDevice();

  public:
    //virtual bool Open()=0;

    virtual bool IsStart()=0;
    virtual void Start()=0;
    virtual void Stop()=0;
    virtual bool SetSampleRate(uint32_t samplerate) {_deviceParams.sample_rate=samplerate;return true;}
    virtual void SetNumberOfChannels(uint32_t numberOfChannels) {_deviceParams.n_channels=numberOfChannels;}
    virtual void SetVolume(uint32_t vol) {_volume=vol;}
    virtual int32_t GetVolume() {return _volume;}

    virtual int32_t GetSampleRate() {return _deviceParams.sample_rate;}
    virtual int32_t GetBitsPerSample() {return _deviceParams.bitwidth;}
    virtual int32_t GetNumberOfChannels() {return _deviceParams.sample_rate;}

    //Capturer
    virtual const device_parameters& GetDeviceParameters();
    virtual void AudioAlarm(string& alramMSG) {NYI;}
    //Player
    virtual uint32_t GetAudioFrameSize();
    virtual int32_t PutData(uint8_t *pData, uint32_t size);
};


#endif
