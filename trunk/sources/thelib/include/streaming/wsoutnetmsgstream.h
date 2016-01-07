
#ifdef HAS_PROTOCOL_HTTP
#ifndef _WSOUTNETMSGSTREAM_H
#define _WSOUTNETMSGSTREAM_H

#include "streaming/baseoutnetstream.h"

class BaseHTTPWSProtocol;

class WSOutNetMSGStream
: public BaseOutNetStream {
private:
public:
  WSOutNetMSGStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager, string streamName);
  virtual ~WSOutNetMSGStream();
	virtual bool SignalPlay(double &absoluteTimestamp, double &length);
	virtual bool SignalPause();
	virtual bool SignalResume();
	virtual bool SignalSeek(double &absoluteTimestamp);
	virtual bool SignalStop();
	virtual bool IsCompatibleWithType(uint64_t type);
	virtual void SignalStreamCompleted();
	virtual void SignalAttachedToInStream();
  virtual void SignalDetachedFromInStream();
	virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
			uint32_t processedLength, uint32_t totalLength,
			double absoluteTimestamp, bool isAudio);
};

#endif /* _WSOUTNETMSGSTREAM_H */

#endif /* HAS_PROTOCOL_HTTP */
