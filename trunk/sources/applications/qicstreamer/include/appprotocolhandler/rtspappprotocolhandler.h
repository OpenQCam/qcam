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
#ifdef HAS_PROTOCOL_RTP
#ifndef _RTSPAPPPROTOCOLHANDLER_H
#define	_RTSPAPPPROTOCOLHANDLER_H

#include "protocols/rtp/basertspappprotocolhandler.h"

namespace app_qicstreamer {

	class RTSPAppProtocolHandler
	: public BaseRTSPAppProtocolHandler {
	public:
		RTSPAppProtocolHandler(Variant &configuration);
		virtual ~RTSPAppProtocolHandler();
	};
}

#endif	/* _RTSPAPPPROTOCOLHANDLER_H */
#endif /* HAS_PROTOCOL_RTP */

