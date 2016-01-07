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
#ifndef _RTPAPPPROTOCOLHANDLER_H
#define	_RTPAPPPROTOCOLHANDLER_H

#include "protocols/rtp/basertpappprotocolhandler.h"

namespace app_qicstreamer {

	class RTPAppProtocolHandler
	: public BaseRTPAppProtocolHandler {
	public:
		RTPAppProtocolHandler(Variant &configuration);
		virtual ~RTPAppProtocolHandler();
	};
}

#endif	/* _RTPAPPPROTOCOLHANDLER_H */
#endif /* HAS_PROTOCOL_RTP */

