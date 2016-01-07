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
// // Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//

#include "audio/alsaaudiocapdevice.h"
#include "streaming/basestream.h"

BaseHardwareAbstractDevice *AlsaAudioCapDevice::_pDeviceInstance = NULL;

/* static method */
bool AlsaAudioCapDevice::GetAudioCaptureDeviceName(string& audioDeviceName)
{
  register int  err;
  int           cardNum;
#define TMP_CHAR_LEN 100
  // Start with first cardNumi
  cardNum = -1;

  for (;;) {
    // Get next sound card's card number. When "cardNum" == -1, then ALSA
    // fetches the first card
    if ((err = snd_card_next(&cardNum)) < 0) {
       printf("Can't get the next card number: %s\n", snd_strerror(err));
       break;
    }
    else {
      char *tmpname=new char[TMP_CHAR_LEN];
      if (snd_card_get_longname(cardNum, (char**)&tmpname) != 0) {
        DEBUG ("snd_card_Get_longname fail, %s", tmpname);
        delete [] tmpname;
        return false;
      }

      string tmpnameStr(tmpname);
      if (tmpnameStr.size() > TMP_CHAR_LEN) {
        delete [] tmpname;
        DEBUG("char overflow!!\n");
        return false;
      }

      delete [] tmpname;
      if (tmpnameStr.find("usb") != string::npos) {
       std::ostringstream stm;
       stm<<cardNum;
       audioDeviceName = "hw:"+stm.str()+",0";
       DEBUG ("\nUSB AUDIO DEVICE:%s\n", STR(audioDeviceName));
       return true;
      }
    }
  }

  return false;
}

BaseHardwareAbstractDevice* AlsaAudioCapDevice::GetInstance(string devName) {
  if(_pDeviceInstance == NULL){
    if (AlsaAudioCapDevice::GetAudioCaptureDeviceName(devName)) {
      snd_pcm_t *pDevice;
      struct pollfd *pUFDS;
      alsa_parameters alsaParams;
      device_parameters deviceParams;

      deviceParams.sample_rate=44100;
      deviceParams.n_channels=2;
      deviceParams.bitwidth=16;
      alsaParams.buffer_size= 4096; //2048
      alsaParams.period_size= 512; //256

      alsaParams.sample_format=SND_PCM_FORMAT_S16_LE;
      if (AlsaAudioCapDevice::Open(devName, alsaParams, deviceParams, pDevice, pUFDS)) {
        _pDeviceInstance = (BaseHardwareAbstractDevice *) new AlsaAudioCapDevice(devName, alsaParams, deviceParams, pDevice, pUFDS);
        if(!_pDeviceInstance->InitializeDevice()){
          delete _pDeviceInstance;
          return NULL;
        }
      }
    }
  }
  return _pDeviceInstance;
}

bool AlsaAudioCapDevice::Open(string devName, alsa_parameters& alsaParams, device_parameters& deviceParams,
    snd_pcm_t* &pDevice, struct pollfd* &pUFDS) {

  int32_t error;
  /************************************** opens the device *****************************************/
  if ((error = snd_pcm_open (&pDevice, STR(devName), SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "microphone: Device cannot be opened  %s (%s)\n", STR(devName), snd_strerror (error));
    return false;
  }
  fprintf (stderr, "microphone: Device: %s open_mode = %d\n", STR(devName), 0);

  //allocating the hardware configuration structure
  if ((error = snd_pcm_hw_params_malloc (&alsaParams.hw_params)) < 0) {
    fprintf (stderr, "microphone: Hardware configuration structure cannot be allocated (%s)\n", snd_strerror (error));
    return false;
  }

  //assigning the hw configuration structure to the device
  if ((error = snd_pcm_hw_params_any (pDevice, alsaParams.hw_params)) < 0) {
    fprintf (stderr, "microphone: Hardware configuration structure cannot be assigned to device (%s)\n", snd_strerror (error));
    return false;
  }

  /*************************** configures access method ******************************************/
  //sets the configuration method
  fprintf (stderr, "microphone: Access method: %d\n", SND_PCM_ACCESS_RW_INTERLEAVED);
  if ((error = snd_pcm_hw_params_set_access (pDevice, alsaParams.hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "microphone: Access method cannot be configured (%s)\n", snd_strerror (error));
    return false;
  }
  //checks the access method
  if ((error = snd_pcm_hw_params_get_access (alsaParams.hw_params, &alsaParams.access_type)) < 0) {
    fprintf (stderr, "microphone: Access method cannot be obtained (%s)\n", snd_strerror (error));
    return false;
  }

  //shows the access method
  if (alsaParams.access_type != SND_PCM_ACCESS_RW_INTERLEAVED) {
    FATAL ("ACCESS method:%d\n", alsaParams.access_type);
    return false;
  }

  //_ioMethod = method;
#if 0
  SetSampleRate(deviceParams.sample_rate);
  SetSampleFormat(alsaParams.sample_format);
  SetNumberOfChannels(deviceParams.n_channels);
  SetBufferSize(alsaParams.buffer_size);
  SetPeriodSize(alsaParams.period_size);

  //UpdateAudioParameters();
  if ((error = snd_pcm_hw_params (_pDevice, _alsaParams.hw_params)) < 0) {
      fprintf (stderr, "microphone: Hardware parameters cannot be configured (%s)\n",
           snd_strerror (error));
      exit (1);
  }
#endif
  //IOSelect();
  int count = snd_pcm_poll_descriptors_count (pDevice);
  if (count <= 0) {
    fprintf(stderr,"microphone: Invalid poll descriptors count\n");
    return count;
  }
  pUFDS = (struct pollfd *) malloc(sizeof(struct pollfd) * count);
  if ((error = snd_pcm_poll_descriptors(pDevice, pUFDS, count)) < 0) {
    fprintf(stderr,"microphone: Unable to obtain poll descriptors for capture: %s\n", snd_strerror(error));
    return error;
  }
  INFO("audio fd = %d", pUFDS->fd);
  fflush(stderr);

  return true;
}

AlsaAudioCapDevice::AlsaAudioCapDevice(string devName, alsa_parameters alsaParams, device_parameters deviceParams,
    snd_pcm_t* pDevice, struct pollfd* pUFDS)
  : BaseAudioCaptureDevice (HT_MIC, pUFDS->fd)
    ,_pUFDS(pUFDS)
    ,_pDevice(pDevice)
    ,_alsaParams(alsaParams)
    ,_deviceName(devName)
    ,_isStart(false)
{
  _deviceParams = deviceParams;
}


AlsaAudioCapDevice::~AlsaAudioCapDevice()
{
  this->Close();
}

bool AlsaAudioCapDevice::InitializeDevice() {

  int error;

  DEBUG ("InitializeDevice");
  SetSampleRate(_deviceParams.sample_rate);
  SetSampleFormat(_alsaParams.sample_format);
  SetNumberOfChannels(_deviceParams.n_channels);
  SetBufferSize(_alsaParams.buffer_size);
  SetPeriodSize(_alsaParams.period_size);

  //UpdateAudioParameters();
  if ((error = snd_pcm_hw_params (_pDevice, _alsaParams.hw_params)) < 0) {
      fprintf (stderr, "microphone: Hardware parameters cannot be configured (%s)\n",
           snd_strerror (error));
      exit (1);
  }

  return true;
}
bool AlsaAudioCapDevice::OnReadEvent() {
  OnAudioDataReady();
  return true;
}

int32_t AlsaAudioCapDevice::GetAudioFD()
{
  return _pUFDS->fd;
}

int32_t AlsaAudioCapDevice::GetSampleRate()
{
  return _deviceParams.sample_rate;
}

int32_t AlsaAudioCapDevice::GetBitsPerSample()
{
  return _deviceParams.bitwidth;
}

int32_t AlsaAudioCapDevice::GetNumberOfChannels()
{
  return _deviceParams.n_channels;
}

bool AlsaAudioCapDevice::SetSampleRate(uint32_t samplerate)
{

  int32_t error;

  //assigning the hw configuration structure to the device
  if ((error = snd_pcm_hw_params_any (_pDevice, _alsaParams.hw_params)) < 0) {
    fprintf (stderr, "microphone: Hardware configuration structure cannot be assigned to device (%s)\n", snd_strerror (error));
    return false;
  }

  //sets the configuration method
//  fprintf (stderr, "microphone: Access method: %d\n", _ioMethod.access);
  if ((error = snd_pcm_hw_params_set_access (_pDevice, _alsaParams.hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "microphone: Access method cannot be configured (%s)\n", snd_strerror (error));
    return false;
  }

  //sets the sample rate
  if ((error = snd_pcm_hw_params_set_rate (_pDevice, _alsaParams.hw_params, samplerate, 0)) < 0) {
    fprintf (stderr, "microphone: Sample rate cannot be configured (%s), Unsupported Sample Rate\n", snd_strerror (error));
    return false;
  }
  //checks sample rate
  if ((error = snd_pcm_hw_params_get_rate (_alsaParams.hw_params, &_deviceParams.sample_rate, 0)) < 0) {
    fprintf (stderr, "microphone: Sample rate cannot be obtained (%s)\n", snd_strerror (error));
    return false;
  }

  fprintf (stderr, "microphone: Sample_rate_real = %d\n", _deviceParams.sample_rate);
  return true;
}


void AlsaAudioCapDevice::SetSampleFormat(snd_pcm_format_t format)
{

    int32_t error;

    //SND_PCM_FORMAT_S16_LE => 16 bit signed little endian
    if ((error = snd_pcm_hw_params_set_format (_pDevice, _alsaParams.hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
    {
        fprintf (stderr, "microphone: Capture format cannot be configured (%s)\n",
             snd_strerror (error));
        exit (1);
    }
    //checks capture format
    if ((error = snd_pcm_hw_params_get_format (_alsaParams.hw_params, &_alsaParams.sample_format)) < 0)
    {
        fprintf (stderr, "microphone: Capture sample format cannot be obtained (%s)\n",
             snd_strerror (error));
        exit (1);
    }
    //just shows the capture format in a human readable way
    switch(_alsaParams.sample_format)
    {
    case SND_PCM_FORMAT_S16_LE:
        fprintf (stderr, "microphone: PCM capture sample format: SND_PCM_FORMAT_S16_LE \n");
        break;
    default:
        fprintf (stderr, "microphone: PCM capture sample format = %d\n", _alsaParams.sample_format);
    }


}
void AlsaAudioCapDevice::SetBufferSize(snd_pcm_uframes_t buffer_size)
{
  int32_t error;

    //sets the buffer size
    if ( (error = snd_pcm_hw_params_set_buffer_size(_pDevice, _alsaParams.hw_params, buffer_size)) < 0) {
        fprintf (stderr, "microphone: Buffer size cannot be configured (%s)\n",
             snd_strerror (error));
        exit (1);
    }
    //checks the value of the buffer size
    if ( (error = snd_pcm_hw_params_get_buffer_size(_alsaParams.hw_params, &_alsaParams.buffer_size)) < 0) {
    fprintf (stderr, "microphone: Buffer size cannot be obtained (%s)\n",
             snd_strerror (error));
        exit (1);
    }
    fprintf (stderr, "microphone: Buffer size = %d [frames]\n", (int)_alsaParams.buffer_size);


}
void AlsaAudioCapDevice::SetNumberOfChannels(uint32_t n_channels)
{
  int32_t error;
/**************************** configures the number of channels ********************************/
    //sets the number of channels
    if ((error = snd_pcm_hw_params_set_channels (_pDevice, _alsaParams.hw_params, n_channels)) < 0) {
        fprintf (stderr, "microphone: Number of channels cannot be configured (%s)\n",
             snd_strerror (error));
        exit (1);
    }
    //checks the number of channels
    if ((error = snd_pcm_hw_params_get_channels (_alsaParams.hw_params, &_deviceParams.n_channels)) < 0) {
        fprintf (stderr, "microphone: Number of channels cannot be obtained (%s)\n",
             snd_strerror (error));
        exit (1);
    }
    fprintf (stderr, "microphone: real_n_channels = %d\n", _deviceParams.n_channels);
}

int32_t AlsaAudioCapDevice::GetVolume()
{
    const char* mix_name = "Mic";
    const int mix_index = 0;
    snd_mixer_t* handle=NULL;
    snd_mixer_elem_t* elem=NULL;
    snd_mixer_selem_id_t* sid=NULL;
    uint32_t ret;

    snd_mixer_selem_id_alloca(&sid);

  if (sid == NULL) {
    DEBUG ("sid is NULL\n");
    return -1;
  }
    //sets simple-mixer index and name
    snd_mixer_selem_id_set_index(sid, mix_index);
    snd_mixer_selem_id_set_name(sid, mix_name);

    if ((snd_mixer_open(&handle, 0)) < 0) {
    DEBUG ("snd_mixer_open fail\n");
    //snd_mixer_selem_id_free(sid);
        return -1;
  }
    if ((snd_mixer_attach(handle, STR("hw:2"))) < 0) {
    DEBUG ("snd_mixer_attach fail\n");
    //snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return -1;
    }
    if ((snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
    DEBUG ("snd_mixer_selem_register fail\n");
    //snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return -1;
    }
    ret = snd_mixer_load(handle);
    if (ret < 0) {
    DEBUG ("snd_mixer_load fail\n");
    //snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return -1;
    }
    elem = snd_mixer_find_selem(handle, sid);
    if (!elem) {
    DEBUG ("snd_mixer_find_selem fail\n");
    //snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return -1;
  }

    long curvol=0;
  long minv, maxv;

    ret = snd_mixer_selem_get_capture_volume (elem, SND_MIXER_SCHN_FRONT_LEFT, &curvol);
  if (ret < 0) {
    DEBUG("snd_mixer_selem_get_capture_volume fail\n");
    //snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return -1;
  }

    ret = snd_mixer_selem_get_capture_volume_range (elem, &minv, &maxv);
  if (ret < 0) {
    DEBUG("snd_mixer_selem_get_capture_volume_range fail\n");
    //snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return -1;
  }

  curvol = ((curvol - minv)/(maxv - minv))*100;
  // DEBUG("Get volume %ld with status %d\n", curvol, ret);

  //snd_mixer_selem_id_free(sid);
  snd_mixer_close(handle);
  return curvol;

}

void AlsaAudioCapDevice::SetVolume(uint32_t vol)
{
  const char* mix_name = "Mic";
  const int mix_index = 0;
  int ret;
  snd_mixer_t* handle=NULL;
  snd_mixer_elem_t* elem=NULL;
  snd_mixer_selem_id_t* sid=NULL;

  long longvol;

  snd_mixer_selem_id_alloca(&sid);

  if (sid == NULL) {
    DEBUG ("sid is NULL\n");
    return;
  }
    //sets simple-mixer index and name
    snd_mixer_selem_id_set_index(sid, mix_index);
    snd_mixer_selem_id_set_name(sid, mix_name);

    if ((snd_mixer_open(&handle, 0)) < 0) {
    DEBUG ("snd_mixer_open fail\n");
    //snd_mixer_selem_id_free(sid);
        return;
  }

    if ((snd_mixer_attach(handle, STR(_deviceName.substr(0, 4)))) < 0) {
    DEBUG ("snd_mixer_attach fail\n");
    //snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return;
    }
    if ((snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
    DEBUG ("snd_mixer_selem_register fail\n");
    //snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return;
    }
    ret = snd_mixer_load(handle);
    if (ret < 0) {
    DEBUG ("snd_mixer_load fail\n");
    //snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return;
    }
    elem = snd_mixer_find_selem(handle, sid);
    if (!elem) {
    DEBUG ("snd_mixer_find_selem fail\n");
    //snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return;
  }

    long minv=0, maxv=0;

    ret = snd_mixer_selem_get_capture_volume_range (elem, &minv, &maxv);
  if (ret < 0) {
    DEBUG("snd_mixer_selem_get_capture_volume_range fail\n");
    //snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return;
  }

    //DEBUG("Volume range <%ld,%ld>, ret:%d\n", minv, maxv, ret);

  if(vol < 0 || vol > 100) // out of bounds
      return;

    longvol = (vol * (maxv - minv) / 100) + minv;

    ret = snd_mixer_selem_set_capture_volume_all (elem, longvol);
  if (ret < 0) {
    DEBUG("snd_mixer_selem_set_capture_volume_all fail\n");
    //snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return;
    }

    DEBUG("Set volume %ld with status %d\n", longvol, ret);

  //snd_mixer_selem_id_free(sid);
  snd_mixer_close(handle);

}

void AlsaAudioCapDevice::SetPeriodSize(snd_pcm_uframes_t period_size)
{
  int32_t error, dir;
  dir=0; //dir=0  =>  period size must be equal to period_size
  //sets the period size
  if ( (error = snd_pcm_hw_params_set_period_size(_pDevice, _alsaParams.hw_params, period_size, dir)) < 0) {
      fprintf (stderr, "microphone: Period size cannot be configured (%s)\n",
           snd_strerror (error));
      exit (1);
  }
  //checks the value of period size
  if ( (error = snd_pcm_hw_params_get_period_size(_alsaParams.hw_params, &_alsaParams.period_size, &dir)) < 0) {
  fprintf (stderr, "microphone: Period size cannot be obtained (%s)\n",
           snd_strerror (error));
      exit (1);
  }
  fprintf (stderr, "microphone: Period size = %d [frames]\n", (int)_alsaParams.period_size);
}

#if 0
void AlsaAudioCapDevice::UpdateAudioParameters()
{
  int32_t error;

  if ((error = snd_pcm_hw_params (_pDevice, _alsaParams.hw_params)) < 0) {
    fprintf (stderr, "microphone: Hardware parameters cannot be configured (%s)\n",
             snd_strerror (error));
    exit (1);
  }
}
#endif


bool AlsaAudioCapDevice::Close()
{
  if (_pDevice) {
    //snd_pcm_close(_pDevice);
  }
  return true;
}

void AlsaAudioCapDevice::ProcessAudioData(uint8_t *buf, uint32_t size)
{
  string rmsValue= numToString(DetectSoundEvent(buf, size), 2);
  SendDataToCarriers(buf, size, 0, HW_DATA_AUDIO);
  if (IsRMSEnabled()) {
    NotifyObservers(ACM_RMSVALUE, rmsValue);
  }
}


//Recovery callback in case of error
int32_t AlsaAudioCapDevice::Recovery(snd_pcm_t *handle, int32_t error)
{
  switch(error)
  {
    case -EPIPE:    // Buffer Over-run
      fprintf(stderr,"microphone: \"Buffer Overrun\" \n");
      if ( (error = snd_pcm_prepare(handle)) < 0)
        fprintf(stderr,"microphone: Buffer overrrun cannot be recovered, snd_pcm_prepare fail: %s\n", snd_strerror(error));
      return 0;
      break;
    case -ESTRPIPE: //suspend event occurred
      fprintf(stderr,"microphone: Error ESTRPIPE\n");
      //EAGAIN means that the request cannot be processed immediately
      while ( (error = snd_pcm_resume(handle)) == -EAGAIN)
        sleep(1);// wait until the suspend flag is clear

      if (error < 0) // error case
      {
        if ( (error = snd_pcm_prepare(handle)) < 0)
          fprintf(stderr,"microphone: Suspend cannot be recovered, snd_pcm_prepare fail: %s\n", snd_strerror(error));
      }
      return 0;
      break;
    case -EBADFD://Error PCM descriptor is wrong
      fprintf(stderr,"microphone: Error EBADFD\n");
      break;
    default:
      fprintf(stderr,"microphone: Error unknown, error = %d\n",error);
      break;
  }
  return error;
}

#if 0
int32_t AlsaAudioCapDevice::IOSelect()
{
  int count;
  int error;
  //struct pollfd *ufds;//file descriptor array used by pool
  count = snd_pcm_poll_descriptors_count (_pDevice);
  if (count <= 0) {
    fprintf(stderr,"microphone: Invalid poll descriptors count\n");
    return count;
  }
  _pUFDS = (struct pollfd *) malloc(sizeof(struct pollfd) * count);
  if ((error = snd_pcm_poll_descriptors(_pDevice, _pUFDS, count)) < 0) {
    fprintf(stderr,"microphone: Unable to obtain poll descriptors for capture: %s\n", snd_strerror(error));
    return error;
  }
  INFO("audio fd = %d", _pUFDS->fd);
  //OnAudioDataReady();
  return 0;
}
#endif

bool AlsaAudioCapDevice::IsStart() {
  return _isStart;
}

void AlsaAudioCapDevice::Start() {
  ENABLE_DEVICE_READ_DATA;
  if (OnAudioDataReady()) {
    NotifyObservers(ACM_STARTED, "ALSA Audio Capture Started");
    _isStart=true;
  }
}

void AlsaAudioCapDevice::Stop() {
  DISABLE_DEVICE_READ_DATA;
  NotifyObservers(ACM_STOPPED, "ALSA Audio Capture Stopped");
  _isStart=false;
}

uint32_t AlsaAudioCapDevice::GetRMSWindowSize() {
  snd_pcm_sframes_t period_size = _alsaParams.period_size;
  return (sizeof(uint32_t)*period_size);
}

bool AlsaAudioCapDevice::OnAudioDataReady()
{

  snd_pcm_sframes_t period_size = _alsaParams.period_size;
  int n_channels = _deviceParams.n_channels;
  uint8_t *ptr;//pointer in buffer
  int cptr;//captured frames counter
  uint8_t buf[2*n_channels*period_size];//audio frames buffer
  int error;

  ptr = buf;
  cptr = period_size;
  while(cptr > 0) //waits until buff is filled with period_size frames
  {
    if ((error = snd_pcm_readi (_pDevice, buf, period_size)) < 0)
    {
      if (Recovery(_pDevice, error)) {
        fprintf(stderr,"microphone: Write error: %s\n", snd_strerror(error));
        exit(EXIT_FAILURE);
      }
      continue;
    }
    ptr += error * n_channels;
    cptr -= error;
    ProcessAudioData(buf, sizeof(uint32_t)*period_size);
    if (cptr == 0)//exits if the read of the period is done
      break;
    else
      WARN("Audio Error");
  }
  return true;
}

void AlsaAudioCapDevice::CheckStatus(double ts) {
  NYI;
}
