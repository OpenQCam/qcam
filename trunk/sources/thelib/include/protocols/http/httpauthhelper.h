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

#ifndef _HTTPAUTHHELPER_H
#define	_HTTPAUTHHELPER_H

#include "common.h"


//TODO::Rename the class to a general class
class HTTPAuthHelper {
public:
	HTTPAuthHelper();
	virtual ~HTTPAuthHelper();

	static bool GetAuthorizationHeader(string wwwAuthenticateHeader,
			string username, string password, string uri, string method,
			Variant &result);
	static string GetWWWAuthenticateHeader(string type, string realmName);
	static bool ValidateAuthRequest(string rawChallange, string rawResponse,
			string method, string requestUri, Variant &realm);

//	static bool ValidateRTSPResponse(string& accessToken);
  static bool IsAuthedSuccess(string clientId, string token, string ts);
//  static void ExpireTicket(uint64_t ts);
  static bool SetAuthKey (string clientId, string key);
  static bool SetAuthTimeStamp (uint64_t ts, uint64_t expire);
  static void ClearAllClientId();
  static void ClearAuthToken (string clientId);
 // static void ClearTicket (uint64_t ticket);
  static void GetSHA1Token(string &token, const string& k1,
                           const string& k2, const string& k3);

	static uint32_t Size();
//  static uint32_t TicketSize();

private:
//  static map<uint64_t, uint64_t> _mTicketMap;
  static map<string, string> _mClientIdKeyMap;
	static bool ParseAuthLine(string challenge, Variant &result, bool isResponse);
	static bool ValidateChallenge(Variant &challenge);
	static bool ValidateResponse(Variant &response);
	static bool GetAuthorizationHeaderBasic(Variant &result);
	static bool GetAuthorizationHeaderDigest(Variant &result);
	static string ComputeResponseMD5(string username, string password,
			string realm, string method, string uri, string nonce);

  static void Print();
};

#endif	/* _HTTPAUTHHELPER_H */
