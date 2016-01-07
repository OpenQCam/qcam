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

#ifndef _QICMJPGSTREAM_H
#define _QICMJPGSTREAM_H

#include "video/basevideostream.h"

class QICMJPGStream
: public BaseVideoStream {
private:
  StreamCapabilities *_pStreamCapabilities;

public:
  QICMJPGStream(StreamsManager *pStreamsManager, string name);
  virtual ~QICMJPGStream();

  virtual StreamCapabilities * GetCapabilities();

  //virtual bool FeedData(IOBuffer &buffer, struct timeval &tv);

  // QIC Data Callback
  //virtual void SendData (IOBuffer *buf, struct timeval *ts);
  // QIC Message Callback
  //virtual void OnNotifyMessage (NOTIFICATION_MSG_TYPE type, string message);
  //virtual void OnError(ERROR_MSG_TYPE type, string errmsg)se;

  virtual bool IsCompatibleWithType(uint64_t type);
  virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
  virtual void SignalOutStreamDetached(BaseOutStream *pOutStream);

  virtual void OnDeviceMessage (DeviceMessage &msg);
  virtual void OnDeviceData(DeviceData &data);
};

#endif  /* _QICMJPGSTREAM_H */
