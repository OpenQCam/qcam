
#if (defined(HAS_PROTOCOL_HTTP)&&defined(HAS_PROTOCOL_HTTPWSCONNECTION))
#include "protocols/http/outboundhttpwsprotocol.h"
#include "protocols/http/websocket/websocketdefine.h"
#include "protocols/protocoltypes.h"
#include "protocols/http/basewebsocketappprotocolhandler.h"
#include "application/baseclientapplication.h"
#include "streaming/baseoutstream.h"

OutboundHTTPWSProtocol::OutboundHTTPWSProtocol()
:BaseHTTPWSProtocol(PT_OUTBOUND_HTTP_WSCONNECTION),
 _pWSHandler(NULL) {
}

OutboundHTTPWSProtocol::~OutboundHTTPWSProtocol() {
  if (_pWSHandler != NULL) {
    _pWSHandler->OnProtocolEvent (this, WS_HTTPWSCONNECTION_DTOR);
  }
  _pWSHandler=NULL;
}

void OutboundHTTPWSProtocol::SetUrl (string url) {
  _url=url;
}

string& OutboundHTTPWSProtocol::GetUrl() {
  return _url;
}

BaseWebSocketAppProtocolHandler* OutboundHTTPWSProtocol::GetProtocolHandler() {
  return _pWSHandler;
}

string OutboundHTTPWSProtocol::GetOutputFirstLine() {
	return format("%s %s HTTP/1.1", STR(_method), STR(_document));
}

void OutboundHTTPWSProtocol::SetApplication(BaseClientApplication *pApplication) {
  BaseProtocol::SetApplication(pApplication);
  if (pApplication != NULL) {
    _pWSHandler = (BaseWebSocketAppProtocolHandler *) (pApplication->GetProtocolHandler(PT_OUTBOUND_HTTP_WSCONNECTION));
  }
}

void OutboundHTTPWSProtocol::SetProtocolHandler(string handlerAddr)
{
  _pWSHandler=(BaseWebSocketAppProtocolHandler*)(strtoul(handlerAddr.c_str(), NULL, 16));
}

bool OutboundHTTPWSProtocol::Initialize(Variant &parameters)
{
  //Save the parameters
  GetCustomParameters() = parameters;
  return true;
}

IOBuffer* OutboundHTTPWSProtocol::GetOutputBuffer()
{
  //This is for GracefullyEnqueueForDelete
  //If not NULL, it will not delete the protocol
  if (GETAVAILABLEBYTESCOUNT(_outputBuffer) > 0) {
    return &_outputBuffer;
  }
  return NULL;
}

bool OutboundHTTPWSProtocol::AllowFarProtocol(uint64_t type)
{
  return BaseHTTPProtocol::AllowFarProtocol(type);
}

bool OutboundHTTPWSProtocol::AllowNearProtocol(uint64_t type)
{
  if (type==PT_WEBSOCKET_SUBPROTOCOL) {
    return true;
  }
  return false;
}

bool OutboundHTTPWSProtocol::ParseFirstLine(string &line, Variant &firstLineHeader)
{
  string statusCode;
	vector<string> parts;

	split(line, " ", parts);
	if (parts.size() < 3) {
		FATAL("Incorrect first line: %s", STR(line));
		return false;
	}

	if ((parts[0] != HTTP_VERSION_1_1)
			&& (parts[0] != HTTP_VERSION_1_0)) {
		FATAL("Http version not supported: %s", STR(parts[0]));
		return false;
	}

	if (!isNumeric(parts[1])) {
		FATAL("Invalid HTTP code: %s", STR(parts[1]));
		return false;
	}

	string reason = "";
	for (uint32_t i = 2; i < parts.size(); i++) {
		reason += parts[i];
		if (i != parts.size() - 1)
			reason += " ";
	}

	firstLineHeader[HTTP_VERSION] = parts[0];
	firstLineHeader[HTTP_STATUS_CODE] = parts[1];
	firstLineHeader[HTTP_STATUS_CODE_REASON] = reason;
  statusCode = STR(firstLineHeader[HTTP_STATUS_CODE]);

  if (statusCode.compare("101")) {
    WARN ("connect reject status code:%s", statusCode.c_str());
    _pWSHandler->OnProtocolEvent (this, WS_HTTP_PARSE_STATUS_ERROR);
    SetWSState(HTTPWS_CONNECT_REJECT);
  }

  return true;
}

bool OutboundHTTPWSProtocol::SignalInputData(IOBuffer &buffer) {

  if (IsEnqueueForDelete())
    return true;

  if (GetWSState()==HTTPWS_CONNECT_ALIVE) {
    //DEBUG ("inbound buffer:%s", STR(buffer.ToString()));
    if (!ProcessWSDataFrame(buffer)) {
      INFO ("process ws data frame error");
      return false;
    }
    return true;
  }
  else if (GetWSState() != HTTPWS_CONNECT_REJECT) { //http connection
    if (!BaseHTTPProtocol::SignalInputData(buffer))  {
      return false;
    }
    if (_httpHeaders.HasKey("headers")) {
      Variant wsMeta= _httpHeaders["headers"];
      if (wsMeta.HasKey("Sec-WebSocket-Accept")) {
        string serverWSKey, wsKey=WEBSOCKETCLIENTKEY;
        string responseKey = (string)_httpHeaders["headers"]["Sec-WebSocket-Accept"];
        uint8_t sha1Digest[20];

        wsKey +=  WEBSOCKET_GUID;
        SHA1((const unsigned char *)STR(wsKey), wsKey.length(), sha1Digest);
        serverWSKey = b64(sha1Digest, 20);

        if (responseKey==serverWSKey) {
          URI uri;
          URI::FromString(_url+_document, false, uri);
          Variant parameters = (Variant) uri.parameters();
          WARN ("========= WS connected success: %s  ========", STR(GetWSProtocol()));
          _pSubProtocol=_pWSHandler->CreateWSSubProtocol(_subProtocolName, parameters, this);
          if (!_pSubProtocol) {
            FATAL("Unable to create subprotocol(%s)", STR(_subProtocolName));
            return false;
          }
          //SetNearProtocol(_pSubProtocol);
          SetWSState (HTTPWS_CONNECT_ALIVE);
          _pWSHandler->OnProtocolEvent(this, WS_HANDSHAKE_SUCCESS);
          //Flush queued event to cloud

          _httpHeaders.RemoveAllKeys();
          //server may send token data with same IOBuffer with http header data
          if (GETAVAILABLEBYTESCOUNT(buffer))
            SignalInputData(buffer);
        }
      } //Sec-WebSocket-Accept
    }  //HasKey ("headers");
  } // WS CONNECT LIVE

  //4. consume the buffer
  //buffer.IgnoreAll();

  //5. We are done
  return true;
}

bool OutboundHTTPWSProtocol::IsBufferFull() {
  return false;
  //if (GETAVAILABLEBYTESCOUNT(_outputBuffer) > MAXVIDEOBUFSIZE) {
  //  WARN ("ws outbound buffer full");
  //  return true;
  //}
  //else return false;
}

//override BaseHTTPProtocol::SignalDataProcessed
bool OutboundHTTPWSProtocol::SignalDataProcessed()
{
  //Save httpHeader for WebSocket HandShake key verification
  _httpHeaders.Reset();
  if (TransferCompleted()) {
    _httpHeaders = GetHeaders();
  }
  return true;
}

/* static */
bool OutboundHTTPWSProtocol::OpenWSSConnection(ws_param_t wsParam,
                                               BaseClientApplication *pClientApp,
                                               uint32_t& tcpConnectID)
{
  BaseWebSocketAppProtocolHandler *pHandler = NULL;
  if (pClientApp == NULL) {
    DEBUG ("client application is NULL");
    return false;
  }

  pHandler = reinterpret_cast<BaseWebSocketAppProtocolHandler*>
      (pClientApp->GetProtocolHandler(PT_OUTBOUND_HTTP_WSCONNECTION));

  if (pHandler == NULL) {
    DEBUG ("protocol handler is NULL");
    return false;
  }

  //1. Split the URL into components
  URI uri;
  if (!URI::FromString(wsParam.uri, true, uri)) {
    FATAL("Invalid uri: %s", STR(wsParam.uri));
    return false;
  }

  //2. resolve the protocol chain types
  vector<uint64_t> chain=
      ProtocolFactoryManager::ResolveProtocolChain(CONF_PROTOCOL_OUTBOUND_HTTPWSSCONNECTION);

  //3. Prepare the custom parameters (the payload)
  Variant parameters;
  vector<string> result;
  split(wsParam.uri, "?", result);

  if (result.size() < 2) {
    FATAL ("invalid url:%s", STR(wsParam.uri));
    return false;
  }

  parameters[CONF_WSCONN_PARAM_URI] = uri;
  parameters[CONF_WSCONN_PARAM_IPURI] = result[0];
  //parameters[CONF_WSCONN_PARAM_MAPID] = wsParam.key;
  parameters[CONF_WSCONN_PARAM_SUBPTNAME] = wsParam.name;
  parameters[CONF_WSCONN_PARAM_APPADDR] = numToString<void*>(pClientApp, 0).substr(2,8);

  pHandler->OnProtocolEvent(NULL, WS_EVT_HTTP_CONNECTTING, wsParam.name);

  //4. Start the HTTP request
  if (!TCPConnector<OutboundHTTPWSProtocol>::Connect(uri.ip(), uri.port(), chain,
      parameters)) {
    FATAL("Unable to open connection");
    return false;
  }

  /* used to handler connection failure when protocol is not even created,
     system call *connect* but remote do not response */
  tcpConnectID = (uint32_t)(parameters[CONF_WSCONN_PARAM_TCPCONNID]);

  //5. Done :)
  return true;
}

/* static */
bool OutboundHTTPWSProtocol::OpenWSConnection(ws_param_t wsParam,
                                              BaseClientApplication *pClientApp,
                                              uint32_t& tcpConnectID)
{
  BaseWebSocketAppProtocolHandler *pHandler = NULL;
  if (pClientApp == NULL) {
    DEBUG ("client application is NULL");
    return false;
  }

  pHandler = reinterpret_cast<BaseWebSocketAppProtocolHandler*>
      (pClientApp->GetProtocolHandler(PT_OUTBOUND_HTTP_WSCONNECTION));
  if (pHandler == NULL) {
    DEBUG ("protocol handler is NULL");
    return false;
  }

  //1. Split the URL into components
  URI uri;
  if (!URI::FromString(wsParam.uri, true, uri)) {
    FATAL("Invalid uri: %s", STR(wsParam.uri));
    return false;
  }

  //2. resolve the protocol chain types
  vector<uint64_t> chain = ProtocolFactoryManager::ResolveProtocolChain(CONF_PROTOCOL_OUTBOUND_HTTPWSCONNECTION);

  //3. Prepare the custom parameters (the payload)
  Variant parameters;
  vector<string> result;
  split(wsParam.uri, "?", result);

  if (result.size() < 2) {
    FATAL ("invalid uri:%s", STR(wsParam.uri));
    return false;
  }

  parameters[CONF_WSCONN_PARAM_URI] = uri;
  parameters[CONF_WSCONN_PARAM_IPURI] = result[0];
  //parameters[CONF_WSCONN_PARAM_MAPID] = wsParam.key;
  parameters[CONF_WSCONN_PARAM_SUBPTNAME] = wsParam.name;
  parameters[CONF_WSCONN_PARAM_APPADDR] = numToString<void*>(pClientApp, 0).substr(2,8);

  //4. Start the HTTP request
  if (!TCPConnector<OutboundHTTPWSProtocol>::Connect(uri.ip(), uri.port(), chain,
      parameters)) {
    FATAL("Unable to open connection");
    return false;
  }

  /* used to handler connection failure when protocol is not even created,
     system call *connect* but remote do not response */
  tcpConnectID = (uint32_t)(parameters[CONF_WSCONN_PARAM_TCPCONNID]);
  pHandler->OnProtocolEvent(NULL, WS_EVT_HTTP_CONNECTTING, wsParam.name);

  //5. Done :)
  return true;
}

/* static function */
bool OutboundHTTPWSProtocol::SignalProtocolCreated(BaseProtocol *pProtocol, Variant &parameters) {

  bool ret;

  string clienAddr = parameters[CONF_WSCONN_PARAM_APPADDR];
  BaseClientApplication *pClientApp=(BaseClientApplication*)(strtoul(clienAddr.c_str(), NULL, 16));

  //1. Did we connected?
  if (pProtocol == NULL) {
    FATAL("Connection failed");
    return false;
  }

  OutboundHTTPWSProtocol *wsProtocol = reinterpret_cast<OutboundHTTPWSProtocol *>(pProtocol);

  wsProtocol->SetApplication(pClientApp);
  wsProtocol->BaseWebSocketProtocol::SetWSProtocol(parameters[CONF_WSCONN_PARAM_SUBPTNAME]);
  wsProtocol->SetUrl(parameters[CONF_WSCONN_PARAM_IPURI]);
  //wsProtocol->SetMapId(parameters[CONF_WSCONN_PARAM_MAPID]);

  //We Set ProtocolHandler Before doing handshake
  wsProtocol->GetProtocolHandler()->OnProtocolEvent(wsProtocol, WS_HTTP_PROTOCOL_CREATED, "");
  ret = wsProtocol->DoWSHandshake(parameters);

  return ret;
}

//override, warp http header and send to tcp protocol
bool OutboundHTTPWSProtocol::EnqueueForOutbound()
{
  IOBuffer payloadBuffer;
  bool ret;
  payloadBuffer.ReadFromString("payload");

  SetOutboundHeader(HTTP_HEADERS_HOST, _host);

  IOBuffer *pBuffer = &payloadBuffer;
  uint32_t bufferLength = 0;
  if (pBuffer != NULL) {
    bufferLength = GETAVAILABLEBYTESCOUNT(*pBuffer);
  }

  //3. add or replace X-Powered-By attribute
  _outboundHeaders[HTTP_HEADERS_X_POWERED_BY] = HTTP_HEADERS_X_POWERED_BY_US;


  //x. hack cross-origin problem, no need
  //_outboundHeaders[HTTP_HEADERS_ACCESS_ORIGIN] = "*";

  //4. add or replace the Server attribute
  //if (GetType() == PT_INBOUND_HTTP) {
  //  _outboundHeaders[HTTP_HEADERS_SERVER] = HTTP_HEADERS_SERVER_US;
  //}

  //5. Get rid of the Content-Length attribute and add it only if necessary
  _outboundHeaders.RemoveKey(HTTP_HEADERS_CONTENT_LENGTH);
  if (bufferLength > 0) {
    _outboundHeaders[HTTP_HEADERS_CONTENT_LENGTH] = format("%u", bufferLength);
  }

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

  //9. Write the actual content if necessary
  if (bufferLength > 0) {
    _outputBuffer.ReadFromBuffer(GETIBPOINTER(*pBuffer), GETAVAILABLEBYTESCOUNT(*pBuffer));
    //10. Empty the upper output buffer
    pBuffer->IgnoreAll();
  }

  //string data = string ((const char*) GETIBPOINTER(_outputBuffer), GETAVAILABLEBYTESCOUNT(_outputBuffer));

  //12. Let the request flow further
  ret = BaseProtocol::EnqueueForOutbound();

  return ret;
}

bool OutboundHTTPWSProtocol::EnqueueForPing()
{
  bool ret=false;
  WSHeader headerWS;
  IOBuffer headerBuf;
  WSHEADER(headerWS, true, true, WS_OPCODE_PING, 0x0, 0);

  if (ConstructDataFrameHeader (headerWS, headerBuf)) {
    _outputBuffer.ReadFromInputBuffer(headerBuf, GETAVAILABLEBYTESCOUNT(headerBuf));
    if (BaseProtocol::EnqueueForOutbound()) {
      ret = true;
    }
  }

  return ret;
}

//bool OutboundHTTPWSProtocol::EnqueueForAuthOutbound(string result)
//{
//  IOBuffer headerBuf;
//  WSHeader headerWS;
//
//  WSHEADER(headerWS, true, true, WS_OPCODE_TEXT_FRAME, 0x0, result.length());
//  if (ConstructDataFrameHeader (headerWS, headerBuf)) {
//      _outputBuffer.ReadFromInputBuffer(headerBuf, GETAVAILABLEBYTESCOUNT(headerBuf));
//      _outputBuffer.ReadFromString(result);
//  }
//  else {
//    FATAL ("construct websocket fails");
//    return false;
//  }
//
//  if (!BaseProtocol::EnqueueForOutbound()) {
//    FATAL ("token enqueue for outbound fails");
//    return false;
//  }
//  return true;
//}

bool OutboundHTTPWSProtocol::DoWSHandshake(Variant &parameters)
{
  bool ret=false;

  //1. Ok, we connected. Do the actual HTTP request now. First, get the HTTP protocol
  //OutboundHTTPProtocol *pHTTP = (OutboundHTTPProtocol *) GetFarProtocol();
  // assign subprotocol here

  //Web socket http header
  SetOutboundHeader("Connection", "Upgrade");
  SetOutboundHeader("Sec-WebSocket-Extensions", "Upgrade");
  SetOutboundHeader("Sec-WebSocket-Key", WEBSOCKETCLIENTKEY);
  SetOutboundHeader("Sec-WebSocket-Protocol", GetWSProtocol());
  SetOutboundHeader("Sec-WebSocket-Version", "13");
  SetOutboundHeader("Upgrade", "WebSocket");

  //2. We wish to disconnect after the transfer is complete
  SetDisconnectAfterTransfer(true);

  //3. This is going to be a GET request
  _method = HTTP_METHOD_GET;

  //4. This 2 are mandatory
  _document = (string)parameters["uri"]["fullDocumentPathWithParameters"];
  //DEBUG ("fullDocumentPathWithParameters:%s", ((string)parameters["uri"]["fullDocumentPathWithParameters"]).c_str());
  _host = (string)parameters["uri"]["host"];

  //_outputBuffer.ReadFromString((string) parameters["payload"]);

  //6. Done
  ret = EnqueueForOutbound();
  if (ret)
    SetWSState(HTTPWS_CONNECT_WAIT_RESPONSE);

  return ret;
}

bool OutboundHTTPWSProtocol::SignalInboundWebSocketClose() {
  EnqueueForDelete();
  return true;
}

#if 0
/* TODO-move to subprotocol handler, Signal inbound websocket text */
bool OutboundHTTPWSProtocol::SignalInboundWebSocketText(IOBuffer &buffer, WebSocketDataFrame &dataFrame)
{
  uint32_t offset = dataFrame._payloadOffset;
  uint8_t *pBuf = (uint8_t*)(GETIBPOINTER(buffer)+offset);
  string raw = string((char *) pBuf, dataFrame._header.payloadLength);

  return _pWSHandler->ProcessWebsocketText(this, pBuf, dataFrame._header.payloadLength);
}

/* TODO-move to subprotocol handler, Signal inbound websocket text */
bool OutboundHTTPWSProtocol::SignalInboundWebSocketBinary(IOBuffer &buffer, WebSocketDataFrame &dataFrame)
{
  WARN ("Binary data received");
#if 0
  uint32_t offset = dataFrame._payloadOffset;
  uint8_t *pBuf = (uint8_t*)(GETIBPOINTER(buffer)+offset);

  NYI;
  if (_pInStream)
    _pInStream->FeedData(pBuf, dataFrame._header.payloadLength, 0, 0, 0, false);
  else
    WARN ("[Binary]No input stream is created");
#endif
  return true;
}
#endif


#endif /* HAS_PROTOCOL_HTTP&&HAS_PROTOCOL_HTTPWSCONNECTION */
