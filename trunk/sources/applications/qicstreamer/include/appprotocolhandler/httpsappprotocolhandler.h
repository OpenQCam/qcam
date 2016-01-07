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
#ifdef HAS_PROTOCOL_HTTP
#ifndef _HTTPSAPPPROTOCOLHANDLER_H
#define	_HTTPSAPPPROTOCOLHANDLER_H

#include "protocols/http/basehttpprotocolhandler.h"

namespace app_qicstreamer {

	class HTTPSAppProtocolHandler
	: public BaseHTTPAppProtocolHandler {
	public:
		HTTPSAppProtocolHandler(Variant &configuration);
		virtual ~HTTPSAppProtocolHandler();
	};
}

#endif	/* _HTTPSAPPPROTOCOLHANDLER_H */
#endif	/* HAS_PROTOCOL_HTTP */


