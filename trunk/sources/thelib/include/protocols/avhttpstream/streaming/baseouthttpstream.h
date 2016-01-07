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

#ifndef _BASEOUTHTTPSTREAM_H
#define _BASEOUTHTTPSTREAM_H

#include "streaming/streamstypes.h"
#include "streaming/baseoutstream.h"
class BaseOutHTTPStream
: public BaseOutStream
{
  public:
  BaseOutHTTPStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager,
      uint64_t type, string name);
  virtual ~BaseOutHTTPStream();
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

#endif /* _BASEOUTHTTPSTREAM_H */

#endif
