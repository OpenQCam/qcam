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
#include "protocols/http/inboundhttpwsprotocol.h"
//common
#include "defines.h"
#include "protocols/http/websocket/websocketdefine.h"
#include "application/clientapplicationmanager.h"
#include "protocols/http/basewebsocketappprotocolhandler.h"
#include "streaming/baseoutstream.h"

#define MAXBUFSIZE (1024*1024)

InboundHTTPWSProtocol::InboundHTTPWSProtocol()
: BaseHTTPWSProtocol(PT_INBOUND_HTTP_WSCONNECTION),
  _pWSHandler(NULL),
  _statusCode(0),
  _isAuthed(false) {
}

InboundHTTPWSProtocol::~InboundHTTPWSProtocol() {
}

void InboundHTTPWSProtocol::SetApplication(BaseClientApplication *pApplication) {
  BaseProtocol::SetApplication(pApplication);
  if (pApplication != NULL) {
    _pWSHandler = (BaseWebSocketAppProtocolHandler *) pApplication->GetProtocolHandler(this);
  } else {
    _pWSHandler = NULL;
  }
}

void InboundHTTPWSProtocol::SetStatusCode(uint16_t statusCode) {
  _statusCode = statusCode;
}

bool InboundHTTPWSProtocol::Initialize(Variant& parameters) {
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

string InboundHTTPWSProtocol::GetOutputFirstLine() {
  switch (_statusCode) {
    case 101:
      return "HTTP/1.1 101 Switching Protocols";
    case 200:
      return "HTTP/1.1 200 OK";
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

bool InboundHTTPWSProtocol::ParseFirstLine(string &line, Variant &headers) {

  vector<string> parts;
  split(line, " ", parts);
  if (parts.size() != 3) {
    FATAL("Incorrect first line: %s", STR(line));
    return false;
  }

  if (parts[2] != HTTP_VERSION_1_1) {
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

bool InboundHTTPWSProtocol::Authenticate() {
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


bool InboundHTTPWSProtocol::EnqueueForOutbound()
{
  uint32_t bufferLength = GETAVAILABLEBYTESCOUNT(_outputBuffer);

  if (bufferLength > 0) {
    WARN ("oubound buffer is not empty");
    _outputBuffer.IgnoreAll();
  }

  //3. add or replace X-Powered-By attribute
  _outboundHeaders[HTTP_HEADERS_X_POWERED_BY] = HTTP_HEADERS_X_POWERED_BY_US;

  //5. Get rid of the Content-Length attribute and add it only if necessary
  _outboundHeaders.RemoveKey(HTTP_HEADERS_CONTENT_LENGTH);
  //if (bufferLength > 0) {
  //  _outboundHeaders[HTTP_HEADERS_CONTENT_LENGTH] = format("%u", bufferLength);
  //}

  //6. Get rid of Transfer-Encoding attribute
  _outboundHeaders.RemoveKey(HTTP_HEADERS_TRANSFER_ENCODING);

  //7. Write the first line of the request/response
  _outputBuffer.ReadFromString(GetOutputFirstLine() + "\r\n");

  //8. Write the headers and the final '\r\n'
  FOR_MAP(_outboundHeaders, string, Variant, i) {
    if (MAP_VAL(i) != V_STRING) {
      FATAL("Invalid HTTP headers:\n%s", STR(_outboundHeaders.ToString()));
      return false;
    }
    _outputBuffer.ReadFromString(format("%s: %s\r\n", STR(MAP_KEY(i)), STR(MAP_VAL(i))));
  }
  _outboundHeaders.Reset();
  _outboundHeaders.IsArray(false);
  _outputBuffer.ReadFromString("\r\n");


  string data = string ((const char*) GETIBPOINTER(_outputBuffer), GETAVAILABLEBYTESCOUNT(_outputBuffer));

  return BaseProtocol::EnqueueForOutbound();
}


/* The function is called when header is parsed */
bool InboundHTTPWSProtocol::SignalDataProcessed() {
  Variant header = _headers[HTTP_HEADERS];
  Variant firstLine = _headers[HTTP_FIRST_LINE];
  string uriString = "http://localhost" + (string)firstLine[HTTP_URL];

  if (!_isAuthed) {
    if(!GetApplication()->IsAuthedSuccess(uriString)){
      return SendHTTPResponse(HC_401_UNAUTHORIZED);
    }else{
      _isAuthed = true;
    }
  }

  if(_wsState != HTTPWS_UNCONNECTED){
    WARN("Seemed websocket is incomplete but handshaking was done!");
    return false;
  }

  if (header.HasKey("Connection") && header.HasKey("Upgrade")
     && header.HasKey("Sec-WebSocket-Key")
     && header.HasKey("Sec-WebSocket-Protocol")) {

     string wsKey, serverWSKey;
     _subProtocolName = (string)header["Sec-WebSocket-Protocol"];
     uint8_t sha1Digest[20];

     wsKey = (string)header["Sec-WebSocket-Key"]+WEBSOCKET_GUID;
     SHA1((const unsigned char *)STR(wsKey), wsKey.length(), sha1Digest);
     serverWSKey = b64(sha1Digest, 20);
     SetOutboundHeader("Sec-WebSocket-Accept", serverWSKey);
     SetOutboundHeader("Sec-WebSocket-Protocol",((string)header["Sec-WebSocket-Protocol"]));
     if(!Send101SwitchProtocol()){
       DEBUG ("websocket response:%s", ((string)header["sec-websocket-protocol"]).c_str());
       _outputBuffer.IgnoreAll();
       GracefullyEnqueueForDelete(this);
     }else{

      URI uri;
      uri.Reset();
      URI::FromString(uriString, false, uri);
      Variant parameters = (Variant) uri.parameters();

      // handshaking done. spawn subprotoocl to deal with dataframe
      _wsState = HTTPWS_CONNECT_ALIVE;
      _pSubProtocol = _pWSHandler->CreateWSSubProtocol(_subProtocolName, parameters, this);
      if(!_pSubProtocol) {
        FATAL("Unable to create subprotocol(%s)", STR(_subProtocolName));
        return false;
      }
      SetNearProtocol(_pSubProtocol);
      //return _pWSProtocolHandler->OnProtocolEvent(static_cast<InboundHTTPWSProtocol*>(this), WS_IN_CONNECTION_SUCCESS , GetWSProtocol());
     }
  }

  return true;
}

bool InboundHTTPWSProtocol::SignalInboundWebSocketClose() {
  EnqueueForDelete();
  return true;
}

#if 0
bool InboundHTTPWSProtocol::SignalInboundWebSocketText(IOBuffer &buffer,
                                                       WebSocketDataFrame &dataFrame) {
  uint32_t offset = dataFrame._payloadOffset;
  uint8_t *pBuf = (uint8_t*)(GETIBPOINTER(buffer)+offset);
  uint32_t length=GETAVAILABLEBYTESCOUNT(buffer)-offset;

  _pWSProtocolHandler->ProcessWebsocketText(this, pBuf, length);
  return true;
}

bool InboundHTTPWSProtocol::SignalInboundWebSocketBinary(IOBuffer &buffer,
                                                         WebSocketDataFrame &dataFrame)
{
  uint32_t offset = dataFrame._payloadOffset;
  uint8_t *pBuf = (uint8_t*)(GETIBPOINTER(buffer)+offset);
  uint32_t length=GETAVAILABLEBYTESCOUNT(buffer)-offset;

  _pWSProtocolHandler->ProcessWebsocketBinary(this, pBuf, length);
  return true;
}
#endif

bool InboundHTTPWSProtocol::SignalInputData(IOBuffer &buffer) {

  if (IsEnqueueForDelete())
    return true;

  if (_wsState == HTTPWS_CONNECT_ALIVE) {
    return ProcessWSDataFrame(buffer);
  }

  if(_wsState == HTTPWS_UNCONNECTED) { //HTTP handshaking
    return BaseHTTPProtocol::SignalInputData(buffer);
  }


  return true;
}

bool InboundHTTPWSProtocol::SendAuthRequired(Variant &auth) {
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



bool InboundHTTPWSProtocol::SendOptionsAccepted() {
  SetStatusCode(200);
  SetOutboundHeader(HTTP_HEADERS_CONTENT_TYPE, "text/html");
  SetOutboundHeader(HTTP_HEADERS_ACCESS_ORIGIN, "*");
  SetOutboundHeader(HTTP_HEADERS_ACCESS_HEADERS, "origin, x-requested-with, x-file-name, cache-control");
  SetOutboundHeader(HTTP_HEADERS_ACCESS_METHODS, "POST, GET, OPTIONS");
  return EnqueueForOutbound();
}

bool InboundHTTPWSProtocol::Send101SwitchProtocol() {
  SetStatusCode(101);
  SetOutboundHeader("Upgrade", "websocket");
  SetOutboundHeader("Connection", "Upgrade");
  return EnqueueForOutbound();
}

bool InboundHTTPWSProtocol::Send400BadRequest() {
  SetStatusCode(400);
  SetOutboundHeader(HTTP_HEADERS_CONNECTION, HTTP_HEADERS_CONNECTION_CLOSE);
  SetOutboundHeader(HTTP_HEADERS_CONTENT_TYPE, "text/html");
  SetOutboundHeader(HTTP_HEADERS_ACCESS_ORIGIN, "*");
  SetOutboundHeader(HTTP_HEADERS_CACHE_CONTROL, "no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0");
  SetOutboundHeader("Pragma", "no-cache");
  SetOutboundHeader("Expires", "Mon, 3 Jan 2000 00:00:00 GMT");
  return EnqueueForOutbound();
}

bool InboundHTTPWSProtocol::SendHTTPResponse(RestHTTPCode statusCode) {
  bool ret=false;
  SetStatusCode(statusCode);
  ret=EnqueueForOutbound();
  GracefullyEnqueueForDelete();
  return ret;
}

bool InboundHTTPWSProtocol::IsBufferFull() {
  if (GETAVAILABLEBYTESCOUNT(_outputBuffer) > MAXBUFSIZE)
    return true;
  else return false;
}

string InboundHTTPWSProtocol::GetHTTPMethod(){
  return _headers[HTTP_FIRST_LINE][HTTP_METHOD];
}

BaseWebSocketAppProtocolHandler* InboundHTTPWSProtocol::GetProtocolHandler() {
  return _pWSHandler;
}

#endif /* HAS_PROTOCOL_HTTP */
