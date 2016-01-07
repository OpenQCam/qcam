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

#ifdef QIC1816

#ifndef _QICAVCSTREAM_H
#define _QICAVCSTREAM_H

#include "video/basevideostream.h"

class QICAVCVideoCapture;

class QICAVCStream
: public BaseVideoStream {
private:

  struct NALU {
    uint8_t *start;
    uint32_t length;
    uint8_t type;
  };

  uint8_t _naluIndex;
  NALU _naluSet[100]; //FIXME: if slide size increase, find out correct nalu set size
  StreamCapabilities *_pStreamCapabilities;
  bool _requestKeyFrame;
  bool _isInitVideo;
  double _currentTS;
  uint32_t _lastTS;
  IOBuffer _dataBuffer;
  IOBuffer _videoBuffer;

  QICAVCVideoCapture *_pAVCCaptureInstance;



private:
  int FindAUD(uint8_t *data, uint32_t length);
  void ResendSPSPPS();
  void RequestKeyFrame();

public:
  QICAVCStream(StreamsManager *pStreamsManager, string name);
  virtual ~QICAVCStream();

  virtual StreamCapabilities * GetCapabilities();
  virtual bool IsCompatibleWithType(uint64_t type);

  virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
  virtual void SignalOutStreamDetached(BaseOutStream *pOutStream);

  //virtual bool FeedData(IOBuffer &buffer, struct timeval &tv);
  // QIC Data Callback
  //virtual void SendData (IOBuffer *buf, struct timeval *ts);
  // QIC Message Callback
  //virtual void OnNotifyMessage (NOTIFICATION_MSG_TYPE type, string message);
  //virtual void OnError(ERROR_MSG_TYPE type, string errmsg);
  virtual void OnDeviceMessage (DeviceMessage &msg);
  virtual void OnDeviceData(DeviceData &data);


  //Signal event to linked outboundstream
  virtual bool SignalStart();
  virtual bool SignalStop();


};

#endif  /* _QICAVCSTREAM_H */

#endif
