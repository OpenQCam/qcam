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

#ifndef _QIC32AVCSTREAM_H
#define _QIC32AVCSTREAM_H

#include "video/basevideostream.h"

class QIC32AVCVideoCapture;

class QIC32AVCStream
: public BaseVideoStream {
private:

  StreamCapabilities *_pStreamCapabilities;
  bool _requestKeyFrame;
  bool _isInitVideo;
  double _currentTS;
  uint64_t _lastFrameTS;
  IOBuffer _dataBuffer;

  QIC32AVCVideoCapture *_pAVCCaptureInstance;

private:
  void ResendSPSPPS();
  void RequestKeyFrame();

public:
  QIC32AVCStream(StreamsManager *pStreamsManager, string name);
  virtual ~QIC32AVCStream();

  virtual StreamCapabilities * GetCapabilities();
  virtual bool IsCompatibleWithType(uint64_t type);

  virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
  virtual void SignalOutStreamDetached(BaseOutStream *pOutStream);

  virtual void OnDeviceMessage (DeviceMessage &msg);
  virtual void OnDeviceData(DeviceData &data);

  void UnlinkLiveStream();
};

#endif  /* _QIC32AVCSTREAM_H */

#endif
