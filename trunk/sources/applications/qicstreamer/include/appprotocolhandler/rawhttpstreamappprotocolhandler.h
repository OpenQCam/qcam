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
#ifdef HAS_PROTOCOL_RAWHTTPSTREAM
#ifndef _RAWHTTPSTREAMAPPPROTOCOLHANDLER_H
#define	_RAWHTTPSTREAMAPPPROTOCOLHANDLER_H

#include "protocols/rawhttpstream/baserawhttpstreamappprotocolhandler.h"

namespace app_qicstreamer {

	class RawHTTPStreamAppProtocolHandler
	: public BaseRawHTTPStreamAppProtocolHandler {
	public:
		RawHTTPStreamAppProtocolHandler(Variant &configuration);
		virtual ~RawHTTPStreamAppProtocolHandler();
	};
};

#endif	/* _RAWHTTPSTREAMAPPPROTOCOLHANDLER_H */
#endif /* HAS_PROTOCOL_RAWHTTPSTREAM */
