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
#ifndef _BASERESTCLIAPPPROTOCOLHANDLER_H
#define _BASERESTCLIAPPPROTOCOLHANDLER_H

#include "application/baseappprotocolhandler.h"

class DLLEXP BaseRestCLIAppProtocolHandler
: public BaseAppProtocolHandler {
private:
  //static const string kRestCUIDString;
  //static const string kRestSessionIdString;
  //static const string kRestErrorCodeString;
  //static const string kRestDescriptionString;
  //static const string kRestErrorString;
  //static const string kRestSuccessString;
  //static const string kRestDataString;

protected:
  BaseRestCLIAppProtocolHandler(Variant &configuration);
public:
  //static  bool SendMessage(BaseProtocol *pTo, uint16_t httpStatusCode, Variant &data);
  virtual ~BaseRestCLIAppProtocolHandler();

  virtual void RegisterProtocol(BaseProtocol *pProtocol);
  virtual void UnRegisterProtocol(BaseProtocol *pProtocol);

  virtual bool ProcessMessage(BaseProtocol *pFrom, Variant &message) = 0;
  //virtual bool SendFail(BaseProtocol *pTo, uint16_t httpStatusCode, uint32_t errorCode, string description);
  //virtual bool SendSuccess(BaseProtocol *pTo, uint16_t httpStatusCode, Variant &data);
};

#endif  /* _BASERESTCLIAPPPROTOCOLHANDLER_H */
#endif /* HAS_PROTOCOL_CLI */
