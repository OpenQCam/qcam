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

#include "appprotocolhandler/rtspappprotocolhandler.h"
#include "protocols/rtp/rtspprotocol.h"
#include "protocols/rtp/sdp.h"
#include "protocols/protocolfactorymanager.h"
#include "netio/netio.h"
#include "protocols/protocolmanager.h"
using namespace app_qicstreamer;

RTSPAppProtocolHandler::RTSPAppProtocolHandler(Variant& configuration)
: BaseRTSPAppProtocolHandler(configuration) {

}

RTSPAppProtocolHandler::~RTSPAppProtocolHandler() {
}

#endif /* HAS_PROTOCOL_RTP */

