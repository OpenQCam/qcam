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
#ifndef _BASEMJPGHTTPSTREAMAPPPROTOCOLHANDLER_H
#define	_BASEMJPGHTTPSTREAMAPPPROTOCOLHANDLER_H

#include "application/baseappprotocolhandler.h"

class BaseMJPGHTTPStreamAppProtocolHandler
: public BaseAppProtocolHandler {
public:
	BaseMJPGHTTPStreamAppProtocolHandler(Variant &configuration);
	virtual ~BaseMJPGHTTPStreamAppProtocolHandler();

	virtual void RegisterProtocol(BaseProtocol *pProtocol);
	virtual void UnRegisterProtocol(BaseProtocol *pProtocol);
};


#endif	/* _BASEMJPGHTTPSTREAMAPPPROTOCOLHANDLER_H */
#endif /* HAS_PROTOCOL_MJPGHTTPSTREAM */
