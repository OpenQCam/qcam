#include "mux/basemp4muxoutstream.h"

class StreamsManager;

class ManualMP4MuxOutStream
: public BaseMP4MuxOutStream
{
private:
  uint64_t _minReqSize;

public:
  ManualMP4MuxOutStream(StreamsManager *pStreamsManager, string name);
  virtual ~ManualMP4MuxOutStream();
  virtual bool StopRecord();

  virtual bool SignalStop();
  virtual bool SignalPlay(double &absTS, double &length);
  virtual bool SignalAudioPlay();
  virtual bool SignalAudioStop();

  virtual bool FeedData(uint8_t *pData,  uint32_t dataLength,
                        uint32_t processedLength, uint32_t totalLength,
                        double absoluteTimestamp, bool isAudio);
};
