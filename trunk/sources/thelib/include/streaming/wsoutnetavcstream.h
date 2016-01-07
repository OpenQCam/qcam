
#ifdef HAS_PROTOCOL_HTTP
#ifndef _WSOUTNETAVCSTREAM_H
#define _WSOUTNETAVCSTREAM_H

#include "streaming/baseoutnetstream.h"

class BaseHTTPWSProtocol;
class WSOutNetAVCStream
: public BaseOutNetStream {
private:
  uint32_t _dropPeriod;
  uint32_t _curDropped;

public:
  WSOutNetAVCStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager, string streamName);
  virtual ~WSOutNetAVCStream();
	virtual void SignalAttachedToInStream();
  virtual void SignalDetachedFromInStream();
	virtual bool SignalPlay(double &absoluteTimestamp, double &length);
	virtual bool SignalPause();
	virtual bool SignalResume();
	virtual bool SignalSeek(double &absoluteTimestamp);
	virtual bool SignalStop();
	virtual bool IsCompatibleWithType(uint64_t type);
	virtual void SignalStreamCompleted();
	virtual bool Link(BaseInStream *pInStream, bool reverseLink);

  virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
      uint32_t processedLength, uint32_t totalLength,
      double absoluteTimestamp, bool isAudio);
};

#endif /* _WSOUTNETAVCSTREAM_H */

#endif /* HAS_PROTOCOL_HTT */
