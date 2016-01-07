#include "mux/cvrmuxoutstream.h"
#include "system/systemmanager.h"
#include "system/systemdefine.h"

CVRMuxOutStream::CVRMuxOutStream(StreamsManager *pStreamsManager, string name)
  : BaseMP4MuxOutStream(pStreamsManager, name) {
}

CVRMuxOutStream::~CVRMuxOutStream() {
  _muxState=MUX_STOP_RECORD;
}

bool CVRMuxOutStream::StartRecord() {
  uint32_t samplerate=22050;
  uint8_t channel=0x1;
  uint8_t bytewidth=0x2;
  uint64_t audioConfig=((uint64_t)samplerate<<32)|(channel<<24)|(bytewidth<<16);
  uint32_t resolution= (_pVideoStream->GetCapabilities()->avc._height<<16)|
                       (_pVideoStream->GetCapabilities()->avc._width);

  if (IsRecording()) {
    WARN("mp4 mux stream already start");
    return false;
  }
  string path="/tmp/";
  if (!_pVideoStream)
    return false;
  UnixDomainSocketManager::SendDataToCVRThread(CVR_SPS, 0,
                                              _pVideoStream->GetCapabilities()->avc._spsLength,
                                              _pVideoStream->GetCapabilities()->avc._pSPS);
  UnixDomainSocketManager::SendDataToCVRThread(CVR_PPS, 0,
                                               _pVideoStream->GetCapabilities()->avc._ppsLength,
                                               _pVideoStream->GetCapabilities()->avc._pPPS);
  UnixDomainSocketManager::SendDataToCVRThread(CVR_AUDIO_CONFIG, 0,
                                               sizeof(audioConfig),(uint8_t*)&(audioConfig));
  UnixDomainSocketManager::SendDataToCVRThread(CVR_RESOLUTION, 0,
                                               sizeof(resolution), (uint8_t*)&(resolution));
  UnixDomainSocketManager::SendDataToCVRThread(START_CVR_RECORD, 0,
                                               path.length(), (uint8_t*)path.c_str());
  _muxState=MUX_START_RECORD;
  return true;
}

bool CVRMuxOutStream::StopRecord() {
  UnixDomainSocketManager::SendDataToCVRThread(STOP_CVR_RECORD, 0, 0, NULL);
  _muxState=MUX_STOP_RECORD;
  return true;
}

bool CVRMuxOutStream::FeedData (uint8_t *pData, uint32_t dataLength,
                                uint32_t processedLength,
                                uint32_t totalLength,
                                double absTimestamp,
                                bool isAudio) {

#if 0
  //Feed to uxdomainsocket for constant video recording
  UnixDomainSocketManager::SendDataToCVRThread(VIDEO_AVC_DATA|isAudio,
                                               absTimestamp, dataLength, pData);
#endif
  #if 0
  //failover recording
  BaseMP4MuxOutStream::FeedData(pData, dataLength, processedLength,
                                totalLength, absTimestamp, isAudio)
  #endif
  return true;
}

bool CVRMuxOutStream::SignalStop() {
  NYI;
  return true;
}

bool CVRMuxOutStream::SignalPlay(double &absTS, double &length) {
  NYI;
  return true;
}

bool CVRMuxOutStream::SignalAudioPlay() {
  NYI;
  return true;
}

bool CVRMuxOutStream::SignalAudioStop() {
  NYI;
  return true;
}
