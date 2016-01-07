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


#ifdef HAS_PROTOCOL_CLI
#ifndef _INBOUNDXMLCLIPROTOCOL_H
#define _INBOUNDXMLCLIPROTOCOL_H

#include "protocols/cli/inboundbasecliprotocol.h"

class DLLEXP InboundXMLCLIProtocol
: public InboundBaseCLIProtocol {
private:
  bool _useLengthPadding;
public:
  InboundXMLCLIProtocol();
  virtual ~InboundXMLCLIProtocol();

  virtual bool Initialize(Variant &parameters);
  virtual bool SignalInputData(IOBuffer &buffer);
  virtual bool SendMessage(Variant &message);
private:
  bool ParseCommand(string &command);
};


#endif  /* _INBOUNDXMLCLIPROTOCOL_H */
#endif /* HAS_PROTOCOL_CLI */
