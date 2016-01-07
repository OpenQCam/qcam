
#ifndef _INMSGSTREAM_H
#define _INMSGSTREAM_H

#include "streaming/baseinstream.h"

class BaseOutStream;

class InMSGStream
: public BaseInStream{

private:
  queue<string> _queuedMsg;

public:
  InMSGStream(StreamsManager *pStreamsManager, string streamName);
  virtual ~InMSGStream();
	virtual bool SignalPlay(double &absoluteTimestamp, double &length);
	virtual bool SignalPause();
	virtual bool SignalResume();
	virtual bool SignalSeek(double &absoluteTimestamp);
	virtual bool SignalStop();
	virtual bool IsCompatibleWithType(uint64_t type);
	virtual void SignalStreamCompleted();
	virtual void ReadyForSend();
  virtual StreamCapabilities *GetCapabilities();
	virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
	virtual void SignalOutStreamDetached(BaseOutStream *pOutStream);
  virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
      uint32_t processedLength, uint32_t totalLength,
      double absoluteTimestamp, bool isQueueMsg);

  virtual bool FeedMSGData(uint8_t *pData, uint32_t dataLength, bool queued);
};

#endif
