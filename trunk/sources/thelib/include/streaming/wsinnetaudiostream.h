
#ifdef HAS_PROTOCOL_HTTP
#ifndef _WSINNETAUDIOSTREAM_H
#define _WSINNETAUDIOSTREAM_H

#include "streaming/baseinnetstream.h"
class WSInNetAudioStream
: public BaseInNetStream {
private:
  
public:
  WSInNetAudioStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager, string name);
  virtual ~WSInNetAudioStream();
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
			double absoluteTimestamp, bool isAudio);

};

#endif /* _WSOUTNETSTREAM_H */

#endif /* HAS_PROTOCOL_HTTP */
