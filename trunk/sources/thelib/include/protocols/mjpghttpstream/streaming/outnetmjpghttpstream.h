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

#ifdef HAS_PROTOCOL_MJPGHTTPSTREAM
#ifndef _OUTNETMJPGSTREAM_H
#define	_OUTNETMJPGSTREAM_H

#include "streaming/baseoutnetstream.h"

class InboundMJPGHTTPStreamProtocol;

class OutNetMJPGHTTPStream
: public BaseOutNetStream {
private:
	uint64_t _bytesCount;
	uint64_t _packetsCount;
public:
	OutNetMJPGHTTPStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager,
			string name);
	virtual ~OutNetMJPGHTTPStream();

	virtual void GetStats(Variant &info, uint32_t namespaceId = 0);
	virtual void SignalAttachedToInStream();
	virtual void SignalDetachedFromInStream();
	virtual void SignalStreamCompleted();
	virtual bool SignalPlay(double &absoluteTimestamp, double &length);
	virtual bool SignalPause();
	virtual bool SignalResume();
	virtual bool SignalSeek(double &absoluteTimestamp);
	virtual bool SignalStop();
	virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
			uint32_t processedLength, uint32_t totalLength,
			double absoluteTimestamp, bool isAudio);
	virtual bool IsCompatibleWithType(uint64_t type);
};

#endif	/* _OUTNETMJPGSTREAM_H */
#endif /* HAS_PROTOCOL_MJPGHTTPSTREAM */
