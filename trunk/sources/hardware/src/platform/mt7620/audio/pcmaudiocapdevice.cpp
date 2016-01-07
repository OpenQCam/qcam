
#include "audio/pcmaudiocapdevice.h"

//#define PCMDEBUG
#ifdef PCMDEBUG
FILE *pcmfile;
#endif

#define SAMPLERATE 22050
#define NCHANNEL 1
#define NBITS 16
#define DROPFRAME 48


BaseHardwareAbstractDevice *I2SAudioCapDevice::_pDeviceInstance = NULL;

//static member function
BaseHardwareAbstractDevice* I2SAudioCapDevice::GetInstance(string deve) {
  if(_pDeviceInstance == NULL){
    int32_t fd;
    if((fd = open(STR(deve), O_RDWR, 0)) < 0) {
      FATAL("Cannot open device %s", STR(deve));
      return NULL;
    }
    _pDeviceInstance = (BaseHardwareAbstractDevice *) new I2SAudioCapDevice(fd);
    if(!_pDeviceInstance->InitializeDevice()){
      delete _pDeviceInstance;
      return NULL;
    }
  }
  return _pDeviceInstance;
}

I2SAudioCapDevice::I2SAudioCapDevice(int32_t fd)
: BaseAudioCaptureDevice(HT_MIC, fd)
{
  _offset = 0;
  _isStart = false;
  _pcm_record.size = 0;
  _deviceFD=fd;
#ifdef PCMDEBUG
  pcmfile = fopen ("test_16km.wav", "rb");
  if (pcmfile == NULL) {
    DEBUG ("open file failed");
  }
  else {
    fseek (pcmfile, 44, SEEK_SET);
  }
#endif
}

I2SAudioCapDevice::~I2SAudioCapDevice()
{
  Close();
}

bool I2SAudioCapDevice::InitializeDevice() {
  return true;
}

bool I2SAudioCapDevice::OnReadEvent() {
  int ret=-1;
  struct timeval ts;

  //state is not ok return
  if (_isStart == false) {
    return true;
  }

  ret = ioctl (_deviceFD, PCM_READ_PCM, &_pcm_record);

#ifdef PCMDEBUG
  int readsize=0;
  if (pcmfile != NULL) {
    readsize = fread (_pcm_record.pcmbuf, 1, _pcm_record.size, pcmfile);
    if (readsize != _pcm_record.size) {
      DEBUG ("end of read file");
      fclose (pcmfile);
      exit(1);
    }
  }
#endif

  if (_dropFrame > 0) {
    _dropFrame --;
    return true;
  }

  //TODO(recardo): detect audio event here
  if ((_pcm_record.size>0) && (ret!=-1)) {
     SendDataToCarriers((uint8_t*)_pcm_record.pcmbuf, (uint32_t)_pcm_record.size,
                        0, HW_DATA_AUDIO);
     string rmsValue= numToString(DetectSoundEvent((uint8_t*)_pcm_record.pcmbuf, (uint32_t)_pcm_record.size), 2);
     if (IsRMSEnabled()) {
       NotifyObservers(ACM_RMSVALUE, rmsValue);
     }
  }
  else
    DEBUG ("_pcm_record.size == 0");

  _pcm_record.size = 0;
  return true;
}

bool I2SAudioCapDevice::IsStart()
{
  return _isStart;
}

void I2SAudioCapDevice::Start()
{
  if (_isStart)
    return ;
  DEBUG ("PCM_PAGE_SIZE:%d", PCM_PAGE_SIZE);
  _pcm_record.pcmbuf =(char*)buffer;
  if (_pcm_record.pcmbuf <= 0) {
    FATAL ("mmap failed in audio cap:%d", _pcm_record.pcmbuf);
    exit (-1);
    return;
  }
  if (_deviceFD != -1) {
    ioctl (_deviceFD, PCM_START, NULL);
    DEBUG ("SET_RECORD");
    ioctl(_deviceFD, PCM_SET_RECORD, 0); //left channel id = 0
    _isStart = true;
  }
  ENABLE_DEVICE_READ_DATA;
  NotifyObservers(ACM_STARTED, "Start pcm audio capture");

  _dropFrame=DROPFRAME;
}

void I2SAudioCapDevice::Stop()
{
  DEBUG ("audio cap stop state:%d", _isStart);

  if (_deviceFD != -1 && _isStart) {
    DEBUG ("SET_UNRECORD");
    ioctl(_deviceFD, PCM_SET_UNRECORD, 0); //left channel id = 0;
    ioctl(_deviceFD, PCM_STOP, NULL);
    _isStart = false;
  }

  DISABLE_DEVICE_READ_DATA;
  NotifyObservers(ACM_STOPPED, "Start pcm audio capture");
}

bool I2SAudioCapDevice::Close()
{
  Stop();
  if (_deviceFD != -1) {
    close (_deviceFD);
    _deviceFD=-1;
  }
  return true;
}

const device_parameters& I2SAudioCapDevice::GetDeviceParameters()
{
  _deviceParams.sample_rate = this->GetSampleRate();
  _deviceParams.n_channels = this->GetNumberOfChannels();
  _deviceParams.bitwidth = this->GetBitsPerSample();

  return _deviceParams;
}

bool I2SAudioCapDevice::SetSampleRate(uint32_t samplerate)
{
  DEBUG ("I2SAudioCap::SetSampleRate not Supported");
  return false;
}

void I2SAudioCapDevice::SetNumberOfChannels(uint32_t numberOfChannels)
{
  DEBUG ("I2SAudioCap::SetNumberOfChannels not Supported");
  return;
}

void I2SAudioCapDevice::SetVolume(uint32_t volume)
{
  DEBUG ("I2SAudioCap::SetVolume not Supported");
  return;
}

int32_t I2SAudioCapDevice::GetVolume()
{
  return 0;
}

int32_t I2SAudioCapDevice::GetAudioFD()
{
  return _deviceFD;
}

int32_t I2SAudioCapDevice::GetSampleRate()
{
  return SAMPLERATE;
}

int32_t I2SAudioCapDevice::GetBitsPerSample()
{
  return NBITS;
}

int32_t I2SAudioCapDevice::GetNumberOfChannels()
{
  return NCHANNEL;
}

void I2SAudioCapDevice::CheckStatus(double ts) {
//  NYI;
}
