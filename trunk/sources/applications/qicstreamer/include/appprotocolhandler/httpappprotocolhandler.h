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
#ifndef _HTTPAPPPROTOCOLHANDLER_H
#define	_HTTPAPPPROTOCOLHANDLER_H

#include "protocols/http/basehttpprotocolhandler.h"

class BaseProtocol;

namespace app_qicstreamer {

	class HTTPAppProtocolHandler
	: public BaseHTTPAppProtocolHandler {
  private:
  public:
	  HTTPAppProtocolHandler(Variant &configuration);
	  virtual ~HTTPAppProtocolHandler();
    virtual bool ProcessMessage(BaseProtocol *pProtocol, IOBuffer &buffer);
	};
}

#endif	/* _HTTPAPPPROTOCOLHANDLER_H */
#endif	/* HAS_PROTOCOL_HTTP */


