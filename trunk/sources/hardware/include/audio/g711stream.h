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

#ifndef _G711STREAM_H
#define _G711STREAM_H

#define G711STREAM_DEBUG
#include "streaming/baseaudiostream.h"

class DLLEXP G711Stream
: public BaseAudioStream {
private:
  StreamCapabilities *_pStreamCapabilities;

public:
  G711Stream(StreamsManager *pStreamsManager, string name);
  virtual ~G711Stream();
  virtual StreamCapabilities * GetCapabilities();

  // IDeviceObserver
  virtual void OnDeviceMessage(DeviceMessage &msg);
  // IDeviceCarrier
  virtual void OnDeviceData(DeviceData &data);
  virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
  virtual uint32_t GetHeader(IOBuffer& headerBuf, uint32_t payloadlen);
  virtual uint32_t GetAudioDataTh();
  virtual uint32_t GetDataGuardRange();
protected:

private:
  IOBuffer _dataBuffer;
};

#endif  /* _G711STREAM_H */

