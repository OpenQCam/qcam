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
#ifdef HAS_PROTOCOL_TS
#ifndef _TSAPPPROTOCOLHANDLER_H
#define	_TSAPPPROTOCOLHANDLER_H

#include "protocols/ts/basetsappprotocolhandler.h"

namespace app_qicstreamer {

	class TSAppProtocolHandler
	: public BaseTSAppProtocolHandler {
	public:
		TSAppProtocolHandler(Variant &configuration);
		virtual ~TSAppProtocolHandler();
	};
}

#endif	/* _TSAPPPROTOCOLHANDLER_H */
#endif	/* HAS_PROTOCOL_TS */


