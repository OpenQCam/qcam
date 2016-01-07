#ifndef _AVMUXSTREAM_H
#define _AVMUXSTREAM_H

#include "mux/basemuxstream.h"

class AVMuxStream
: public BaseMuxStream {
private:
  bool HLS_IsEnable;
protected:
public:
  BaseOutStream *_myOStream;
  AVMuxStream(StreamsManager *pStreamsManager, string name);
  virtual ~AVMuxStream();

  void EnableHLS();
  virtual bool IsCompatibleWithType(uint64_t type);
  virtual void OnDeviceVideoData(DeviceData& data);
  virtual void OnDeviceAudioData(DeviceData& data);
};

#endif  /* _AVMUXSTREAM_H */
