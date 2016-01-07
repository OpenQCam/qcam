
#include "mux/basemuxstream.h"
#include "video/basevideostream.h"
#include "streaming/baseoutstream.h"
#include "protocols/baseprotocol.h"
#include "hardwaremanager.h"
#include "video/videodefine.h"
#include "video/baseavcvideocapture.h"
#include "audio/baseaudiodevice.h"

static RTMPFrameHeader RTMPInitHeader = {
  { 0x17, 0x0, 0x0, 0x0, 0x0, 0x1, 0x42, 0xE0, 0x1F, 0x3 },
  10,
};

static RTMPFrameHeader RTMPEOSHeader = {
 { 0x17, 0x2, 0x0, 0x0, 0x0 },
 5,
};

BaseMuxStream::BaseMuxStream(StreamsManager *pStreamsManager, uint64_t type,
                             string name)
: BaseVideoStream (pStreamsManager, type, name),
  _pAVCCaptureInstance(NULL),
  _isInitVideo(false),
  _isInitAudio(false),
  _requestKeyFrame(false),
  _currentTS(0),
  _lastFrameTS(0),
  _RTMPAudioHeader(0x32) {

  _pStreamCapabilities = new StreamCapabilities();
  //Video init
  _pAVCCaptureInstance = (BaseAVCVideoCapture *)HardwareManager::GetHardwareInstance(HT_VIDEO_AVC);

  BaseAudioDevice* pAudioDevice=reinterpret_cast<BaseAudioDevice*>
      (HardwareManager::GetHardwareInstance(HT_MIC));
  //Audio init
  if (pAudioDevice) {
    device_parameters deviceparams =
        pAudioDevice->GetDeviceParameters();
    _pStreamCapabilities->InitAudioPCM(deviceparams.sample_rate, deviceparams.n_channels);
    _isInitAudio = true;

    if (deviceparams.sample_rate==11025)
      _RTMPAudioHeader |= (1<<2);
    else if (deviceparams.sample_rate==22050)
      _RTMPAudioHeader |= (2<<2);
    else if (deviceparams.sample_rate==44100)
      _RTMPAudioHeader |= (3<<2);

    if (deviceparams.n_channels==2)
      _RTMPAudioHeader |= (1);
  }
}

BaseMuxStream::~BaseMuxStream() {
  if(_pStreamCapabilities)
    delete _pStreamCapabilities;
}

StreamCapabilities * BaseMuxStream::GetCapabilities() {
  return _pStreamCapabilities;
}

bool BaseMuxStream::IsCompatibleWithType(uint64_t type) {
  return TAG_KIND_OF(type, ST_OUT_NET_RTMP_4_RTMP)
      || TAG_KIND_OF(type, ST_OUT_FILE_RTMP)
      || TAG_KIND_OF(type, ST_OUT_NET_RTP)
      || TAG_KIND_OF(type, ST_OUT_NET_RAW)
      || TAG_KIND_OF(type, ST_OUT_WSNET_AVC)
      ;
}


uint32_t BaseMuxStream::FindNALU(uint8_t *pBuf, uint32_t length, uint8_t &naluType)
{
  uint8_t state=0;
  uint32_t offset = 0;

  static const uint8_t fsm[4][6] = {
    {1, 2, 3, 3, 0, 0},
    {0, 0, 0, 4, 0, 0},
    {0, 0, 0, 0, 5, 0},
    {0, 0, 0, 0, 0, 0}
  };

  if(length < 6) return 0;

  while(offset < length){
    switch(pBuf[offset++]){
    case 0:
      state = fsm[0][state];
      break;
    case 1:
      state = fsm[1][state];
      break;
    case 9:
      state = fsm[2][state];
      break;
    default:
      state = fsm[3][state];
      break;
    }
    switch(state){
      case 4: // found start code
        //type  = pData[offset] & 0x1F;
        naluType  = pBuf[offset] & 0x1F;
        return offset;
        break;
      default:
        continue;
        break;
    }
  }
  return 0;
}

uint8_t BaseMuxStream::ParseFrame(uint8_t *pBuf, uint32_t length, uint8_t naluNum)
{
  uint32_t naluPosition = 0;
  uint8_t naluType;
  if((naluPosition = FindNALU(pBuf, length, naluType)) != 0) {
    switch(naluType) {
      case NALU_TYPE_IDR: // I
      case NALU_TYPE_SLICE: // P
      case NALU_TYPE_SPS:
      case NALU_TYPE_PPS:
      case NALU_TYPE_SEI:
        _nalus[naluNum].type = naluType;
        _nalus[naluNum].data = pBuf + naluPosition;
        if(naluNum > 0) {
          _nalus[naluNum-1].length = naluPosition - 4;
          *((uint32_t*)(_nalus[naluNum-1].data - 4)) = EHTONL(_nalus[naluNum-1].length);
        }
        naluNum = ParseFrame(pBuf + naluPosition, length - naluPosition, naluNum+1);
        break;
      default:
        // next nalu if type is what we want
        naluNum = ParseFrame(pBuf + naluPosition, length - naluPosition, naluNum);
        break;
    }
  } else {
    _nalus[naluNum-1].length = length;
    *((uint32_t*)(_nalus[naluNum-1].data - 4)) = EHTONL(_nalus[naluNum-1].length);
  }
  return naluNum;
}

#ifndef TS_SCALER
#define TS_SCALER 1
#endif

void BaseMuxStream::OnDeviceData(DeviceData &data) {
  if (GetStatus() == VSS_STOP) return;

  if (data.datatype==HW_DATA_AUDIO) {
    OnDeviceAudioData(data);
  }
  else if (data.datatype==HW_DATA_VIDEO) {
    GETTIMESTAMP(_currentTS);
    //1. Sync audio video timestamp
    //double correction=0.99;
    ////INFO("ts %"PRIu64, (uint64_t)data.ts);
    //if (data.ts){
    //  //double correction=0.999880;
    //  if(_lastFrameTS == 0){
    //    _lastFrameTS = data.ts;
    //  }
    //  //INFO("last %"PRIu64, (uint64_t)_lastFrameTS);
    //  if(_lastFrameTS > data.ts) {
    //    WARN("error last ts > current ts %"PRIu64, (uint64_t)(_lastFrameTS - data.ts));
    //  }else{
    //    _currentTS += (double)(data.ts - _lastFrameTS)*correction/TS_SCALER;
    //  }
    //  _lastFrameTS = data.ts;
    //  //INFO("current %"PRIu64, (uint64_t)_currentTS);
    //} else{ // data.ts == 0, resolution changed
    //  //GETTIMESTAMP(_currentTS);
    //  _lastFrameTS = 0;
    //}
    //2. Send video data
    OnDeviceVideoData(data);
  }
}

void BaseMuxStream::ResendSPSPPS() {
  LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;
  while (pTemp != NULL){
    if (pTemp->info->GetType() == ST_OUT_NET_RTP){
      if(!pTemp->info->FeedData(_pStreamCapabilities->avc._pSPS, _pStreamCapabilities->avc._spsLength, 0, _pStreamCapabilities->avc._spsLength, _currentTS, false)){
        FATAL("Unable to feed OS: %p", pTemp->info);
      }
      if(!pTemp->info->FeedData(_pStreamCapabilities->avc._pPPS, _pStreamCapabilities->avc._ppsLength, 0, _pStreamCapabilities->avc._ppsLength, _currentTS, false)){
        FATAL("Unable to feed OS: %p", pTemp->info);
      }
    }
    else if(pTemp->info->GetType() == ST_OUT_NET_RTMP_4_RTMP){
      _dataBuffer.ReadFromBuffer(RTMPEOSHeader.header, RTMPEOSHeader.length);
      if(!pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false)){
        FATAL("Unable to feed OS: %p", pTemp->info);
      }
      _dataBuffer.IgnoreAll();

      _dataBuffer.ReadFromBuffer(RTMPInitHeader.header, RTMPInitHeader.length);
      _dataBuffer.ReadFromByte(0x1);
      _dataBuffer.ReadFromByte(uint8_t ((_pStreamCapabilities->avc._spsLength>>8) & 0xFF));
      _dataBuffer.ReadFromByte(uint8_t (_pStreamCapabilities->avc._spsLength & 0xFF));
      _dataBuffer.ReadFromBuffer(_pStreamCapabilities->avc._pSPS, _pStreamCapabilities->avc._spsLength);
      _dataBuffer.ReadFromByte(0x1);
      _dataBuffer.ReadFromByte(uint8_t ((_pStreamCapabilities->avc._ppsLength>>8) & 0xFF));
      _dataBuffer.ReadFromByte(uint8_t (_pStreamCapabilities->avc._ppsLength & 0xFF));
      _dataBuffer.ReadFromBuffer(_pStreamCapabilities->avc._pPPS, _pStreamCapabilities->avc._ppsLength);

      if(!pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false)){
        FATAL("Unable to feed OS: %p", pTemp->info);
      }
      _dataBuffer.IgnoreAll();
    }
    else {
      WARN("Cannot handle this stream type - %s", STR(tagToString(pTemp->info->GetType())));
    }
    pTemp = pTemp->pPrev;
  }
}

void BaseMuxStream::SignalOutStreamAttached(BaseOutStream *pOutStream)
{
  RequestKeyFrame();
  if (pOutStream->GetType() == ST_OUT_NET_RTP) {
    if(!pOutStream->FeedData(_pStreamCapabilities->avc._pSPS, _pStreamCapabilities->avc._spsLength, 0, _pStreamCapabilities->avc._spsLength, _currentTS, false)){
      FATAL("Unable to feed OS: %p", pOutStream);
    }
    if(!pOutStream->FeedData(_pStreamCapabilities->avc._pPPS, _pStreamCapabilities->avc._ppsLength, 0, _pStreamCapabilities->avc._ppsLength, _currentTS, false)){
      FATAL("Unable to feed OS: %p", pOutStream);
    }
  } else if(pOutStream->GetType() == ST_OUT_NET_RTMP_4_RTMP){
    _dataBuffer.ReadFromBuffer(RTMPInitHeader.header, RTMPInitHeader.length);
    _dataBuffer.ReadFromByte(0x1);
    _dataBuffer.ReadFromByte(uint8_t ((_pStreamCapabilities->avc._spsLength>>8) & 0xFF));
    _dataBuffer.ReadFromByte(uint8_t (_pStreamCapabilities->avc._spsLength & 0xFF));
    _dataBuffer.ReadFromBuffer(_pStreamCapabilities->avc._pSPS, _pStreamCapabilities->avc._spsLength);
    _dataBuffer.ReadFromByte(0x1);
    _dataBuffer.ReadFromByte(uint8_t ((_pStreamCapabilities->avc._ppsLength>>8) & 0xFF));
    _dataBuffer.ReadFromByte(uint8_t (_pStreamCapabilities->avc._ppsLength & 0xFF));
    _dataBuffer.ReadFromBuffer(_pStreamCapabilities->avc._pPPS, _pStreamCapabilities->avc._ppsLength);

    if(!pOutStream->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false)){
      FATAL("Unable to feed OS: %p", pOutStream);
    }
    _dataBuffer.IgnoreAll();
  }
  else{
    DEBUG ("outstream attached:%s", STR(tagToString(pOutStream->GetType())));
  }
}


void BaseMuxStream::SignalOutStreamDetached(BaseOutStream *pOutStream)
{
  FATAL ("outstream detached:%s", STR(tagToString(pOutStream->GetType())));
}

void BaseMuxStream::RequestKeyFrame()
{
  //FIXME(Tim): RequestKeyFrame should rely on handwaremanager to get related instance
  if(_pAVCCaptureInstance->IsStarted()){
    _requestKeyFrame = true;
    _pAVCCaptureInstance->GetKeyFrame();
  }
}

void BaseMuxStream::UnlinkLiveStream() {
  vector<BaseOutStream *> outStreams = GetOutStreams();
  FOR_VECTOR(outStreams, i) {
    outStreams[i]->UnLink();
    BaseProtocol *pProtocol = outStreams[i]->GetProtocol();
    if(pProtocol != NULL){
      pProtocol->EnqueueForDelete();
    }
  }
}

void BaseMuxStream::OnDeviceMessage (DeviceMessage &msg)
{
  switch(msg.type){
    case VCM_STARTED:
      _isInitVideo = false;
      RequestKeyFrame();
      Play(0, 0);
      break;
    case VCM_STOPPED:
      Stop();
      break;
    case VCM_RESOLUTION_CHANGED:
      UnlinkLiveStream();
      _isInitVideo = false;
      RequestKeyFrame();
      break;
    case VCM_FPS_CHANGED:
      _isInitVideo = false;
      RequestKeyFrame();
      break;
    case VCM_PROFILE_CHANGED:
      _isInitVideo = false;
      RequestKeyFrame();
      break;
    default:
    break;
  }
}
