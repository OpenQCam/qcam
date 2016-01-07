#ifndef _OUTJPGSTREAM_H
#define _OUTJPGSTREAM_H
//thelib
#include "streaming/baseoutstream.h"

class StreamsManager;

class OutJPGStream
: public BaseOutStream
{
private:
  list<uint64_t> _capList;

public:
  OutJPGStream (StreamsManager *pStreamManager, string name);
  virtual ~OutJPGStream();

	virtual void SignalAttachedToInStream();
	virtual void SignalDetachedFromInStream();
	virtual void SignalStreamCompleted();
  virtual bool SignalPlay(double &absoluteTimestamp, double &length);
  virtual bool SignalPause();
  virtual bool SignalResume();
  virtual bool SignalSeek(double &absoluteTimestamp);
  virtual bool SignalStop();
  virtual bool IsCompatibleWithType(uint64_t type);
  virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
      uint32_t processedLength, uint32_t totalLength,
      double absoluteTimestamp, bool isAudio) ;

  bool SnapShot(uint32_t delay);
  bool CheckJPGStatus();
};
#endif /* _OUTJPGFILESTREAM_H */
