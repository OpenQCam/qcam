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
#include "application/baseclientapplication.h"
#include "appprotocolhandler/httpappprotocolhandler.h"
#include "protocols/protocolmanager.h"
#include "protocols/baseprotocol.h"
#include "protocols/http/inboundhttpprotocol.h"

using namespace app_qicstreamer;

HTTPAppProtocolHandler::HTTPAppProtocolHandler(Variant &configuration)
: BaseHTTPAppProtocolHandler(configuration)
{

}

HTTPAppProtocolHandler::~HTTPAppProtocolHandler() {
}

bool HTTPAppProtocolHandler::ProcessMessage(BaseProtocol *pProtocol, IOBuffer &buffer) {
  DEBUG ("process message in http");
  return true;
}

#endif  /* HAS_PROTOCOL_HTTP */

