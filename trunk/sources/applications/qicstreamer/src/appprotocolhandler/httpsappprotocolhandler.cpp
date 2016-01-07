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
#include "appprotocolhandler/httpsappprotocolhandler.h"
using namespace app_qicstreamer;

HTTPSAppProtocolHandler::HTTPSAppProtocolHandler(Variant &configuration)
: BaseHTTPAppProtocolHandler(configuration) {

}

HTTPSAppProtocolHandler::~HTTPSAppProtocolHandler() {
}
#endif	/* HAS_PROTOCOL_HTTP */

