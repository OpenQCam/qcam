
#ifndef _AUDIOFILESTREAM_H
#define _AUDIOFILESTREAM_H
#include "streaming/baseinstream.h"

class StreamsManager;

class AudioFileStream
: public BaseInStream {
private:
  FILE *_pAudioFile;

public:
  AudioFileStream(StreamsManager *pStreamsManager, string name);
  virtual ~AudioFileStream();

  virtual StreamCapabilities * GetCapabilities();

  virtual bool SignalPlay(double &absoluteTimestamp, double &length);
  virtual bool SignalPause();
  virtual bool SignalResume();
  virtual bool SignalSeek(double &absoluteTimestamp);
  virtual bool SignalStop();

  virtual void ReadyForSend();
  virtual bool IsCompatibleWithType(uint64_t type);
	virtual void SignalOutStreamAttached(BaseOutStream *pStream);
	virtual void SignalOutStreamDetached(BaseOutStream *pStream);
	virtual void SignalStreamCompleted();

  virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
    uint32_t processedLength, uint32_t totalLength,
    double absoluteTimestamp, bool isAudio);
  virtual bool Open(string filename);
  virtual void Close();
  virtual bool IsReadOver();
};


#endif /* _AUDIOFILESTREAM_H */
