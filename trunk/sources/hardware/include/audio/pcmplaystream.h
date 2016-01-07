
#ifndef _PCMPLAYSTREAM_H
#define _PCMPLAYSTREAM_H

#include "audio/baseaudioplaystream.h"
class Speaker;

class DLLEXP PCMPlayStream
: public BaseAudioPlayStream {
private:
  Speaker *_pSpeakerDevice;
  uint32_t _audioFrameSize;
  bool _bReadyToStop;

public:
  PCMPlayStream(StreamsManager *pStreamsManager, string name);
  virtual ~PCMPlayStream();
  virtual StreamCapabilities * GetCapabilities();

  // IDeviceObserver
  virtual void OnDeviceMessage(DeviceMessage &msg);
  virtual uint32_t GetHeader(IOBuffer& headerBuf, uint32_t payloadLen);
  virtual uint32_t GetAudioDataTh();
  virtual uint32_t GetDataGuardRange();
  virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
                        uint32_t procLength, uint32_t totalLength,
                        double absTS, bool isAudio);

	virtual void SignalAttachedToInStream();
	virtual void SignalDetachedFromInStream();
	virtual bool SignalStop();
  virtual bool PullAudioData();
protected:

private:
  IOBuffer _dataBuffer;
};

#endif
