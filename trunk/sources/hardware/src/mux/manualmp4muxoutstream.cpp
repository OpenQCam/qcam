#include "mux/manualmp4muxoutstream.h"
#include "system/systemmanager.h"
#include "system/systemdefine.h"
#include "system/nvramdefine.h"
#include "mediaformats/mp4/writemp4document.h"

ManualMP4MuxOutStream::ManualMP4MuxOutStream(StreamsManager *pStreamsManager, string name)
  : BaseMP4MuxOutStream(pStreamsManager, name) {
  _minReqSize=stringToNum<uint64_t>(SystemManager::GetNVRam(NVRAM_RECORD_MINSIZE));
}

ManualMP4MuxOutStream::~ManualMP4MuxOutStream() {
}

bool ManualMP4MuxOutStream::StopRecord() {
  bool ret=true;

  UpdateInfo(time(NULL));
  if (_mp4file) {
    _mp4file->Close();
    delete _mp4file;
    _mp4file=NULL;
  }
  else {
    ret=false;
  }
  _muxState=MUX_STOP_RECORD;
  return ret;
}

bool ManualMP4MuxOutStream::SignalStop() {
  NYI;
  return true;
}

bool ManualMP4MuxOutStream::SignalPlay(double &absTS, double &length) {
  NYI;
  return true;
}

bool ManualMP4MuxOutStream::SignalAudioPlay() {
  NYI;
  return true;
}

bool ManualMP4MuxOutStream::SignalAudioStop() {
  NYI;
  return true;
}

bool ManualMP4MuxOutStream::FeedData(uint8_t *pData,  uint32_t dataLength,
                                     uint32_t processedLength, uint32_t totalLength,
                                     double absoluteTimestamp, bool isAudio) {
  //NYI;
  return true;
}
