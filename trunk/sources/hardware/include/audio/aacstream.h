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

#ifndef _AACSTREAM_H
#define	_AACSTREAM_H

#ifdef HAS_CODEC_AACSTREAM
#include "audio/audiostream.h"

class DLLEXP AACStream
: public AudioStream {
public:
	AACStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager, string name, BaseAudioCaptureDevice *pAudioCapDevice);
	virtual ~AACStream();
	virtual StreamCapabilities * GetCapabilities();
  virtual void SendData(uint8_t *buf, uint32_t size, struct timeval *ts);
	virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
	virtual uint32_t GetHeader(IOBuffer &headerBuf, uint32_t payloadlen, AUDIOHEADER_TYPE headerType);
  virtual uint32_t GetAudioDataTh();
  virtual uint32_t GetDataGuardRange();
protected:

private:
	StreamCapabilities *_pAACstreamCapabilities;
  IOBuffer _dataBuffer;

};
#endif //HAS_CODEC_AACSTREAM

#endif	/* _AACSTREAM_H */

