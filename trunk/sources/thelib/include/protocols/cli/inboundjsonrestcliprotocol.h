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
#ifndef _INBOUNDJSONRESTCLIPROTOCOL_H
#define _INBOUNDJSONRESTCLIPROTOCOL_H

#include "protocols/baseprotocol.h"

class InboundHTTPProtocol;
class BaseRestCLIAppProtocolHandler;

class InboundJSONRestCLIProtocol
: public BaseProtocol {
private:
  BaseRestCLIAppProtocolHandler *_pProtocolHandler;
  InboundHTTPProtocol *_pHTTP;
  bool _useLengthPadding;
  Variant _message;
  IOBuffer _outputBuffer;
  IOBuffer *_pInputBuffer;
  URI _uri;
  string _sessionId;
public:
  InboundJSONRestCLIProtocol();
  virtual ~InboundJSONRestCLIProtocol();

  virtual bool Initialize(Variant &parameters);
  virtual bool SignalInputData(IOBuffer &buffer);
  virtual void SetApplication(BaseClientApplication *pApplication);
  virtual bool AllowFarProtocol(uint64_t type);
  virtual bool AllowNearProtocol(uint64_t type);
  virtual IOBuffer * GetOutputBuffer();
  virtual IOBuffer * GetInputBuffer();
  virtual bool SignalInputData(int32_t recvAmount);

  bool SendMessage(uint16_t httpStatusCode, Variant &message);
  bool SendJsonpMessage(uint16_t httpStatusCode, string callbackFunction, Variant &message);
  string GetSessionId();
private:
  //bool ParseURI(const string &reqest);
};

#endif  /* _INBOUNDJSONRESTCLIPROTOCOL_H */
#endif /* HAS_PROTOCOL_CLI */
