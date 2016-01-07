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

#ifdef HAS_PROTOCOL_LIVEFLV
#ifndef _LIVEFLVAPPPROTOCOLHANDLER_H
#define	_LIVEFLVAPPPROTOCOLHANDLER_H

#include "protocols/liveflv/baseliveflvappprotocolhandler.h"
namespace app_qicstreamer {

	class LiveFLVAppProtocolHandler
	: public BaseLiveFLVAppProtocolHandler {
	public:
		LiveFLVAppProtocolHandler(Variant &configuration);
		virtual ~LiveFLVAppProtocolHandler();
	};
}

#endif	/* _LIVEFLVAPPPROTOCOLHANDLER_H */
#endif /* HAS_PROTOCOL_LIVEFLV */

