/*
 *  Copyright (c) 2010,
 *  Gavriloaie Eugen-Andrei (shiretu@gmail.com)
 *
 *  This file is part of crtmpserver.
 *  crtmpserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  crtmpserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with crtmpserver.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifdef HAS_PROTOCOL_HTTP
#ifndef _INBOUNDHTTPWSPROTOCOL_H
#define _INBOUNDHTTPWSPROTOCOL_H

#include "basehttpwsprotocol.h"
#include "protocols/cli/statuscode.h"

#define WS_IN_CONNECTION_SUCCESS 1
class BaseWebSocketAppProtocolHandler;

class InboundHTTPWSProtocol
: public BaseHTTPWSProtocol {
private:
  BaseWebSocketAppProtocolHandler* _pWSHandler;
  uint16_t _statusCode;
  string _httpMethod;

protected:
  bool _isAuthed;
  //virtual bool SignalInboundWebSocketText (IOBuffer &buffer, WebSocketDataFrame &dataFrame);
  //virtual bool SignalInboundWebSocketBinary (IOBuffer &buffer, WebSocketDataFrame &dataFrame);
  virtual bool SignalInboundWebSocketClose();

public:
  InboundHTTPWSProtocol();
  virtual ~InboundHTTPWSProtocol();

  virtual void SetApplication(BaseClientApplication *pApplication);
  virtual bool SignalInputData(IOBuffer &buffer);
  virtual bool SignalDataProcessed(); // called after basic http parsing
  virtual bool EnqueueForOutbound();
  virtual bool IsBufferFull();
  virtual BaseWebSocketAppProtocolHandler* GetProtocolHandler();

  //  http required function
  virtual bool Initialize(Variant &parameters);
  virtual string GetOutputFirstLine();
  virtual bool ParseFirstLine(string &line, Variant &headers);
  virtual bool Authenticate();

  void SetStatusCode(uint16_t statusCode);
  bool SendOptionsAccepted();   // File Upload
  bool Send400BadRequest();
  bool Send101SwitchProtocol(); // WebSocket
  bool SendHTTPResponse(RestHTTPCode statuCode);

  string GetHTTPMethod();

private:
  bool SendAuthRequired(Variant &auth);

};


#endif /* _INBOUNDHTTPWSPROTOCOL_H */
#endif /* HAS_PROTOCOL_HTTP */
