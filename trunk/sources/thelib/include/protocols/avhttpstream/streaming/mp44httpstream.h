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

#ifndef _MP44HTTPSTREAM_H
#define _MP44HTTPSTREAM_H

#include "protocols/avhttpstream/streaming/baseouthttpstream.h"

class InboundAVHTTPStreamProtocol;
class MP44HTTPStream
: public BaseOutHTTPStream
{
  private:
    InboundAVHTTPStreamProtocol *_pAVProtocol;
    uint64_t _fileSize;
  public:
    MP44HTTPStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager,
        string name, uint64_t size);
    virtual ~MP44HTTPStream();

    virtual void SignalAttachedToInStream();
    virtual void SignalDetachedFromInStream();
    virtual void SignalStreamCompleted();
    virtual bool IsCompatibleWithType(uint64_t type);
    virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
        uint32_t processedLength, uint32_t totalLength,
        double absoluteTimestamp, bool isAudio);
};

#endif /* _MP44HTTPSTREAM_H */

#endif
