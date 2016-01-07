#ifndef _RELAYMUXSTREAM_H
#define _RELAYMUXSTREAM_H

#include "mux/basemuxstream.h"

class RelayMuxStream
: public BaseMuxStream {
private:
  uint8_t* _pSPSPPS;
  uint32_t _SPSPPSLen;

protected:
public:
  RelayMuxStream(StreamsManager *pStreamsManager, string name);
  virtual ~RelayMuxStream();

  virtual bool IsCompatibleWithType(uint64_t type);
  virtual void OnDeviceVideoData(DeviceData& data);
  virtual void OnDeviceAudioData(DeviceData& data);
  virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
  virtual void OnDeviceMessage (DeviceMessage &msg);
};

#endif  /* _RELAYMUXSTREAM_H */
