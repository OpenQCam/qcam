
#ifdef HAS_PROTOCOL_HTTP
#include "streaming/wsoutnetavcstream.h"
#include "streaming/streamsmanager.h"
#include "streaming/streamstypes.h"
#include "streaming/baseinstream.h"
#include "system/eventdefine.h"
#include "protocols/http/websocket/basewssubprotocol.h"
#include "application/baseclientapplication.h"

#define MAXVIDEOBUFSIZE (256*1024)

WSOutNetAVCStream::WSOutNetAVCStream(BaseProtocol *pProtocol,
  StreamsManager *pStreamsManager,
  string streamName)
: BaseOutNetStream (pProtocol, pStreamsManager, ST_OUT_WSNET_AVC, streamName){
  _curDropped=0;
}

WSOutNetAVCStream::~WSOutNetAVCStream() {
}

bool WSOutNetAVCStream::SignalPlay (double &absTimeStamp, double &length) {
	return true;
}

bool WSOutNetAVCStream::SignalPause() {
	return true;
}

bool WSOutNetAVCStream::SignalResume() {
	return true;
}

bool WSOutNetAVCStream::SignalSeek(double &absoluteTimestamp) {
	return true;
}

bool WSOutNetAVCStream::SignalStop() {
  return true;
}

bool WSOutNetAVCStream::IsCompatibleWithType(uint64_t type) {
  return true;
}

void WSOutNetAVCStream::SignalStreamCompleted() {
}

void WSOutNetAVCStream::SignalAttachedToInStream() {
  if (_pStreamsManager) {
    const_cast<BaseClientApplication*>
        (_pStreamsManager->GetApplication())->OnNotifyEvent(CLOUD_MSG_EVENT,
                                                            EVT_OUT_AVC_CONNECTED,
                                                            0);
  }
}

void WSOutNetAVCStream::SignalDetachedFromInStream() {
  if (_pStreamsManager) {
    const_cast<BaseClientApplication*>
        (_pStreamsManager->GetApplication())->OnNotifyEvent(CLOUD_MSG_EVENT,
                                                            EVT_OUT_AVC_DISCONNECTED,
                                                            0);
  }
}

bool WSOutNetAVCStream::Link(BaseInStream *pInStream, bool reverseLink) {
  if (!IsLinked()) {
    return BaseOutStream::Link(pInStream, reverseLink);
  }
  else if (reverseLink){
    return pInStream->Link(this);
  }
  return true;
}

bool WSOutNetAVCStream::FeedData(uint8_t *pData, uint32_t dataLength,
		uint32_t processedLength, uint32_t totalLength,
		double absoluteTimestamp, bool isAudio) {

  uint32_t curBufSize=0;

  if (IsEnqueueForDelete())
    return true;

  if (isAudio) {
    return true;
  }

  IOBuffer *pOutputBuffer=_pProtocol->GetOutputBuffer();

  if (pOutputBuffer) {
    curBufSize=GETAVAILABLEBYTESCOUNT(*pOutputBuffer);
  }

  if (curBufSize< (MAXVIDEOBUFSIZE*1>>2)) { // 1/4
    _dropPeriod=0;
  }
  else if (curBufSize< (MAXVIDEOBUFSIZE*2>>2)) { // 2/4
    _dropPeriod=3;
  }
  else if (curBufSize< (MAXVIDEOBUFSIZE*3>>2)) { // 3/4
    _dropPeriod=6;
  }
  else if (curBufSize< (MAXVIDEOBUFSIZE)){ // 4/4
    _dropPeriod=10;
  }
  else {
    WARN ("websocket avc congestion, drop frame");
    return true;
  }

	if (_pProtocol) {
    if (_curDropped >= _dropPeriod) {
      _curDropped=0;
      return reinterpret_cast<BaseWSSubProtocol*>(_pProtocol)->EnqueueForWSOutbound(pData, dataLength, true, WS_OPCODE_BINARY_FRAME);
    }
    else {
      _curDropped++;
    }
  }

  return true;
}
#endif
