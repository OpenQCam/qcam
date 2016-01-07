
#ifdef HAS_PROTOCOL_HTTP
#ifndef _WSOUTNETSTREAM_H
#define _WSOUTNETSTREAM_H

#define WS_OUTNETSTREAM_NAME "wsoutnetstream"
#define WS_OUTNETSTREAM_MP3_NAME "wsoutnetstream_mp3"

#include "streaming/baseinstream.h"
#include "streaming/baseoutnetstream.h"
#define AUDIOTHRE (1024*8)
#define AUDIO_GUARDSIZE (0)

class BaseHTTPWSProtocol;
class WSOutNetAudioStream
: public BaseOutNetStream {
private:
  IOBuffer _soundBuffer;
  uint32_t _dataThreshold;
public:

  WSOutNetAudioStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager, string streamName);
  virtual ~WSOutNetAudioStream();
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
	virtual bool FeedMSGData(uint8_t *pDAta, uint32_t dataLength);
};

#endif /* _WSOUTNETSTREAM_H */

#endif /* HAS_PROTOCOL_HTTP */
