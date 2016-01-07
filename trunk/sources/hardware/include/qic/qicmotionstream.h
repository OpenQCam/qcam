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


#ifdef QIC1832

#ifndef _QICMOTIONSTREAM_H
#define _QICMOTIONSTREAM_H

#include "interface/deviceinterface.h"
#include "streaming/baseinstream.h"
#include "streaming/streamstypes.h"
#include "streaming/inmsgstream.h"


enum VideoStreamStatus {
  VSS_UNKNOWN = 0,
  VSS_START,
  VSS_STOP
};

class QICMotionCapture;
class InMSGStream;

class QICMotionStream
: public BaseInStream,
  public IDeviceObserver,
  public IDeviceCarrier{
private:
  StreamCapabilities *_pStreamCapabilities;
  bool _isInitVideo;
  double _currentTS;
  uint64_t _lastFrameTS;
  IOBuffer _dataBuffer;

  QICMotionCapture *_pMotionCaptureInstance;
  InMSGStream *_pSystemInfoStream;

private:
  void ResendSPSPPS();
  void RequestKeyFrame();

protected:
    VideoStreamStatus _status;

public:
  QICMotionStream(StreamsManager *pStreamsManager, string name);
  virtual ~QICMotionStream();

  virtual StreamCapabilities * GetCapabilities();
  virtual bool IsCompatibleWithType(uint64_t type);

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

  virtual void OnDeviceMessage(DeviceMessage &msg);
  virtual void OnDeviceData(DeviceData &data);

  void UnlinkLiveStream();
};

#endif  /* _QICMOTIONSTREAM_H */

#endif
