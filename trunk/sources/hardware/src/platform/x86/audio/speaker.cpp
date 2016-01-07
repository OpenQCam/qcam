#include "audio/speaker.h"
//thelib
#include "protocols/avhttpstream/avstreamdefine.h"
#include "streaming/audiofilestream.h"
//hardware
#include "audio/pcmplaystream.h"

#define SAMPLE_RATE 44100

BaseHardwareAbstractDevice *Speaker::_pDeviceInstance = NULL;

//static member function
BaseHardwareAbstractDevice* Speaker::GetInstance(string devNode) {
  if(_pDeviceInstance == NULL){
    int32_t fd;
    if((fd = open(STR(devNode), O_RDWR|O_SYNC, 0)) < 0) {
      FATAL("Cannot open device %s", STR(devNode));
      return NULL;
    }
    _pDeviceInstance = (BaseHardwareAbstractDevice *) new Speaker(fd);
  }
  return _pDeviceInstance;
}


Speaker::Speaker(int32_t fd)
: BaseAudioDevice (HT_SPEAKER, fd),
  _pPlayStream(NULL),
  _pFileStream(NULL),
  _skipFrame(5),
  _dummyFrame(MAX_I2S_PAGE),
  _state(ST_SPK_CLOSED),
  _count(0),
  _bReadyToStop(false)
{
  _deviceParams.sample_rate=SAMPLE_RATE;
  memset(_txbuffer, 0, sizeof(_txbuffer));
}

Speaker::~Speaker()
{
  Close();
}

bool Speaker::InitializeDevice() {
  DEBUG ("Speaker initialize device, config sample rate and volume");
  if (_deviceFD) {
    //ioctl (_deviceFD, I2S_SRATE, _deviceParams.sample_rate);
    //ioctl (_deviceFD, I2S_TX_VOL, _volume);
  }
  else {
    FATAL ("Init speaker with null fd");
  }

  system("icam_mute");
  return true;
}

bool Speaker::Open() {

  if (_state!=ST_SPK_CLOSED && _state!=ST_SPK_WAIT_CLOSED) {
    DEBUG ("Speaker already opened");
    return false;
  }
  if (!_pDeviceInstance->InitializeDevice()) {
    return false;
  }
  _state=ST_SPK_STOPPED;
  _skipFrame=MAX_I2S_PAGE*2;
  //ioctl (_deviceFD, I2S_TX_ENABLE, 0);
  return true;
}

bool Speaker::Close() {
  system("icam_mute");
  Stop();
  _state=ST_SPK_CLOSED;
  NotifyObservers(APM_CLOSED, "");
  return true;
}

void Speaker::Start()
{
  if (_state == ST_SPK_STOPPED) {
    //ioctl (_deviceFD, I2S_TX_ENABLE, 0);
    ENABLE_DEVICE_WRITE_DATA;
    NotifyObservers(APM_STARTED, "Start i2s audio play");
    _state = ST_SPK_START_SKIP;
  }
  else {
    DEBUG ("state != stopped");
  }
}

void Speaker::Stop()
{
  if (_state == ST_SPK_START_SKIP ||
      _state == ST_SPK_START_NORMAL ||
      _state == ST_SPK_WAIT_CLOSED) {
    //no tx disable to avoid noise
    //ioctl (_deviceFD, I2S_TX_DISABLE, 0);
    DISABLE_DEVICE_WRITE_DATA;
    NotifyObservers(APM_STOPPED, "Stop i2s audio play");
    _state = ST_SPK_STOPPED;
    DEBUG ("=================================STOP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  }
}

bool Speaker::Play() {
  if (_state != ST_SPK_CLOSED) {
    DEBUG ("state != closed");
    return false;
  }
  Open();
  Start();
  return true;
}

void Speaker::SetVolume(uint32_t volume)
{
  return;
}

int32_t Speaker::GetVolume() {
  return 0;
}

uint32_t Speaker::GetAudioFrameSize() {
  return I2S_PAGE_SIZE;
}

bool Speaker::OnWriteEvent() {
  //uint32_t ret=0;

  _count++;
  if (_state==ST_SPK_START_NORMAL){
    if (_bReadyToStop) {
      _bReadyToStop=false;
      _state=ST_SPK_WAIT_CLOSED;
      DEBUG ("set to wait closed");
    }
    _pPlayStream->PullAudioData();
  }
  else if (_state==ST_SPK_START_SKIP) {
    //ioctl(_deviceFD, I2S_PUT_AUDIO, _txbuffer);
    _skipFrame--;
    if (_skipFrame<=0) {
      DEBUG ("start to play, unmute device");
      system("icam_unmute");
      _state = ST_SPK_START_NORMAL;
    }
  }
  else if (_state==ST_SPK_WAIT_CLOSED) {
    //ioctl(_deviceFD, I2S_PUT_AUDIO, _txbuffer);
    _dummyFrame--;
    DEBUG ("dummFrame:%d",_dummyFrame);
    if (_dummyFrame<=0) {
      DISABLE_DEVICE_WRITE_DATA;
      DEBUG ("dummy data over, change to closed state");
      Close();
    }
  }
  else {
    DEBUG ("unknown state");
  }
  return true;
}

bool Speaker::SignalStop() {
  _bReadyToStop=true;
  return true;
}

bool Speaker::IsStart() {
  return (_state!=ST_SPK_CLOSED);
}

int32_t Speaker::PutData(uint8_t *pData, uint32_t length) {
  //return ioctl(_deviceFD, I2S_PUT_AUDIO, pData);
  return 0;
}

void Speaker::CheckStatus(double ts) {
  NYI;
}

bool Speaker::CreateStream(StreamsManager* pStreamManager) {
  if (!_pPlayStream) {
    _pPlayStream = new PCMPlayStream(pStreamManager, QIC_STREAM_WAVEPLAY);
  }
  if (!_pFileStream) {
    _pFileStream = new AudioFileStream(pStreamManager, QIC_STREAM_WAVEFILE);
  }
  return true;
}

bool Speaker::LinkFileStream(string fullName) {
  if (_pPlayStream && _pFileStream) {
    if (!_pPlayStream->IsLinked()) {
      bool ret=true;
      ret &= _pFileStream->Link(reinterpret_cast<BaseOutStream*>(_pPlayStream), true);
      ret &= _pFileStream->Open(fullName);
      return ret;
    }
  }
  return false;
}

bool Speaker::LinkNetStream(BaseInStream *pStream) {
  NYI;
  return true;
}
