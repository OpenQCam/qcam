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

#include "appprotocolhandler/rawhttpstreamappprotocolhandler.h"
using namespace app_qicstreamer;

RawHTTPStreamAppProtocolHandler::RawHTTPStreamAppProtocolHandler(
		Variant &configuration)
: BaseRawHTTPStreamAppProtocolHandler(configuration) {

}

RawHTTPStreamAppProtocolHandler::~RawHTTPStreamAppProtocolHandler() {
}

#endif /* HAS_PROTOCOL_RAWHTTPSTREAM */
