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

#ifndef _BASERECORDSTREAM_H
#define _BASERECORDSTREAM_H

#include "streaming/streamstypes.h"
#include "streaming/baseoutstream.h"
class BaseRecordStream
: public BaseOutStream
{
  public:
  BaseRecordStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager,
      uint64_t type, string name);
  virtual ~BaseRecordStream();
  virtual void SignalAttachedToInStream() = 0;
  virtual void SignalDetachedFromInStream() = 0;
  virtual void SignalStreamCompleted() = 0;
  virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
      uint32_t processedLength, uint32_t totalLength,
      double absoluteTimestamp, bool isAudio) = 0;
  virtual bool IsCompatibleWithType(uint64_t type) = 0;

  virtual bool SignalPlay(double &absoluteTimestamp, double &length);
  virtual bool SignalPause();
  virtual bool SignalResume();
  virtual bool SignalSeek(double &absoluteTimestamp);
  virtual bool SignalStop();

};

#endif /* _BASERECORDSTREAM_H */

#endif
