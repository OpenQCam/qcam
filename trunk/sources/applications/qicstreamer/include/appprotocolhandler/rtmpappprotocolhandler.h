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
#ifdef HAS_PROTOCOL_RTMP
#ifndef _RTMPAPPPROTOCOLHANDLER_H
#define _RTMPAPPPROTOCOLHANDLER_H

#include "protocols/rtmp/basertmpappprotocolhandler.h"

namespace app_qicstreamer {

  class RTMPAppProtocolHandler
  : public BaseRTMPAppProtocolHandler {
  public:
    RTMPAppProtocolHandler(Variant &configuration);
    virtual ~RTMPAppProtocolHandler();

    virtual bool ProcessInvokeGeneric(BaseRTMPProtocol *pFrom, Variant &request);
  private:
    bool ProcessGetAvailableFlvs(BaseRTMPProtocol *pFrom, Variant &request);
    bool ProcessInsertMetadata(BaseRTMPProtocol *pFrom, Variant &request);
    bool ProcessOnFCPublish(BaseRTMPProtocol *pFrom, Variant & request);
  };
}
#endif  /* _RTMPAPPPROTOCOLHANDLER_H */
#endif /* HAS_PROTOCOL_RTMP */

