/* ==================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 * (C) COPYRIGHT Quanta Computer Inc. ALL RIGHTS RESERVED
 *
 * ==================================================================
 * ------------------------------------------------------------------
 *  Date    :
 *  Version :
 *  Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 *  Purpose :
 *  =================================================================
 */

#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
#ifndef _UXDOMAINSOCKETAPPPROTOCOLHANDLER_H
#define _UXDOMAINSOCKETAPPPROTOCOLHANDLER_H

#include "protocols/unixdomainsocket/baseuxdomainsocketappprotocolhandler.h"

class UnixDomainSocketProtocol;

namespace app_qicstreamer {

  class UXDomainSocketAppProtocolHandler
  : public BaseUXDomainSocketAppProtocolHandler {
    private:
      typedef bool (UXDomainSocketAppProtocolHandler::*SocketFunction)(UnixDomainSocketProtocol *, IOBuffer &);
      struct UXSocketData {
        //bool isTransferDone;
        uint8_t type;
        uint32_t id;
        uint32_t dataLength;
        IOBuffer dataBuffer;
      };
      map<string, SocketFunction> _uxSocketMap;
      queue<UXSocketData*> _qSystemSocketData;
      queue<UXSocketData*> _qInfoSocketData;
    public:
      UXDomainSocketAppProtocolHandler(Variant &configuration);
      virtual ~UXDomainSocketAppProtocolHandler();
      virtual bool ProcessMessage(UnixDomainSocketProtocol *pProtocol, IOBuffer &buffer);
    private:
      inline void ParseUXDomainSocketData(queue<UXSocketData*> &socketData, IOBuffer &buffer);
      bool CVRSocket(UnixDomainSocketProtocol *pProtocol, IOBuffer &buffer);
      bool InfoSocket(UnixDomainSocketProtocol *pProtocol, IOBuffer &buffer);
      bool EventSocket(UnixDomainSocketProtocol *pProtocol, IOBuffer &buffer);
      bool SystemSocket(UnixDomainSocketProtocol *pProtocol, IOBuffer &buffer);
      bool IpcEventSocket(UnixDomainSocketProtocol *pProtocol, IOBuffer &buffer);
  };
}

#endif  /* _UXDOMAINSOCKETAPPPROTOCOLHANDLER_H */
#endif /* HAS_PROTOCOL_UNIXDOMAIN_SOCKET */
