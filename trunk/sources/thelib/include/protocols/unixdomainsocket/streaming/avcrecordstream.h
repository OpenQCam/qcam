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

#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET

#ifndef _AVCRECORDSTREAM_H
#define _AVCRECORDSTREAM_H

#include "protocols/unixdomainsocket/streaming/baserecordstream.h"

class UnixDomainSocketProtocol;
class AVCRecordStream
: public BaseRecordStream
{
  private:
    UnixDomainSocketProtocol *_pUXProtocol;
  public:
    AVCRecordStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager, string name);
    virtual ~AVCRecordStream();

    virtual void SignalAttachedToInStream();
    virtual void SignalDetachedFromInStream();
    virtual void SignalStreamCompleted();
    virtual bool IsCompatibleWithType(uint64_t type);
    virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
        uint32_t processedLength, uint32_t totalLength,
        double absoluteTimestamp, bool isAudio);
};

#endif /* _AVCRECORDSTREAM_H */

#endif

