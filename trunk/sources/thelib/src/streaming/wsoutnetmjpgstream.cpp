
#ifdef HAS_PROTOCOL_HTTP
#include "streaming/wsoutnetmjpgstream.h"
#include "streaming/streamsmanager.h"
#include "streaming/streamstypes.h"
#include "system/eventdefine.h"
#include "protocols/http/websocket/basewssubprotocol.h"
#include "application/baseclientapplication.h"

#define MAXVIDEOBUFSIZE (256*1024)

WSOutNetMJPGStream::WSOutNetMJPGStream (BaseProtocol *pProtocol,
  StreamsManager *pStreamsManager,
  string streamName)
: BaseOutNetStream (pProtocol, pStreamsManager, ST_OUT_WSNET_MJPG, streamName) {
}

WSOutNetMJPGStream::~WSOutNetMJPGStream() {
}

bool WSOutNetMJPGStream::SignalPlay (double &absTimeStamp, double &length)
{
	return true;
}

bool WSOutNetMJPGStream::SignalPause()
{
	return true;
}

bool WSOutNetMJPGStream::SignalResume()
{
	return true;
}

bool WSOutNetMJPGStream::SignalSeek(double &absoluteTimestamp)
{
	return true;
}

bool WSOutNetMJPGStream::SignalStop()
{
  return true;
}

bool WSOutNetMJPGStream::IsCompatibleWithType(uint64_t type)
{
  return true;
}

void WSOutNetMJPGStream::SignalStreamCompleted() {
}

void WSOutNetMJPGStream::SignalAttachedToInStream() {
  if (_pStreamsManager) {
    const_cast<BaseClientApplication*>
        (_pStreamsManager->GetApplication())->OnNotifyEvent(CLOUD_MSG_EVENT,
                                                            EVT_OUT_MJPG_CONNECTED,
                                                            0);
  }
}

void WSOutNetMJPGStream::SignalDetachedFromInStream() {
  if (_pStreamsManager) {
    const_cast<BaseClientApplication*>
        (_pStreamsManager->GetApplication())->OnNotifyEvent(CLOUD_MSG_EVENT,
                                                            EVT_OUT_MJPG_DISCONNECTED,
                                                            0);
  }
}

bool WSOutNetMJPGStream::FeedData(uint8_t *pData, uint32_t dataLength,
		uint32_t processedLength, uint32_t totalLength,
		double absoluteTimestamp, bool isAudio) {

  uint32_t curBufSize=0;

  if (isAudio){
    return true;
  }

  _dropPeriod=0;

  if (IsEnqueueForDelete())
    return true;

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
    WARN ("http mjpg congestion, drop frame");
    return true;
  }

	if (_pProtocol) {
    if (_curDropped >= _dropPeriod) {
      _curDropped=0;
      reinterpret_cast<BaseWSSubProtocol*>(_pProtocol)->EnqueueForWSOutbound (pData, dataLength, true, WS_OPCODE_BINARY_FRAME);
    }
    else {
      _curDropped++;
    }
  }

  return true;
}
#endif
