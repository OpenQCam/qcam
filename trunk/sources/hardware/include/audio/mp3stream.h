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
// // Author  : Recardo Cheng (recardo.cheng@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//

#ifndef _MP3STREAM_H
#define _MP3STREAM_H

#ifdef HAS_CODEC_MP3STREAM
#define  BITSSIZE 4096
#define  SAMP_PER_FRAME 576
#include "audio/baseaudiostream.h"

//#ifdef RT3352
//#include "audio/i2saudiocap.h"
//#else
//#define PCM_PAGE_SIZE 1152
//#endif /* RT3352 */

extern "C" {
#include "audio/mp3enc/g_includes.h"
#include "audio/mp3enc/bitstream.h"
#include "audio/mp3enc/Layer3.h"
#include "audio/mp3enc/L3subband.h"
#include "audio/mp3enc/L3loop.h"
#include "audio/mp3enc/L3mdct.h"
#include "audio/mp3enc/L3bitstrea.h"
}

class DLLEXP MP3Stream
: public BaseAudioStream {
public:
  MP3Stream(StreamsManager *pStreamsManager, string name);
  virtual ~MP3Stream();
  virtual StreamCapabilities * GetCapabilities();
  //virtual void SendData(uint8_t *buf, uint32_t size, struct timeval *ts);
  // IDeviceObserver
  virtual void OnDeviceMessage(DeviceMessage &msg);
  // IDeviceCarrier
  virtual void OnDeviceData(DeviceData &data);
  virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
  virtual uint32_t GetHeader(IOBuffer &headerBuf, uint32_t payloadLen);
  virtual uint32_t GetAudioDataTh();
  virtual uint32_t GetDataGuardRange();
protected:

private:
  StreamCapabilities *_pMP3streamCapabilities;
  IOBuffer _dataBuffer;
  uint32_t _numOfChannel;
  uint32_t _sampleRate;
  uint32_t _bitsPerSample;
  uint32_t _bitRate; //encoding bitrate
  uint32_t _samplesInput; //number of sample to encode

  //codec relative parameters
  //TODO: move to codec and provide an interface for mp3stream class,
  L3_side_info_t _side_info;
  config_t _mp3config;
  int _sideinfo_len;
  double _avg_slots_per_frame;
  double _frac_slots_per_frame;
  long _whole_slots_per_frame;
  double _slot_lag;
  bitstream_t _bs;
  int _l3_enc[2][2][SAMP_PER_FRAME];
  long _l3_sb_sample[2][3][18][SBLIMIT];
  long _mdct_freq[2][2][SAMP_PER_FRAME];

  uint8_t pcmbuf[4096];

};
#endif //HAS_CODEC_MP3STREAM

#endif  /* _MP3STREAM_H */

