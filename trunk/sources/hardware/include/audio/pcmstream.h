

#ifndef _PCMSTREAM_H
#define _PCMSTREAM_H

#include "streaming/baseaudiostream.h"

class DLLEXP PCMStream
: public BaseAudioStream {
private:
  StreamCapabilities *_pStreamCapabilities;
  uint8_t _RTMPAudioHeader;
  bool _isInitAudio;

public:
  PCMStream(StreamsManager *pStreamsManager, string name);
  virtual ~PCMStream();
  virtual StreamCapabilities * GetCapabilities();
  virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
  virtual bool Play();
  virtual bool Stop();

  // IDeviceObserver
  virtual void OnDeviceMessage(DeviceMessage &msg);
  // IDeviceCarrier
  virtual void OnDeviceData(DeviceData &data);

  virtual uint32_t GetHeader(IOBuffer& headerBuf, uint32_t payloadLen);
  virtual uint32_t GetAudioDataTh();
protected:

private:
  IOBuffer _dataBuffer;
};

#endif  /* _PCMSTREAM_H */
