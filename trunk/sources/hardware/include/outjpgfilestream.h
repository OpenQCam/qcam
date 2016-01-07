
//thelib

#ifndef _OUTJPGFILESTREAM_H
#define _OUTJPGFILESTREAM_H
#include "streaming/baseoutstream.h"


class PushHandler;
class StreamsManager;

class OutJPGFileStream
: public BaseOutStream
{
private:
  PushHandler* _pPushHandler;
  bool _bCaptured;
  double _captureTime;
  string _eventId;
  string _eventType;
  string _srcDir;
  string _srcFile;
  string _tarDir;

public:
  OutJPGFileStream (StreamsManager *pStreamManager,
                    PushHandler *pHandler, string name);

  virtual ~OutJPGFileStream();

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

  bool SnapShot(uint32_t delay, string filePath, string eventId, string type);
  bool CheckJPGStatus();
};
#endif /* _OUTJPGFILESTREAM_H */
