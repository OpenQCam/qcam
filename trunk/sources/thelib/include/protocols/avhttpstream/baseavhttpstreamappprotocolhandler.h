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

#ifdef HAS_PROTOCOL_AVHTTPSTREAM
#ifndef _BASEAVHTTPSTREAMAPPPROTOCOLHANDLER_H
#define _BASEAVHTTPSTREAMAPPPROTOCOLHANDLER_H

#include "application/baseappprotocolhandler.h"

class InboundAVHTTPStreamProtocol;
class BaseAVHTTPStreamAppProtocolHandler
: public BaseAppProtocolHandler {

public:
  BaseAVHTTPStreamAppProtocolHandler(Variant &configuration);
  virtual ~BaseAVHTTPStreamAppProtocolHandler();

  virtual void RegisterProtocol(BaseProtocol *pProtocol);
  virtual void UnRegisterProtocol(BaseProtocol *pProtocol);
  virtual bool ProcessMessage(InboundAVHTTPStreamProtocol *pFrom, Variant &message) = 0;
};


#endif  /* _BASEAVHTTPSTREAMAPPPROTOCOLHANDLER_H */
#endif /* HAS_PROTOCOL_AVHTTPSTREAM */
