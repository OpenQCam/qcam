
#ifdef HAS_PROTOCOL_HTTP
#include "streaming/streamstypes.h"
#include "streaming/wsoutnetmsgstream.h"
#include "protocols/http/websocket/basewssubprotocol.h"

WSOutNetMSGStream::WSOutNetMSGStream (BaseProtocol *pProtocol,
  StreamsManager *pStreamsManager,
  string name)
: BaseOutNetStream (pProtocol, pStreamsManager, ST_OUT_WSNET_MSG, name) {
}

WSOutNetMSGStream::~WSOutNetMSGStream() {
}

bool WSOutNetMSGStream::SignalPlay (double &absTimeStamp, double &length)
{
	return true;
}

bool WSOutNetMSGStream::SignalPause()
{
	return true;
}

bool WSOutNetMSGStream::SignalResume()
{
	return true;
}

bool WSOutNetMSGStream::SignalSeek(double &absoluteTimestamp)
{
	return true;
}

bool WSOutNetMSGStream::SignalStop()
{
  return true;
}

bool WSOutNetMSGStream::IsCompatibleWithType(uint64_t type)
{
  return true;
}

void WSOutNetMSGStream::SignalStreamCompleted()
{
}

void WSOutNetMSGStream::SignalAttachedToInStream()
{
  DEBUG ("wsoutnetmsgstream attached to instream");
}

void WSOutNetMSGStream::SignalDetachedFromInStream() {
  DEBUG ("wsoutnetmsgstream deattached to instream");
}

bool WSOutNetMSGStream::FeedData(uint8_t *pData, uint32_t dataLength,
  uint32_t processedLength, uint32_t totalLength,
  double absoluteTimestamp, bool isAudio) {

  if (IsEnqueueForDelete()) {
    DEBUG ("Is EnququeForDelete return");
    return false;
  }

	if (_pProtocol) {
	  BaseWSSubProtocol *pProtocol=reinterpret_cast<BaseWSSubProtocol*>(_pProtocol);

    if (pProtocol->EnqueueForWSOutbound (pData, dataLength, true, WS_OPCODE_BINARY_FRAME)) {
      string evt=string((char*)(pData+12), dataLength-12);
      DEBUG ("\nheader:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n event:%s",
             pData[0], pData[1], pData[2], pData[3],
             pData[4], pData[5], pData[6], pData[7],
             pData[8], pData[9], pData[10], pData[11],
             STR(evt));
      return true;
    }
  }
  return false;
}

#endif
