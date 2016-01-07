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


#ifdef HAS_PROTOCOL_CLI
#include "protocols/cli/baserestcliappprotocolhandler.h"
#include "protocols/cli/restclidefine.h"
#include "protocols/cli/inboundjsonrestcliprotocol.h"

BaseRestCLIAppProtocolHandler::BaseRestCLIAppProtocolHandler(Variant &configuration)
: BaseAppProtocolHandler(configuration) {
}

BaseRestCLIAppProtocolHandler::~BaseRestCLIAppProtocolHandler() {
}

void BaseRestCLIAppProtocolHandler::RegisterProtocol(BaseProtocol *pProtocol) {
}

void BaseRestCLIAppProtocolHandler::UnRegisterProtocol(BaseProtocol *pProtocol) {
}

#endif /* HAS_PROTOCOL_CLI */
