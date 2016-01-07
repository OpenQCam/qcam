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

#ifndef _BASEAUDIOPLAYSTREAM_H
#define _BASEAUDIOPLAYSTREAM_H

#include "audio/baseaudiodevice.h"
#include "streaming/baseoutstream.h"

class StreamCapabilities;

class DLLEXP BaseAudioPlayStream
: public BaseOutStream,
  public IDeviceObserver

{
public:
  BaseAudioPlayStream(StreamsManager *pStreamsManager, uint64_t type, string name);
  virtual ~BaseAudioPlayStream();

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
	virtual void SignalAttachedToInStream();
	virtual void SignalDetachedFromInStream();
	virtual void SignalStreamCompleted();

protected:
private:
};


#endif  /* _AUDIOSTREAM_H */
