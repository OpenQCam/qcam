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
#include "protocols/unixdomainsocket/baseuxdomainsocketappprotocolhandler.h"
#include "protocols/unixdomainsocket/baseudsmessageprotocol.h"

BaseUXDomainSocketAppProtocolHandler::BaseUXDomainSocketAppProtocolHandler(Variant& configuration)
: BaseAppProtocolHandler(configuration)
{

}

BaseUXDomainSocketAppProtocolHandler::~BaseUXDomainSocketAppProtocolHandler()
{

}

void BaseUXDomainSocketAppProtocolHandler::RegisterProtocol(BaseProtocol *pProtocol)
{
  ((BaseUDSMessageProtocol*)(pProtocol))->RegisterObserver(this);
}

void BaseUXDomainSocketAppProtocolHandler::UnRegisterProtocol(BaseProtocol *pProtocol)
{
  ((BaseUDSMessageProtocol*)(pProtocol))->RemoveObserver();
}

#endif /* HAS_PROTOCOL_UNIXDOMAIN_SOCKET */

