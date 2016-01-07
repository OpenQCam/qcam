// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    :
// // Version :
// // Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//

#ifndef _BASEVIDEOSTREAM_H
#define _BASEVIDEOSTREAM_H

#include "interface/deviceinterface.h"
#include "streaming/baseinstream.h"
#include "streaming/streamstypes.h"
#include "streaming/streamcapabilities.h"

enum VideoStreamStatus {
  VSS_UNKNOWN = 0,
  VSS_START,
  VSS_STOP
};

class BaseVideoStream
: public BaseInStream,
  public IDeviceObserver,
  public IDeviceCarrier
{
  protected:
    BaseVideoStream(StreamsManager *pStreamsManager, uint64_t type, string name);

  protected:
    VideoStreamStatus _status;

  public:
    virtual ~BaseVideoStream();
    virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
        uint32_t processedLength, uint32_t totalLength,
        double absoluteTimestamp, bool isAudio);

    virtual VideoStreamStatus GetStatus();
    virtual bool SignalPlay(double &absoluteTimestamp, double &length);
    virtual bool SignalPause();
    virtual bool SignalResume();
    virtual bool SignalSeek(double &absoluteTimestamp);
    virtual bool SignalStop();
    virtual void ReadyForSend();
    virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
    virtual void SignalOutStreamDetached(BaseOutStream *pOutStream);

    //virtual bool FeedData(IOBuffer &buffer, struct timeval &tv) = 0;
    //virtual StreamCapabilities * GetCapabilities() = 0;
    //virtual void OnDeviceMessage (const DeviceMessage &msg) = 0;
    //virtual void OnDeviceData(const DeviceData &data) = 0;
};


#endif  /* _BASEVIDEOSTREAM_H */

