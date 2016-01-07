
#include "mux/relaymuxstream.h"
#include "streaming/baseoutstream.h"
#include "protocols/baseprotocol.h"
#include "video/videodefine.h"


RelayMuxStream::RelayMuxStream(StreamsManager *pStreamsManager, string name)
: BaseMuxStream (pStreamsManager, ST_IN_AV_RELAYMUX, name),
  _pSPSPPS(NULL) {
}

RelayMuxStream::~RelayMuxStream() {
 delete _pSPSPPS;
 _pSPSPPS=NULL;
}

bool RelayMuxStream::IsCompatibleWithType(uint64_t type) {
  return TAG_KIND_OF(type, ST_OUT_WSNET_AVC);
}

void RelayMuxStream::OnDeviceAudioData(DeviceData& data) {
  LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;

  while (pTemp != NULL) {
    if (!pTemp->info->FeedData(data.buffer, data.length, 0, data.length, _currentTS, true)) {
      FATAL("Unable to feed OS: %p", pTemp->info);
    }
    pTemp = pTemp->pPrev;
  }
}

void RelayMuxStream::OnDeviceVideoData(DeviceData& data) {
  LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;

  uint8_t naluType = data.buffer[4] & 0x1F;
  if (naluType==7 && !_pSPSPPS) {
    DEBUG ("sps pps");
    _pSPSPPS = new uint8_t[data.length];
    memcpy(_pSPSPPS, data.buffer, data.length);
    _SPSPPSLen=data.length;
  }

  while (pTemp != NULL) {
    if (!pTemp->info->FeedData(data.buffer, data.length, 0, data.length, _currentTS, false)) {
      FATAL("Unable to feed OS: %p", pTemp->info);
    }
    pTemp = pTemp->pPrev;
  }
}

void RelayMuxStream::SignalOutStreamAttached(BaseOutStream *pOutStream) {
  DEBUG ("out stream attached");
  pOutStream->FeedData(_pSPSPPS, _SPSPPSLen, 0, _SPSPPSLen, 0, false);
  RequestKeyFrame();
}

void RelayMuxStream::OnDeviceMessage (DeviceMessage &msg) {
  BaseMuxStream::OnDeviceMessage(msg);

  switch(msg.type){
    case VCM_STOPPED:
      break;
    case VCM_RESOLUTION_CHANGED:
      UnlinkLiveStream();
      delete _pSPSPPS;
      _pSPSPPS=NULL;
      break;
  }
}
