/* ==================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 * (C) COPYRIGHT Quanta Computer Inc. ALL RIGHTS RESERVED
 *
 * ==================================================================
 * ------------------------------------------------------------------
 *  Date    :
 *  Version :
 *  Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 *  Purpose :
 *  =================================================================
 */

#ifdef HAS_PROTOCOL_AVHTTPSTREAM

#ifndef _SNAPSHOT4HTTPSTREAM_H
#define _SNAPSHOT4HTTPSTREAM_H

#include "protocols/avhttpstream/streaming/baseouthttpstream.h"
#include "video/videodefine.h"
#include "interface/deviceinterface.h"

class InboundAVHTTPStreamProtocol;
class Snapshot4HTTPStream
: public BaseOutHTTPStream, IDeviceObserver
{
  private:
    InboundAVHTTPStreamProtocol *_pAVProtocol;
    bool _isSent;
  public:
    Snapshot4HTTPStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager, string name);
    virtual ~Snapshot4HTTPStream();

    virtual void SignalAttachedToInStream();
    virtual void SignalDetachedFromInStream();
    virtual void SignalStreamCompleted();
    virtual bool IsCompatibleWithType(uint64_t type);
    virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
        uint32_t processedLength, uint32_t totalLength,
        double absoluteTimestamp, bool isAudio);

    // DeviceObserver message callback
    virtual void OnDeviceMessage(DeviceMessage &msg);
};

#endif /* _SNAPSHOT4HTTPSTREAM_H */

#endif
