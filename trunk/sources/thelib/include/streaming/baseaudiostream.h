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

#ifndef _BASEAUDIOSTREAM_H
#define _BASEAUDIOSTREAM_H

#include "audio/baseaudiodevice.h"
#include "streaming/baseinstream.h"

class StreamCapabilities;

//base audio capture stream
class DLLEXP BaseAudioStream
: public BaseInStream,
  public IDeviceObserver,
  public IDeviceCarrier

{
public:
  BaseAudioStream(StreamsManager *pStreamsManager, uint64_t type, string name);
  virtual ~BaseAudioStream();

  virtual bool SignalPlay(double &absoluteTimestamp, double &length);
  virtual bool SignalPause();
  virtual bool SignalResume();
  virtual bool SignalSeek(double &absoluteTimestamp);
  virtual bool SignalStop();
  virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
    uint32_t processedLength, uint32_t totalLength,
    double absoluteTimestamp, bool isAudio);
  virtual void ReadyForSend();
  virtual bool IsCompatibleWithType(uint64_t type);
  virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
  virtual void SignalOutStreamDetached(BaseOutStream *pOutStream);
  virtual StreamCapabilities * GetCapabilities() = 0;

  virtual uint32_t GetHeader(IOBuffer& headerBuf, uint32_t payloadlen) = 0;
  virtual uint32_t GetAudioDataTh()=0;
protected:
private:
};


#endif  /* _AUDIOSTREAM_H */
