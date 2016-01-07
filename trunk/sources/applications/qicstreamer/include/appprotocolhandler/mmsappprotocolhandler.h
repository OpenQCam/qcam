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
#ifdef HAS_PROTOCOL_MMS
#ifndef _MMSAPPPROTOCOLHANDLER_H
#define	_MMSAPPPROTOCOLHANDLER_H

#include "protocols/mms/basemmsappprotocolhandler.h"

namespace app_qicstreamer {

	class MMSAppProtocolHandler
	: public BaseMMSAppProtocolHandler {
	public:
		MMSAppProtocolHandler(Variant &configuration);
		virtual ~MMSAppProtocolHandler();
	};
};
#endif	/* _MMSAPPPROTOCOLHANDLER_H */
#endif /* HAS_PROTOCOL_MMS */
