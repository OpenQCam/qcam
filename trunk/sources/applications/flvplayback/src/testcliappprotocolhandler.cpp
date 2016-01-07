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
#include "testcliappprotocolhandler.h"
#include "protocols/cli/inboundjsoncliprotocol.h"
using namespace app_flvplayback;

TestCLIAppProtocolHandler::TestCLIAppProtocolHandler(Variant &configuration)
: BaseCLIAppProtocolHandler(configuration) {

}

TestCLIAppProtocolHandler::~TestCLIAppProtocolHandler() {
}


bool TestCLIAppProtocolHandler::ProcessMessage(BaseProtocol *pFrom, Variant &message){
  INFO("show msg %s", STR(message.ToString()));
  Variant sendmsg;
  sendmsg["item1"] = "aaaaa";
  sendmsg["item2"] = "bbbbb";
  sendmsg["item3"] = "ccccc";
  InboundJSONCLIProtocol *pCLI = reinterpret_cast<InboundJSONCLIProtocol *>(pFrom);
  pCLI->SendMessage(sendmsg);

  return true;
}

#endif	/* HAS_PROTOCOL_CLI */

