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
#ifndef _INBOUNDMJPGHTTPSTREAMPROTOCOL_H
#define	_INBOUNDMJPGHTTPSTREAMPROTOCOL_H

#include "protocols/baseprotocol.h"

class OutNetMJPGHTTPStream;
class InboundHTTPProtocol;

class InboundMJPGHTTPStreamProtocol
: public BaseProtocol {
private:
	bool _streamNameAcquired;
	string _streamName;
	string _crossDomainFile;
	bool _headersSent;
	IOBuffer _outputBuffer;
	OutNetMJPGHTTPStream *_pOutStream;
public:
	InboundMJPGHTTPStreamProtocol();
	virtual ~InboundMJPGHTTPStreamProtocol();

	virtual bool Initialize(Variant &parameters);
	virtual IOBuffer * GetOutputBuffer();
	virtual bool AllowFarProtocol(uint64_t type);
	virtual bool AllowNearProtocol(uint64_t type);
	virtual bool SignalInputData(int32_t recvAmount);
	virtual bool SignalInputData(IOBuffer &buffer);
	bool PutData(uint8_t *pBuffer, uint32_t length);
private:
	bool AcquireStreamName(IOBuffer &buffer);
	bool Send404NotFound();
	bool SendCrossDomain();
};


#endif	/* _INBOUNDMJPGHTTPSTREAMPROTOCOL_H */
#endif /* HAS_PROTOCOL_MJPGHTTPSTREAM */
