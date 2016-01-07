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
#include "appprotocolhandler/mmsappprotocolhandler.h"
using namespace app_qicstreamer;

MMSAppProtocolHandler::MMSAppProtocolHandler(Variant &configuration)
: BaseMMSAppProtocolHandler(configuration) {

}

MMSAppProtocolHandler::~MMSAppProtocolHandler() {
}

#endif /* HAS_PROTOCOL_MMS */
