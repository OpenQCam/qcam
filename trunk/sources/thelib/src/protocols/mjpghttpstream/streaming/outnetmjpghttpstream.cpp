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
#include "protocols/mjpghttpstream/streaming/outnetmjpghttpstream.h"
#include "streaming/streamstypes.h"
#include "protocols/protocoltypes.h"
#include "protocols/mjpghttpstream/inboundmjpghttpstreamprotocol.h"

OutNetMJPGHTTPStream::OutNetMJPGHTTPStream(BaseProtocol *pProtocol,
		StreamsManager *pStreamsManager, string name)
: BaseOutNetStream(pProtocol, pStreamsManager, ST_OUT_NET_MJPGHTTP, name) {
	if (pProtocol == NULL) {
		ASSERT("OutNetMJPGHTTPStream requires a protocol to host it");
	}
	if (_pProtocol->GetType() != PT_INBOUND_RAW_HTTP_STREAM) {
		ASSERT("OutNetMJPGHTTPStream can be hosted only inside %s protocol",
				STR(tagToString(PT_INBOUND_RAW_HTTP_STREAM)));
	}
	_bytesCount = 0;
	_packetsCount = 0;
}

OutNetMJPGHTTPStream::~OutNetMJPGHTTPStream() {
}

void OutNetMJPGHTTPStream::GetStats(Variant &info, uint32_t namespaceId) {
	BaseOutNetStream::GetStats(info, namespaceId);
	info["video"]["bytesCount"] = _bytesCount;
	info["video"]["packetsCount"] = _packetsCount;
	info["video"]["droppedPacketsCount"] = 0;
	info["audio"]["bytesCount"] = 0;
	info["audio"]["packetsCount"] = 0;
	info["audio"]["droppedPacketsCount"] = 0;
}

void OutNetMJPGHTTPStream::SignalAttachedToInStream() {

}

void OutNetMJPGHTTPStream::SignalDetachedFromInStream() {
	NYI;
}

void OutNetMJPGHTTPStream::SignalStreamCompleted() {
	NYI;
}

bool OutNetMJPGHTTPStream::SignalPlay(double &absoluteTimestamp, double &length) {
	NYIR;
}

bool OutNetMJPGHTTPStream::SignalPause() {
	NYIR;
}

bool OutNetMJPGHTTPStream::SignalResume() {
	NYIR;
}

bool OutNetMJPGHTTPStream::SignalSeek(double &absoluteTimestamp) {
	NYIR;
}

bool OutNetMJPGHTTPStream::SignalStop() {
	NYIR;
}

bool OutNetMJPGHTTPStream::FeedData(uint8_t *pData, uint32_t dataLength,
		uint32_t processedLength, uint32_t totalLength,
		double absoluteTimestamp, bool isAudio) {
	_bytesCount += dataLength;
	_packetsCount++;
	if (_pProtocol != NULL) {
		return ((InboundMJPGHTTPStreamProtocol *) _pProtocol)->PutData(pData, dataLength);
	}
	return false;
}

bool OutNetMJPGHTTPStream::IsCompatibleWithType(uint64_t type) {
	//This stream is compatible with everything
	return TAG_KIND_OF(type, ST_IN_CAM_MJPG);
}
#endif /* HAS_PROTOCOL_MJPGHTTPSTREAM */
