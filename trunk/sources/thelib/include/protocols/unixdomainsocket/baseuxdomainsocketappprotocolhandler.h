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

#ifndef _BASEUXDOMAINSOCKETAPPPROTOCOLHANDLER_H
#define _BASEUXDOMAINSOCKETAPPPROTOCOLHANDLER_H

#include "application/baseappprotocolhandler.h"
#include "protocols/unixdomainsocket/udsmessage.h"

class BaseUXDomainSocketAppProtocolHandler
  : public BaseAppProtocolHandler
  , public IUDSMessageObserver
{
  private:
  public:
   BaseUXDomainSocketAppProtocolHandler(Variant &configuration);
    virtual ~BaseUXDomainSocketAppProtocolHandler();
    virtual void RegisterProtocol(BaseProtocol *pProtocol);
    virtual void UnRegisterProtocol(BaseProtocol *pProtocol);
    virtual bool ProcessMessage(BaseProtocol *pProtocol, IOBuffer &buffer) = 0;
};

#endif  /* _BASEUXDOMAINSOCKETAPPPROTOCOLHANDLER_H */
#endif /* HAS_PROTOCOL_UNIXDOMAIN_SOCKET */

