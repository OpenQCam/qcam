#if 0
#include "audio/speaker.h"
//thelib
#include "protocols/avhttpstream/avstreamdefine.h"
#include "streaming/audiofilestream.h"
#include "netio/select/uxdomainmanager.h"
#include "system/systemmanager.h"
//hardware
#include "audio/pcmplaystream.h"


#define SAMPLE_RATE 22050

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
  memset(_dummy, 0, sizeof(_dummy));
}

Speaker::~Speaker()
{
  Close();
}

bool Speaker::InitializeDevice() {
  DEBUG ("Speaker initialize device, config sample rate and volume");
  if (_deviceFD) {
    ioctl (_deviceFD, I2S_SRATE, _deviceParams.sample_rate);
    ioctl (_deviceFD, I2S_TX_VOL, _volume);
  }
  else {
    FATAL ("Init speaker with null fd");
  }
  Open();
  return true;
}

bool Speaker::Open() {

  if (_state!=ST_SPK_CLOSED) {
    DEBUG ("Speaker already opened");
    return false;
  }

  ioctl (_deviceFD, I2S_TX_ENABLE, 0);
  _state=ST_SPK_STOPPED;
  DISABLE_DEVICE_WRITE_DATA;
  return true;
}

bool Speaker::Close() {

  system("icam_mute");
  if (_state!=ST_SPK_CLOSED) {
    Stop();
    ioctl (_deviceFD, I2S_TX_DISABLE, 0);
    _state=ST_SPK_CLOSED;
    NotifyObservers(APM_CLOSED, "");
  }
}

void Speaker::Start()
{
  if (_state == ST_SPK_STOPPED) {
    ENABLE_DEVICE_WRITE_DATA;
    _dummyFrame=MAX_I2S_PAGE;
    _skipFrame=MAX_I2S_PAGE*2;
    NotifyObservers(APM_STARTED, "Start i2s audio play");
    _state = ST_SPK_START_SKIP;
  }
  else {
    DEBUG ("state != stopped:%d", _state);
  }
}

void Speaker::Stop()
{
  if (_state == ST_SPK_START_SKIP ||
      _state == ST_SPK_START_NORMAL ||
      _state == ST_SPK_WAIT_STOPPED) {
    DISABLE_DEVICE_WRITE_DATA;
    NotifyObservers(APM_STOPPED, "Stop i2s audio play");
    _state = ST_SPK_STOPPED;
    system("icam_mute");
  }
}

bool Speaker::Play() {
  if (_state != ST_SPK_STOPPED) {
    DEBUG ("state != stopped");
    return false;
  }
  Start();
  return true;
}

void Speaker::SetVolume(uint32_t volume)
{
  string cmd = "icam_set_speaker_volume ";
  char param[10];
  sprintf(param,"%d", volume);
  cmd = cmd + " " + param;
  DEBUG("command: %s", cmd.c_str());
  UnixDomainSocketManager::ExecuteSystemCommand(cmd);
  return;
}

int32_t Speaker::GetVolume() {
  string output;
  Variant result;
  uint32_t start = 0;

  SystemManager::DoSystemCommand("icam_get_speaker_volume", output);
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("volume")){
    int8_t volume = (int8_t)atoi(STR(result["volume"]));
    DEBUG( "Speaker volume: %d", volume);
    return volume;
  }
  else{
    DEBUG( "Error! Can't get speaker volume");
    return -1;
  }
}

uint32_t Speaker::GetAudioFrameSize() {
  return I2S_PAGE_SIZE;
}

bool Speaker::OnWriteEvent() {
  bool ret=false;

  DEBUG ("Speaker::OnWriteEvent(state=%d)", _state);
  _count++;
  if (_state==ST_SPK_START_NORMAL){
    if (_bReadyToStop) {
      _bReadyToStop=false;
      _state=ST_SPK_WAIT_STOPPED;
      DEBUG ("set to wait stopped");
      DEBUG ("play:%p, file:%p", _pPlayStream, _pFileStream);
    }
    if (!_pPlayStream->PullAudioData()) {
      ioctl(_deviceFD, I2S_PUT_AUDIO, _dummy);
    }
  }
  else if (_state==ST_SPK_START_SKIP) {
    ioctl(_deviceFD, I2S_PUT_AUDIO, _txbuffer);
    _skipFrame--;
    if (_skipFrame<=0) {
      DEBUG ("start to play, unmute device");
      system("icam_unmute");
      _state = ST_SPK_START_NORMAL;
    }
  }
  else if (_state==ST_SPK_WAIT_STOPPED) {
    ioctl(_deviceFD, I2S_PUT_AUDIO, _txbuffer);
    _dummyFrame--;
    DEBUG ("dummFrame:%d",_dummyFrame);
    if (_dummyFrame<=0) {
      DEBUG ("dummy data over, change to stopped state");
      Stop();
    }
  }
  else {
    DEBUG ("unknown state");
    Stop();
  }
  return true;
}

bool Speaker::SignalStop() {
  _bReadyToStop=true;
}

bool Speaker::IsStart() {
  return (_state!=ST_SPK_CLOSED && _state!=ST_SPK_STOPPED);
}

int32_t Speaker::PutData(uint8_t *pData, uint32_t length) {
  return ioctl(_deviceFD, I2S_PUT_AUDIO, pData);
}

void Speaker::CheckStatus(double ts) {
//  NYI;
}

bool Speaker::CreateStream(StreamsManager* pStreamManager) {
  if (pStreamManager) {
    if (!_pPlayStream) {
      _pPlayStream = new PCMPlayStream(pStreamManager, QIC_STREAM_WAVEPLAY);
    }
    if (!_pFileStream) {
      _pFileStream = new AudioFileStream(pStreamManager, QIC_STREAM_WAVEFILE);
    }
  }
  else
    return false;
  return true;
}

bool Speaker::LinkFileStream(string fullName) {
  if (_pPlayStream && _pFileStream) {
    bool ret=true;
    if (_pPlayStream->IsLinked()) {
      ret &= _pPlayStream->UnLink(true);
    }
    ret &= _pFileStream->Link(reinterpret_cast<BaseOutStream*>(_pPlayStream), true);
    ret &= _pFileStream->Open(fullName);
    return ret;
  }
  return false;
}

bool Speaker::LinkNetStream(BaseInStream *pStream) {
  NYI;
  return true;
}
#endif
