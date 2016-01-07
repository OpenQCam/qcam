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

#include "appselectorapplication.h"
#include "protocols/protocoltypes.h"
#include "application/clientapplicationmanager.h"
#include "protocols/rtmp/basertmpprotocol.h"
#include "protocols/rtmp/outboundrtmpprotocol.h"
#include "protocols/http/httpauthhelper.h"
#include "rtmpappprotocolhandler.h"
#include "httpappprotocolhandler.h"
#include "system/nvramdefine.h"
//hardware
#include "system/systemmanager.h"

using namespace app_appselector;

AppSelectorApplication::AppSelectorApplication(Variant &configuration)
: BaseClientApplication(configuration) {
#ifdef HAS_PROTOCOL_RTMP
	_pRTMPHandler = NULL;
#endif /* HAS_PROTOCOL_RTMP */
#ifdef HAS_PROTOCOL_HTTP
	_pHTTPHandler = NULL;
#endif /* HAS_PROTOCOL_HTTP */
}

AppSelectorApplication::~AppSelectorApplication() {
#ifdef HAS_PROTOCOL_RTMP
	UnRegisterAppProtocolHandler(PT_INBOUND_RTMP);
	UnRegisterAppProtocolHandler(PT_OUTBOUND_RTMP);
	if (_pRTMPHandler != NULL) {
		delete _pRTMPHandler;
		_pRTMPHandler = NULL;
	}
#endif /* HAS_PROTOCOL_RTMP */
#ifdef HAS_PROTOCOL_HTTP
	UnRegisterAppProtocolHandler(PT_INBOUND_HTTP);
	UnRegisterAppProtocolHandler(PT_OUTBOUND_HTTP);
	if (_pHTTPHandler != NULL) {
		delete _pHTTPHandler;
		_pHTTPHandler = NULL;
	}
#endif /* HAS_PROTOCOL_HTTP */
}

bool AppSelectorApplication::Initialize() {
	if (!BaseClientApplication::Initialize()) {
		FATAL("Unable to initialize application");
		return false;
	}
#ifdef HAS_PROTOCOL_RTMP
	_pRTMPHandler = new RTMPAppProtocolHandler(_configuration);
	RegisterAppProtocolHandler(PT_INBOUND_RTMP, _pRTMPHandler);
	RegisterAppProtocolHandler(PT_INBOUND_RTMPS_DISC, _pRTMPHandler);
	RegisterAppProtocolHandler(PT_INBOUND_SSL, _pRTMPHandler);
	RegisterAppProtocolHandler(PT_OUTBOUND_RTMP, _pRTMPHandler);
#endif /* HAS_PROTOCOL_RTMP */
#ifdef HAS_PROTOCOL_HTTP
	_pHTTPHandler = new HTTPAppProtocolHandler(_configuration);
	RegisterAppProtocolHandler(PT_INBOUND_HTTP_FOR_RTMP, _pHTTPHandler);
	RegisterAppProtocolHandler(PT_OUTBOUND_HTTP_FOR_RTMP, _pHTTPHandler);
#endif /* HAS_PROTOCOL_HTTP */
	return true;
}

bool AppSelectorApplication::IsAuthedSuccess(string uriString) {
  URI uri;
  string accessToken;
  string timeStamp;
  string clientId;


  if (SystemManager::IsFactoryMode()) {
    return true;
  }

  URI::FromString(uriString, false, uri);
  Variant params=uri.parameters();
  if (!params.HasKey(URL_CLIENT_ID)) {
    clientId=(string)SystemManager::GetNVRam("Login");
  }
  else {
    clientId=(string)params[URL_CLIENT_ID];
  }
  if (params.HasKey(URL_ACCESS_TOKEN)) {
    accessToken=(string)params[URL_ACCESS_TOKEN];
  }
  if (params.HasKey(URL_TIMESTAMP)) {
    timeStamp=(string)params[URL_TIMESTAMP];
  }

  DEBUG ("clientId:%s, accessToken:%s, timeStamp:%s",
         STR(clientId), STR(accessToken), STR(timeStamp));
  if (HTTPAuthHelper::IsAuthedSuccess(clientId, accessToken, timeStamp)){
    return true;
  }
  return false;
}
