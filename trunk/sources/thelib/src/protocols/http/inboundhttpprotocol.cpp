// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    :
// // Version :
// // Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//

#ifdef HAS_PROTOCOL_HTTP
#include "protocols/http/inboundhttpprotocol.h"
#include "application/clientapplicationmanager.h"


InboundHTTPProtocol::InboundHTTPProtocol()
: BaseHTTPProtocol(PT_INBOUND_HTTP),
  _statusCode(200),
  _isAuthed(true)
{
}

InboundHTTPProtocol::InboundHTTPProtocol(uint64_t protocolType)
: BaseHTTPProtocol(protocolType) {
  _statusCode = 200;
}

InboundHTTPProtocol::~InboundHTTPProtocol() {
}

void InboundHTTPProtocol::SetStatusCode(uint16_t statusCode) {
  _statusCode = statusCode;
}

bool InboundHTTPProtocol::Initialize(Variant& parameters) {
  if (parameters["hasAuth"] == V_BOOL) {
    _hasAuth = (bool)parameters["hasAuth"];
    return BaseHTTPProtocol::Initialize(parameters);
  }
  if ((parameters.HasKey("auth"))
      && (parameters["auth"] == V_MAP)
      && (parameters["auth"].HasKey("realms"))
      && (parameters["auth"]["realms"] == V_MAP)
      && (parameters["auth"]["realms"].MapSize() != 0)
      && (parameters["auth"].HasKey("uris"))
      && (parameters["auth"]["uris"] == V_MAP)
      && (parameters["auth"]["uris"].MapSize() != 0)
      ) {

    Variant optimizedRealms;
    optimizedRealms.IsArray(false);

    FOR_MAP(parameters["auth"]["realms"], string, Variant, i) {
      Variant realm = MAP_VAL(i);
      if ((!realm.HasKey("method"))
          || (realm["method"] != V_STRING)
          || (realm["method"] != "digest")
          ) {
        FATAL("Invalid realm:\n%s", STR(realm.ToString()));
        return false;
      }
      if ((!realm.HasKey("name"))
          || (realm["name"] != V_STRING)
          || (optimizedRealms.HasKey(realm["name"]))
          ) {
        FATAL("Duplicate realm:\n%s", STR(realm.ToString()));
        return false;
      }
      if ((!realm.HasKey("credentials"))
          || (realm["credentials"] != V_MAP)
          || (realm["credentials"].MapSize() == 0)
          ) {
        FATAL("Credentials list is empty:\n%s", STR(realm.ToString()));
        return false;
      }

      FOR_MAP(realm["credentials"], string, Variant, j) {
        if (MAP_VAL(j) != V_STRING) {
          FATAL("Invalid credentials in realm:\n%s", STR(realm.ToString()));
          return false;
        }
      }
      optimizedRealms[realm["name"]] = realm;
    }
    parameters["auth"]["realms"] = optimizedRealms;

    Variant optimizedUris;
    optimizedUris.IsArray(false);

    FOR_MAP(parameters["auth"]["uris"], string, Variant, i) {
      Variant uri = MAP_VAL(i);
      if ((!uri.HasKey("uri"))
          || (uri["uri"] != V_STRING)
          || (optimizedUris.HasKey(uri["uri"]))
          ) {
        FATAL("Invalid or duplicate uri value:\n%s", STR(uri.ToString()));
        return false;
      }
      if ((!uri.HasKey("realm"))
          || (uri["realm"] != V_STRING)
          || (!optimizedRealms.HasKey(uri["realm"]))
          ) {
        FATAL("Invalid realm or realm not found:\n%s", STR(uri.ToString()));
        return false;
      }
      optimizedUris[uri["uri"]] = uri["realm"];
    }
    parameters["auth"]["uris"] = optimizedUris;
    _hasAuth = true;
  }
  parameters["hasAuth"] = (bool)_hasAuth;
  //INFO("%s", STR(parameters.ToString()));
  return BaseHTTPProtocol::Initialize(parameters);
}

string InboundHTTPProtocol::GetOutputFirstLine() {
  switch (_statusCode) {
    case 101:
      return "HTTP/1.1 101 Switching Protocols";
    case 200:
      return "HTTP/1.1 200 OK";
    case 202:
      return "HTTP/1.1 202 Accepted";
    case 204:
      return "HTTP/1.1 204 No Content";
    case 206:
      return "HTTP/1.1 206 Partial Content";
    case 400:
      return "HTTP/1.1 400 Bad Request";
    case 401:
      return "HTTP/1.1 401 Unauthorized";
    case 403:
      return "HTTP/1.1 403 Forbidden";
    case 404:
      return "HTTP/1.1 404 Not Found";
    case 500:
      return "HTTP/1.1 500 Internal Server Error";
    default:
      return format("HTTP/1.1 %hu Unknwon", _statusCode);
  }
}

bool InboundHTTPProtocol::ParseFirstLine(string &line, Variant &headers) {

  vector<string> parts;
  split(line, " ", parts);
  if (parts.size() != 3) {
    FATAL("Incorrect first line: %s", STR(line));
    return false;
  }

  if (parts[2] != HTTP_VERSION_1_1 && parts[2] != HTTP_VERSION_1_0) {
    FATAL("Http version not supported: %s", STR(parts[2]));
    return false;
  }

  if ((parts[0] != HTTP_METHOD_GET) && (parts[0] != HTTP_METHOD_POST) && (parts[0] != HTTP_METHOD_OPTIONS)) {
    FATAL("Http method not supported: %s", STR(parts[0]));
    return false;
  }

  headers[HTTP_METHOD] = parts[0];
  headers[HTTP_URL] = parts[1];
  headers[HTTP_VERSION] = parts[2];

  return true;
}

bool InboundHTTPProtocol::Authenticate() {
  _continueAfterParseHeaders = true;
  if (!_hasAuth)
    return true;

  //1. Get the auth node
  BaseProtocol *pProtocol = GetNearEndpoint();
  if (pProtocol == NULL) {
    FATAL("No near endpoint");
    return false;
  }
  Variant &auth = pProtocol->GetCustomParameters()["auth"];

  //2. Is realm/uri combination into authentication mode?

  //if (!auth["uris"].HasKey(_headers[HTTP_FIRST_LINE][HTTP_URL])) {
  //  return true;
  //}

  //3. Do we have Authorization header in the request?
  if (!_headers[HTTP_HEADERS].HasKey(HTTP_HEADERS_AUTORIZATION, false)) {
    return SendAuthRequired(auth);
  }
  string authorization = _headers[HTTP_HEADERS][HTTP_HEADERS_AUTORIZATION];
  if (authorization.length() < 7) {
    FATAL("Invalid authorization header");
    return SendAuthRequired(auth);
  }
  authorization = authorization.substr(7);
  map<string, string> authMap = mapping(authorization, ",", "=", true);

  FOR_MAP(authMap, string, string, i) {
    replace(MAP_VAL(i), "\"", "");
  }

  string pass = "";
  if (!auth["realms"].HasKey(authMap["realm"])) {
    FATAL("Invalid authorization header");
    return SendAuthRequired(auth);
  }
  if (!auth["realms"][authMap["realm"]]["credentials"].HasKey(authMap["username"])) {
    FATAL("Invalid authorization header");
    return SendAuthRequired(auth);
  }
  pass = (string) auth["realms"][(string) authMap["realm"]]["credentials"][(string) authMap["username"]];

  string wanted = md5(
      md5(authMap["username"] + ":" + authMap["realm"] + ":" + pass, true)
      + ":" + authMap["nonce"] + ":" + authMap["nc"] + ":" + authMap["cnonce"] + ":" + authMap["qop"] + ":"
      + md5((string) _headers[HTTP_FIRST_LINE][HTTP_METHOD] + ":" + authMap["uri"], true), true);
  if (wanted != authMap["response"]) {
    FATAL("Invalid authorization header");
    return SendAuthRequired(auth);
  }

  return true;
}

bool InboundHTTPProtocol::SendAuthRequired(Variant &auth) {
  SetStatusCode(401);
  string wwwAuth = format("Digest realm=\"%s\",qop=\"auth\",nonce=\"%s\",opaque=\"%s\"",
      //STR(auth["uris"][_headers[HTTP_FIRST_LINE][HTTP_URL]]),
      STR(auth["realms"]["QCAM"]["name"]),
      STR(md5(generateRandomString(8), true)),
      STR(md5(generateRandomString(8), true)));
  SetOutboundHeader(HTTP_HEADERS_ACCESS_ORIGIN, "*");
  SetOutboundHeader(HTTP_HEADERS_WWWAUTHENTICATE, wwwAuth);
  _continueAfterParseHeaders = false;
  EnqueueForOutbound();
  GracefullyEnqueueForDelete();
  return true;
}



bool InboundHTTPProtocol::SendOptionsAccepted() {
  SetStatusCode(200);
  SetOutboundHeader(HTTP_HEADERS_CONTENT_TYPE, "text/html");
  SetOutboundHeader(HTTP_HEADERS_ACCESS_ORIGIN, "*");
  SetOutboundHeader(HTTP_HEADERS_ACCESS_HEADERS, "origin, x-requested-with, x-file-name, cache-control");
  SetOutboundHeader(HTTP_HEADERS_ACCESS_METHODS, "POST, GET, OPTIONS");
  return EnqueueForOutbound();
}

bool InboundHTTPProtocol::Send400BadRequest() {
  SetStatusCode(400);
  SetOutboundHeader(HTTP_HEADERS_CONNECTION, HTTP_HEADERS_CONNECTION_CLOSE);
  SetOutboundHeader(HTTP_HEADERS_CONTENT_TYPE, "text/html");
  SetOutboundHeader(HTTP_HEADERS_ACCESS_ORIGIN, "*");
  SetOutboundHeader(HTTP_HEADERS_CACHE_CONTROL, "no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0");
  SetOutboundHeader("Pragma", "no-cache");
  SetOutboundHeader("Expires", "Mon, 3 Jan 2000 00:00:00 GMT");
  return EnqueueForOutbound();
}

bool InboundHTTPProtocol::Send403Forbidden() {
  SetStatusCode(403);
  return EnqueueForOutbound();
}

bool InboundHTTPProtocol::Send404NotFound() {
  SetStatusCode(404);
  return EnqueueForOutbound();
}

string InboundHTTPProtocol::GetHTTPMethod(){
  return _headers[HTTP_FIRST_LINE][HTTP_METHOD];
}
#endif /* HAS_PROTOCOL_HTTP */
