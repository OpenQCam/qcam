// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    : 2012/05/15
// // Version :
// // Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose : ALSA audio class for handling alsa relevant procedures
// // ======================================================================
//


#ifndef _AUDIOCAPTUREDEVICE_H
#define	_AUDIOCAPTUREDEVICE_H


#include "audio/baseaudiocapturedevice.h"
#include <alsa/asoundlib.h>

//Enum needed to choose the type of I/O loop
typedef enum {
    METHOD_DIRECT_RW,   //method with direct use of read/write functions
    METHOD_DIRECT_MMAP, //method with direct use of memory mapping
    METHOD_ASYNC_MMAP,  //method with async use of memory mapping
    METHOD_ASYNC_RW,    //method with async use of read/write functions
    METHOD_RW_AND_POLL, //method with use of read/write functions and pool
    METHOD_RW_AND_SELECT, //method with use of read/write functions and select
    METHOD_DIRECT_RW_NI //method with direct use of read/write and noninterleaved format (not implemented)
} enum_io_method;

//struct that defines one I/O method, for ALSA device only
struct io_method {
    enum_io_method method;   //I/O loop type
    snd_pcm_access_t access; //PCM access type
    int open_mode;           //open function flags
};

//special parameters applid to alsa device
typedef struct _alsa_parameters {
  snd_pcm_uframes_t buffer_size;    //buffer size in frames
  snd_pcm_uframes_t period_size;    //period size in frames
  snd_pcm_format_t sample_format;   //format of the samples
  snd_pcm_access_t access_type;     //PCM access type
  snd_pcm_hw_params_t *hw_params;   //hardware configuration structure
} alsa_parameters;

class AlsaAudioCapDevice
: public BaseAudioCaptureDevice {
  private:
    static BaseHardwareAbstractDevice *_pDeviceInstance;
    struct pollfd *_pUFDS;
    snd_pcm_t *_pDevice;
    alsa_parameters _alsaParams;
    string _deviceName;
    bool _isStart;

    static bool Open(string devName, alsa_parameters& alsaParams, device_parameters& deviceParams, snd_pcm_t* &pDevice, struct pollfd* &pUFDS);
    //FIXME-use alsa_parameters to put other parameters
    AlsaAudioCapDevice(string devName, alsa_parameters alsaParams, device_parameters deviceParams, snd_pcm_t* pDevice, struct pollfd *pUFDS);

  public:
    static BaseHardwareAbstractDevice* GetInstance(string audioDeviceName);
    static bool GetAudioCaptureDeviceName(string& audioDeviceName);
    virtual ~AlsaAudioCapDevice();

    //BaseHardwareINterface
    virtual bool InitializeDevice();
    virtual bool OnReadEvent();

    virtual bool Close();
    virtual void Start();
    virtual void Stop();
    virtual bool SetSampleRate(uint32_t samplerate);
    virtual void SetSampleFormat(snd_pcm_format_t format);
    virtual void SetBufferSize(snd_pcm_uframes_t size);
    virtual void SetNumberOfChannels(uint32_t number);
    virtual void SetPeriodSize(snd_pcm_uframes_t size);
    virtual void SetVolume(uint32_t vol);
    virtual int32_t GetVolume();
    virtual bool OnAudioDataReady();
    virtual bool IsStart();

    virtual int32_t GetAudioFD();
    virtual int32_t GetSampleRate();
    virtual int32_t GetBitsPerSample();
    virtual int32_t GetNumberOfChannels();
    virtual uint32_t GetRMSWindowSize();
    virtual void CheckStatus(double ts);

  private:
    int32_t Recovery(snd_pcm_t *handle, int32_t error);
    //int32_t IOSelect();
    void ProcessAudioData(uint8_t *buf, uint32_t size);
    void UpdateAudioParameters();
};


#endif	/* _AUDIOCAPTUREDEVICE_H */
