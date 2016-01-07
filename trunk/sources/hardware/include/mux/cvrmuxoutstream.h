#include "mux/basemp4muxoutstream.h"
#include "system/errordefine.h"

#define EVT_CVR_READY             1
#define EVT_CVR_OUT_OF_STORAGE    ERROR_OUT_OF_STORAGE
#define EVT_CVR_OUT_OF_NAS        ERROR_OUT_OF_NAS
#define EVT_CVR_STORAGE_FAILED    ERROR_STORAGE
#define EVT_CVR_NAS_FAILED        ERROR_NAS

// zero MSb 6-bit is normal event
// 0000_001x_xxxx_xxxx
#define EVT_CVR_ERROR(MSGTYPE) (MSGTYPE>>9)

class StreamsManager;

class CVRMuxOutStream
: public BaseMP4MuxOutStream
{
  public:
    CVRMuxOutStream(StreamsManager *pStreamsManager, string name);
    virtual ~CVRMuxOutStream();
    virtual bool FeedData (uint8_t *pData, uint32_t dataLength,
                           uint32_t processedLength, uint32_t totalLength,
                           double absoluteTimestamp, bool isAudio);
    virtual bool StartRecord();
    virtual bool StopRecord();

    virtual bool SignalStop();
    virtual bool SignalPlay(double &absTS, double &length);
    virtual bool SignalAudioPlay();
    virtual bool SignalAudioStop();
};
