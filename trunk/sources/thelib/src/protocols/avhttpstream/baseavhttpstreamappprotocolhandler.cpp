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
//
#ifdef HAS_PROTOCOL_AVHTTPSTREAM

#include "protocols/avhttpstream/baseavhttpstreamappprotocolhandler.h"

BaseAVHTTPStreamAppProtocolHandler::BaseAVHTTPStreamAppProtocolHandler(
		Variant &configuration)
: BaseAppProtocolHandler(configuration) {

}

BaseAVHTTPStreamAppProtocolHandler::~BaseAVHTTPStreamAppProtocolHandler() {
}

void BaseAVHTTPStreamAppProtocolHandler::RegisterProtocol(BaseProtocol *pProtocol) {

}

void BaseAVHTTPStreamAppProtocolHandler::UnRegisterProtocol(BaseProtocol *pProtocol) {

}

#endif /* HAS_PROTOCOL_AVHTTPSTREAM */
