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

#include "protocols/http/httpauthhelper.h"

#define AUTH_PARSING_STAGE_KEY			0
#define AUTH_PARSING_STAGE_BEGIN_VALUE	1
#define AUTH_PARSING_STAGE_VALUE		2
#define AUTH_PARSING_STAGE_SEPARATOR	3

//map<uint64_t, uint64_t> HTTPAuthHelper::_mTicketMap;
map<string, string> HTTPAuthHelper::_mClientIdKeyMap;

HTTPAuthHelper::HTTPAuthHelper() {
}

HTTPAuthHelper::~HTTPAuthHelper() {
}

//void HTTPAuthHelper::ExpireTicket(uint64_t ts) {
//  map<uint64_t, uint64_t>::iterator it=_mTicketMap.begin();
//
//  while (it!=_mTicketMap.end()) {
//    if (ts>(it->second)) {
//      _mTicketMap.erase(it++);
//    }
//    else {
//      it++;
//    }
//  }
//}

void HTTPAuthHelper::ClearAllClientId() {
  _mClientIdKeyMap.clear();
}

//void HTTPAuthHelper::ClearTicket(uint64_t ticket) {
//  MAP_ERASE1(_mTicketMap, ticket);
//}

void HTTPAuthHelper::ClearAuthToken(string clientId) {
  MAP_ERASE1(_mClientIdKeyMap, clientId);
  Print();
}

uint32_t HTTPAuthHelper::Size() {
  return _mClientIdKeyMap.size();
}

//uint32_t HTTPAuthHelper::TicketSize() {
//  return _mTicketMap.size();
//}

void HTTPAuthHelper::Print() {
  //FOR_MAP (_mTicketMap, uint64_t, uint64_t, i) {
  //  DEBUG ("ticket:%llu", MAP_KEY(i));
  //}
  FOR_MAP (_mClientIdKeyMap, string, string, i) {
    DEBUG ("clientId:%s key:%s", STR(MAP_KEY(i)), STR(MAP_VAL(i)));
  }
}


bool HTTPAuthHelper::SetAuthKey(string clientId, string key) {
  if (!MAP_HAS1(_mClientIdKeyMap, clientId)) {
    _mClientIdKeyMap[clientId]=key;
    return true;
  }
  return true;
}

//bool HTTPAuthHelper::SetAuthTimeStamp(uint64_t ts, uint64_t expire) {
//  if (!MAP_HAS1(_mTicketMap, ts)) {
//    _mTicketMap[ts]=expire;
//    return true;
//  }
//  return false;
//}

bool HTTPAuthHelper::IsAuthedSuccess(string clientId, string token, string ts) {

  uint64_t timeStamp=stringToNum<uint64_t>(ts);
  uint64_t now;
  GETTIMESTAMP(now)
  if (now>timeStamp) {
    DEBUG ("time expired:now:%llu, ts:%llu", now, timeStamp);
    return false;
  }
  if (!MAP_HAS1(_mClientIdKeyMap, clientId)) {
    return false;
  }

  string vToken;
  GetSHA1Token(vToken, clientId, _mClientIdKeyMap[clientId], ts);
  DEBUG ("vToken:%s, clientId:%s, key:%s, ts:%s, inputToken:%s",
         STR(vToken), STR(clientId),
         STR(_mClientIdKeyMap[clientId]), STR(ts),
         STR(token));
  if (vToken.compare(token)==0) {
    //ClearTicket(timeStamp);
    return true;
  }

  return false;
}

bool HTTPAuthHelper::GetAuthorizationHeader(string wwwAuthenticateHeader,
		string username, string password, string uri, string method, Variant &result) {

	result.Reset();
	result["authLine"]["raw"] = wwwAuthenticateHeader;
	result["username"] = username;
	result["password"] = password;
	result["uri"] = uri;
	result["method"] = method;
	//LOG_HTTP_AUTH_STAGE;

	if (!ParseAuthLine(wwwAuthenticateHeader, result["authLine"], false)) {
		FATAL("Unable to parse challenge: %s", STR(wwwAuthenticateHeader));
		return false;
	}

	if (result["authLine"]["method"] == "Digest") {
		return GetAuthorizationHeaderDigest(result);
	} else {
		return GetAuthorizationHeaderBasic(result);
	}
}

string HTTPAuthHelper::GetWWWAuthenticateHeader(string type, string realmName) {
	string result = "";
	replace(realmName, "\\", "\\\\");
	replace(realmName, "\"", "\\\"");
	result = type + " realm=\"" + realmName + "\"";
	if (type == "Digest") {
		result += ", nonce=\"" + md5(generateRandomString(8), true) + "\", algorithm=\"MD5\"";
	}
	return result;
}

bool HTTPAuthHelper::ValidateAuthRequest(string rawChallange, string rawResponse,
		string method, string requestUri, Variant &realm) {
	Variant challenge;
	Variant response;

	if (!ParseAuthLine(rawChallange, challenge, false)) {
		FATAL("Unable to parse challenge: %s", STR(rawChallange));
		return false;
	}
	if (!ValidateChallenge(challenge)) {
		FATAL("Invalid server challenge");
		return false;
	}
	if (!ParseAuthLine(rawResponse, response, true)) {
		FATAL("Unable to parse response: %s", STR(rawResponse));
		return false;
	}

		//FINEST("realm:\n%s", STR(realm.ToString()));
		//FINEST("challenge:\n%s", STR(challenge.ToString()));
		//FINEST("response:\n%s", STR(response.ToString()));

	if (challenge["method"] != response["method"]) {
		FATAL("challenge/response type miss match");
		return false;
	}

	if (realm["method"] != challenge["method"]) {
		FATAL("challenge/response type miss match");
		return false;
	}

	if (challenge["parameters"]["nonce"] != response["parameters"]["nonce"]) {
		FATAL("challenge/response nonce miss match");
		return false;
	}

	if (challenge["method"] == "Digest") {
		if (challenge["parameters"]["realm"] != response["parameters"]["realm"]) {
			FATAL("challenge/response realm miss match");
			return false;
		}

		if (realm["name"] != response["parameters"]["realm"]) {
			FATAL("challenge/response realm miss match");
			return false;
		}

		if (challenge["parameters"]["algorithm"] != response["parameters"]["algorithm"]) {
			FATAL("challenge/response algorithm miss match");
			return false;
		}
	} else {
		if (realm["name"] != challenge["parameters"]["realm"]) {
			FATAL("challenge/response realm miss match");
			return false;
		}
	}

  //Ignore username check
	//if (!realm["users"].HasKey(response["parameters"]["username"])) {
	//	FATAL("Invalid username");
	//	return false;
	//}

	if (realm["method"] == "Digest") {
		string username = response["parameters"]["username"];
		string password = realm["users"][username];
		string realmName = realm["name"];
		string nonce = response["parameters"]["nonce"];
		string uri = response["parameters"]["uri"];
		trim(requestUri);
		if (requestUri == "") {
			FATAL("Invalid URI");
			return false;
		}
		//#define HTTP_HELPER_CHECK_URI_AUTH
#ifdef HTTP_HELPER_CHECK_URI_AUTH
		if (uri.length() > requestUri.length()) {
			FATAL("Invalid URI");
			return false;
		}
		WARN("URI checking NYI");
		string part = requestUri.substr(requestUri.length() - uri.length());
		if (part != uri) {
			FATAL("Invalid URI. Wanted: `%s`; got: `%s`",
					STR(part), STR(uri));
			return false;
		}
#endif /* HTTP_HELPER_CHECK_URI_AUTH */
		string got = response["parameters"]["response"];
		string wanted = ComputeResponseMD5(username, password, realmName, method,
				uri, nonce);

		return wanted == got;
	}
//	else {
//		string username = response["parameters"]["username"];
//		return realm["users"][username] == response["parameters"]["password"];
//	}
	return true;
}

bool HTTPAuthHelper::ParseAuthLine(string challenge,
		Variant &result, bool isResponse) {
	//1. get the auth type: Digest or Basic
	if ((challenge.find("Digest ") != 0)
			&& (challenge.find("digest ") != 0)
			&& (challenge.find("Basic ") != 0)
			&& (challenge.find("basic ") != 0)) {
		FATAL("Invalid authentication type: %s", STR(challenge));
		return false;
	}
	if ((challenge.find("Digest ") == 0)
			|| (challenge.find("digest ") == 0)) {
		result["method"] = "Digest";
		challenge = challenge.substr(7);
	} else {
		result["method"] = "Basic";
		challenge = challenge.substr(6);
	}

	if ((result["method"] == "Basic") && isResponse) {
	//	string usernamePassword = unb64(challenge);
	//	string::size_type pos = usernamePassword.find(":");
	//	if (pos == string::npos) {
	//		FATAL("Invalid request");
	//		return false;
	//	}
	//	string username = usernamePassword.substr(0, pos);
	//	string password = usernamePassword.substr(pos + 1);
	//	result["parameters"]["username"] = username;
	//	result["parameters"]["password"] = password;
		return true;
	}

	uint32_t stage = AUTH_PARSING_STAGE_KEY;
	bool escape = false;
	bool separator = false;
	string key;
	string value;
	result["parameters"].IsArray(false);
	for (uint32_t i = 0; i < challenge.size(); i++) {
		char c = challenge[i];
		switch (stage) {
			case AUTH_PARSING_STAGE_KEY:
			{
				if (isalpha(c) == 0) {
					if (c != '=') {
						FATAL("Invalid character detected");
						return false;
					}
					stage = AUTH_PARSING_STAGE_BEGIN_VALUE;
				} else {
					key += c;
				}
				break;
			}
			case AUTH_PARSING_STAGE_BEGIN_VALUE:
			{
				if (c != '"') {
					FATAL("Invalid character detected");
					return false;
				}
				stage = AUTH_PARSING_STAGE_VALUE;
				break;
			}
			case AUTH_PARSING_STAGE_VALUE:
			{
				if (escape) {
					if (c != '"') {
						FATAL("Invalid character detected");
						return false;
					}
					value += c;
					escape = false;
					break;
				}
				if (c == '\\') {
					escape = true;
					break;
				}
				if (c == '"') {
					//FINEST("Insert %s:%s", STR(key), STR(value));
					result["parameters"][lowerCase(key)] = value;
					key = "";
					value = "";
					stage = AUTH_PARSING_STAGE_SEPARATOR;
					break;
				}
				value += c;
				break;
			}
			case AUTH_PARSING_STAGE_SEPARATOR:
			{
				if (separator) {
					if (c == ' ')
						break;
					separator = false;
					i--;
					stage = AUTH_PARSING_STAGE_KEY;
					break;
				}
				if (c == ',') {
					separator = true;
					break;
				} else if (c != ' ') {
					FATAL("Invalid character detected");
					return false;
				}
				break;
			}
			default:
			{
				FATAL("Invalid stage: %"PRIu32, stage);
				return false;
			}
		}
	}

	return true;
}

bool HTTPAuthHelper::ValidateChallenge(Variant &challenge) {
	if (challenge["method"] == "Digest") {
		Variant &parameters = challenge["parameters"];
		if ((parameters.HasKey("domain"))
				|| (parameters.HasKey("digest-opaque"))
				|| (parameters.HasKey("stale"))) {
			FATAL("Invalid challenge:\n%s", STR(challenge.ToString()));
			return false;
		}
		string algorithm = "";
		if (parameters.HasKey("algorithm"))
			algorithm = lowerCase((string) parameters["algorithm"]);
		else
			algorithm = "md5";
		if (algorithm != "md5") {
			FATAL("Invalid challenge:\n%s", STR(challenge.ToString()));
			return false;
		}
		parameters["algorithm"] = algorithm;

		if ((!parameters.HasKeyChain(V_STRING, false, 1, "realm"))
				|| (!parameters.HasKeyChain(V_STRING, false, 1, "nonce"))) {
			FATAL("Invalid challenge:\n%s", STR(challenge.ToString()));
			return false;
		}
	} else {
		return true;
	}

	return true;
}


//bool HTTPAuthHelper::ValidateRTSPResponse(string& accessToken) {
//
//  if (MAP_HAS1(_mAuthTokens, tokens[1])) {
//    return true;
//  }
//  return false;
//}

bool HTTPAuthHelper::ValidateResponse(Variant &response) {
	Variant &parameters = response["parameters"];

	if (response["method"] == "Digest") {
		string algorithm = "";
		if (parameters.HasKey("algorithm"))
			algorithm = lowerCase((string) parameters["algorithm"]);
		else
			algorithm = "md5";
		if (algorithm != "md5") {
			FATAL("Invalid response:\n%s", STR(response.ToString()));
			return false;
		}
		parameters["algorithm"] = algorithm;

		if ((!parameters.HasKeyChain(V_STRING, true, 1, "nonce"))
				|| (parameters["nonce"] == "")
				|| (!parameters.HasKeyChain(V_STRING, true, 1, "realm"))
				|| (parameters["realm"] == "")
				|| (!parameters.HasKeyChain(V_STRING, true, 1, "response"))
				|| (parameters["response"] == "")
				|| (!parameters.HasKeyChain(V_STRING, true, 1, "uri"))
				|| (parameters["uri"] == "")
				|| (!parameters.HasKeyChain(V_STRING, true, 1, "username"))
				|| (parameters["username"] == "")) {
			FATAL("Invalid response:\n%s", STR(response.ToString()));
			return false;
		}

		return true;
	} else {
		if ((!parameters.HasKeyChain(V_STRING, true, 1, "username"))
				|| (parameters["username"] == "")
				|| (!parameters.HasKeyChain(V_STRING, true, 1, "password"))
				|| (parameters["password"] == "")) {
			FATAL("Invalid response:\n%s", STR(response.ToString()));
			return false;
		}

		return true;
	}
}

bool HTTPAuthHelper::GetAuthorizationHeaderBasic(Variant &result) {
	string username = result["username"];
	string password = result["password"];
	string value = b64(username + ":" + password);
	result["authorizationHeader"]["raw"] = "Basic " + value;

	result["authorizationHeader"]["method"] = "Basic";
	result["authorizationHeader"]["parameters"]["value"] = value;

	return true;
}

bool HTTPAuthHelper::GetAuthorizationHeaderDigest(Variant &result) {
	if (!ValidateChallenge(result["authLine"])) {
		FATAL("Invalid server challenge");
		return false;
	}
	Variant &parameters = result["authLine"]["parameters"];

	string username = result["username"];
	string password = result["password"];
	string uri = result["uri"];
	string method = result["method"];
	string realm = parameters["realm"];
	string nonce = parameters["nonce"];

	string response = ComputeResponseMD5(username, password, realm, method, uri,
			nonce);

	result["authorizationHeader"]["raw"] =
			"Digest username=\"" + username + "\", "
			+ "realm=\"" + realm + "\", "
			+ "nonce=\"" + nonce + "\", "
			+ "uri=\"" + uri + "\", "
			+ "response=\"" + response + "\""
			;
	result["authorizationHeader"]["method"] = "Digest";
	result["authorizationHeader"]["parameters"]["username"] = username;
	result["authorizationHeader"]["parameters"]["realm"] = realm;
	result["authorizationHeader"]["parameters"]["nonce"] = nonce;
	result["authorizationHeader"]["parameters"]["uri"] = uri;
	result["authorizationHeader"]["parameters"]["response"] = response;

	return true;
}

string HTTPAuthHelper::ComputeResponseMD5(string username, string password,
		string realm, string method, string uri, string nonce) {
	string A1 = username + ":" + realm + ":" + password;
	string A2 = method + ":" + uri;
	return md5(md5(A1, true) + ":" + nonce + ":" + md5(A2, true), true);
}

void HTTPAuthHelper::GetSHA1Token(string &token,
                                  const string& k1,
                                  const string& k2,
                                  const string& k3) {
  uint8_t sha1Digest[20];

  SHA1((const unsigned char*)STR(k1+k2+k3), k1.length()+k2.length()+k3.length(),
        sha1Digest);
  token=b64(sha1Digest, 20);
}
