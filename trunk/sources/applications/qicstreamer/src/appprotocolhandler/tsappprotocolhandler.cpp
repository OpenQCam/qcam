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
#include "appprotocolhandler/tsappprotocolhandler.h"
#include "application/baseclientapplication.h"
#include "protocols/ts/tspacketpmt.h"
#include "protocols/rtmp/sharedobjects/so.h"
#include "protocols/rtmp/sharedobjects/somanager.h"
#include "protocols/rtmp/basertmpappprotocolhandler.h"
#include "protocols/protocoltypes.h"
using namespace app_qicstreamer;

TSAppProtocolHandler::TSAppProtocolHandler(Variant &configuration)
: BaseTSAppProtocolHandler(configuration) {

}

TSAppProtocolHandler::~TSAppProtocolHandler() {
}
#endif	/* HAS_PROTOCOL_TS */

