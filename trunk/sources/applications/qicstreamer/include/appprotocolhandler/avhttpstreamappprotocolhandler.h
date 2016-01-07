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
#ifndef _AVHTTPSTREAMAPPPROTOCOLHANDLER_H
#define _AVHTTPSTREAMAPPPROTOCOLHANDLER_H

#include "protocols/avhttpstream/baseavhttpstreamappprotocolhandler.h"

class BaseInStream;
class BaseOutHTTPStream;
class StreamsManager;
class InboundAVHTTPStreamProtocol;
namespace app_qicstreamer {

  class AVHTTPStreamAppProtocolHandler
  : public BaseAVHTTPStreamAppProtocolHandler {
  private:
    map<string, uint64_t> _supportedStreamType;
    StreamsManager *_pStreamsManager;
    bool ParseHttpRangeToken (Variant& msg, uint64_t &offset);

  public:
    AVHTTPStreamAppProtocolHandler(Variant &configuration);
    virtual ~AVHTTPStreamAppProtocolHandler();
    virtual bool ProcessMessage(InboundAVHTTPStreamProtocol *pFrom, Variant &message);

  private:
    bool IsValidRequest(string request);
    bool IsValidAPIVersion(string apiversion);
    BaseInStream * FindInStream(string streamName);
    BaseOutHTTPStream * FindOutStream(string streamName);
    BaseInStream * CreateInFile4HTTPStream(InboundAVHTTPStreamProtocol *pProtocol, string filePath, string fileName, uint64_t &size);

  };
};

#endif  /* _AVHTTPSTREAMAPPPROTOCOLHANDLER_H */
#endif /* HAS_PROTOCOL_AVHTTPSTREAM */
