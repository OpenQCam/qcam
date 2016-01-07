#ifndef _AUDIOCAPTUREDEVICE_H
#define _AUDIOCAPTUREDEVICE_H

#include <sys/ioctl.h>
//#include <char/i2c_drv.h>
#include <char/pcm/pcm_ctrl.h>
#include "audio/baseaudiocapturedevice.h"


class I2SAudioCapDevice
: public BaseAudioCaptureDevice {
  private:
    static BaseHardwareAbstractDevice *_pDeviceInstance;
    pcm_record_type _pcm_record;
    uint8_t buffer[PCM_PAGE_SIZE];
    uint32_t _offset;
    int32_t _dropFrame;

  private:
    I2SAudioCapDevice(int32_t fd);
    // Device singleton Interface

  public:
    static BaseHardwareAbstractDevice* GetInstance(string devNode);
    static bool GetAudioCaptureDeviceName(string& audioDeviceName);
    virtual ~I2SAudioCapDevice();

    // BaseHardwareAbstractDevice
    virtual bool InitializeDevice();
    virtual bool OnReadEvent();
    virtual void CheckStatus(double ts);

    virtual bool IsStart();
    virtual bool Close();
    virtual void Start();
    virtual void Stop();
    virtual bool SetSampleRate(uint32_t samplerate);
    virtual void SetNumberOfChannels(uint32_t numberOfChannels);
    virtual void SetVolume(uint32_t vol);
    virtual int32_t GetVolume();
    virtual int32_t GetAudioFD();
    virtual int32_t GetSampleRate();
    virtual int32_t GetBitsPerSample();
    virtual int32_t GetNumberOfChannels();
    virtual const device_parameters& GetDeviceParameters();
};

#endif /*_AUDIOCAPTUREDEVICE_H*/
