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
#ifndef _RESTCLIAPPPROTOCOLHANDLER_H
#define _RESTCLIAPPPROTOCOLHANDLER_H

#include "protocols/cli/baserestcliappprotocolhandler.h"

namespace app_qicstreamer {
  class RestCLIAppProtocolHandler
  : public BaseRestCLIAppProtocolHandler {
  private:
    map<uint32_t, string> _allowedAPI;

  public:
    RestCLIAppProtocolHandler(Variant &configuration);
    virtual ~RestCLIAppProtocolHandler();
    virtual void SetApplication(BaseClientApplication *pApplication);
    // message processing function
    virtual bool ProcessMessage(BaseProtocol *pFrom, Variant &message);
  };
}

#endif  /* _RESTCLIAPPPROTOCOLHANDLER_H */
#endif  /* HAS_PROTOCOL_CLI */
