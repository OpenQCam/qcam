#ifndef _BASEMUXSTREAM_H
#define _BASEMUXSTREAM_H

#include "video/basevideostream.h"
#define MAX_NALU_COUNT 10
#define NALU_TYPE_SLICE    1
#define NALU_TYPE_DPA      2
#define NALU_TYPE_DPB      3
#define NALU_TYPE_DPC      4
#define NALU_TYPE_IDR      5
#define NALU_TYPE_SEI      6
#define NALU_TYPE_SPS      7
#define NALU_TYPE_PPS      8
#define NALU_TYPE_AUD      9
#define NALU_TYPE_EOSEQ    10
#define NALU_TYPE_EOSTREAM 11
#define NALU_TYPE_FILL     12

// frame header for RTMP
typedef struct {
  uint8_t header[10];
  uint8_t length;
} RTMPFrameHeader;

#if 0
// TODO : RTMP header for main profile and high profile
static RTMPFrameHeader RTMPInitHeader = {
  { 0x17, 0x0, 0x0, 0x0, 0x0, 0x1, 0x42, 0xE0, 0x1F, 0x3 },
  10,
};

static RTMPFrameHeader RTMPIFrameHeader = {
  { 0x17, 0x1, 0x0, 0x0, 0x0 },
  5,
};

static RTMPFrameHeader RTMPPFrameHeader = {
 { 0x27, 0x1, 0x0, 0x0, 0x0 },
 5,
};

static RTMPFrameHeader RTMPEOSHeader = {
 { 0x17, 0x2, 0x0, 0x0, 0x0 },
 5,
};
#endif

class BaseAVCVideoCapture;

class BaseMuxStream
: public BaseVideoStream {
private:
  BaseAVCVideoCapture *_pAVCCaptureInstance;

protected:
  StreamCapabilities *_pStreamCapabilities;
  bool _isInitVideo;
  bool _isInitAudio;
  bool _requestKeyFrame;
  double _currentTS;
  uint64_t _lastFrameTS;
  IOBuffer _dataBuffer;
  uint8_t _RTMPAudioHeader;

  struct NALU_t {
    uint8_t type;
    uint8_t *data;
    uint32_t length;
  } _nalus[MAX_NALU_COUNT];


  void ResendSPSPPS();
  void RequestKeyFrame();

  uint32_t FindNALU(uint8_t *pBuf, uint32_t length, uint8_t &naluType);
  uint8_t ParseFrame(uint8_t *pBuf, uint32_t length, uint8_t naluNum);

public:
  BaseMuxStream(StreamsManager *pStreamsManager, uint64_t type, string name);
  virtual ~BaseMuxStream();

  virtual StreamCapabilities * GetCapabilities();
  virtual bool IsCompatibleWithType(uint64_t type);

  virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
  virtual void SignalOutStreamDetached(BaseOutStream *pOutStream);

  virtual void OnDeviceMessage (DeviceMessage &msg);
  virtual void OnDeviceData(DeviceData &data);
  virtual void OnDeviceVideoData(DeviceData& data)=0;
  virtual void OnDeviceAudioData(DeviceData& data)=0;
  void UnlinkLiveStream();
};

#endif  /* _AVMUXSTREAM_H */
