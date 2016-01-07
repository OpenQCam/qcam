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

#include "protocols/cli/inboundxmlrestcliprotocol.h"
#include "protocols/cli/baserestcliappprotocolhandler.h"
#include "protocols/http/inboundhttpprotocol.h"
#include "application/baseclientapplication.h"

InboundXMLRestCLIProtocol::InboundXMLRestCLIProtocol()
: BaseProtocol(PT_INBOUND_XMLRESTCLI),
  _pProtocolHandler(0),
  _pHTTP(0),
  _pInputBuffer(0)
{
  _useLengthPadding = false;
  _sessionId = "";
}

InboundXMLRestCLIProtocol::~InboundXMLRestCLIProtocol() {
}

bool InboundXMLRestCLIProtocol::Initialize(Variant &parameters) {
  if (parameters["useLengthPadding"] == V_BOOL) {
    _useLengthPadding = (bool)parameters["useLengthPadding"];
  }
  return true;
}

void InboundXMLRestCLIProtocol::SetApplication(BaseClientApplication *pApplication) {
  BaseProtocol::SetApplication(pApplication);
  if (pApplication != NULL) {
    _pProtocolHandler = (BaseRestCLIAppProtocolHandler *)
        pApplication->GetProtocolHandler(this);
  } else {
    _pProtocolHandler = NULL;
  }
}

bool InboundXMLRestCLIProtocol::AllowFarProtocol(uint64_t type) {
  return (type == PT_INBOUND_HTTP);
}

bool InboundXMLRestCLIProtocol::AllowNearProtocol(uint64_t type) {
  ASSERT("Operation not supported");
  return false;
}

IOBuffer * InboundXMLRestCLIProtocol::GetOutputBuffer() {
  if (GETAVAILABLEBYTESCOUNT(_outputBuffer) != 0)
    return &_outputBuffer;
  return NULL;
}

IOBuffer * InboundXMLRestCLIProtocol::GetInputBuffer() {
  return _pInputBuffer;
}

bool InboundXMLRestCLIProtocol::SignalInputData(int32_t recvAmount) {
  ASSERT("Operation not supported");
  return false;
}


bool InboundXMLRestCLIProtocol::SignalInputData(IOBuffer &buffer) {
  //_message.Reset(false);
  _pHTTP = (InboundHTTPProtocol *) GetFarProtocol();
  if(_pHTTP == NULL) return false;
  //Variant httpHeaders = _pHTTP->GetHeaders();
  //Variant resourceURI = (Variant)httpHeaders[HTTP_FIRST_LINE];

  ////_pHTTP->SetOutboundHeader(HTTP_HEADERS_CONNECTION, HTTP_HEADERS_CONNECTION_CLOSE);
  ////_pHTTP->SetOutboundHeader(HTTP_HEADERS_CONTENT_TYPE, "text/html");
  ////_pHTTP->SetOutboundHeader(HTTP_HEADERS_ACCESS_ORIGIN, "*");
  ////_pHTTP->SetOutboundHeader(HTTP_HEADERS_CACHE_CONTROL, "no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0");
  ////_pHTTP->SetOutboundHeader("Pragma", "no-cache");
  ////_pHTTP->SetOutboundHeader("Expires", "Mon, 3 Jan 2000 00:00:00 GMT");


  //if(resourceURI.HasKey(HTTP_URL)){
  //  if(!ParseURI((string)resourceURI[HTTP_URL]))
  //    return _pHTTP->Send400BadRequest();
  //}else{
  //  FATAL("Unable to find requst %s", STR(httpHeaders.ToString()));
  //  return false;
  //}


  // pass http header for futher use
  //_message["HTTP"]["HTTP_HEADER"] = (Variant)httpHeaders;
  //_pInputBuffer = &buffer;

  // HTTP Method: GET
  //buffer.IgnoreAll();

  _pInputBuffer = &buffer;

  if (_pProtocolHandler == NULL) {
    FATAL("No handler available yet");
    return false;
  }
  Variant headers = _pHTTP->GetHeaders();
  return _pProtocolHandler->ProcessMessage((BaseProtocol *)this, headers);
}


bool InboundXMLRestCLIProtocol::SendMessage(uint16_t httpStatusCode, Variant &message) {
  string xml;
#if (defined(HAS_TINYXML))
  if (!message.SerializeToIControlXml(xml, true)) {
    FATAL("Unable to serialize to XML");
    return false;
  }
  xml += "\r\n\r\n";
  if (_useLengthPadding) {
    uint32_t size = EHTONL((uint32_t) xml.length());
    _outputBuffer.ReadFromBuffer((uint8_t *) & size, 4);
  }
  _outputBuffer.ReadFromString(xml);
  _pHTTP->SetStatusCode(httpStatusCode);
  _pHTTP->SetOutboundHeader(HTTP_HEADERS_CONTENT_TYPE, "application/xml");
  _pHTTP->SetOutboundHeader(HTTP_HEADERS_ACCESS_ORIGIN, "*");
  return EnqueueForOutbound();
#else
  FATAL("Unable to serialize to XML, must enable tinyXML");
  return false;
#endif
}

bool InboundXMLRestCLIProtocol::SendXmlpMessage(uint16_t httpStatusCode, string callbackFunction, Variant &message) {
  string xml;
#if (defined(HAS_TINYXML))
  if (!message.SerializeToIControlXml(xml, true)) {
    FATAL("Unable to serialize to XML");
    return false;
  }
  _outputBuffer.ReadFromString(callbackFunction + "(" + xml + ")");
  _pHTTP->SetStatusCode(httpStatusCode);
  _pHTTP->SetOutboundHeader(HTTP_HEADERS_CONTENT_TYPE, "application/javascript");
  _pHTTP->SetOutboundHeader(HTTP_HEADERS_ACCESS_ORIGIN, "*");
  return EnqueueForOutbound();
#else
  FATAL("Unable to serialize to XML, must enable tinyXML");
  return false;
#endif
}

string InboundXMLRestCLIProtocol::GetSessionId() {
  return _sessionId;
}

// Parse Generic
//bool InboundXMLRestCLIProtocol::ParseURI(const string &reqest) {
//
//  string uriString = "https://localhost" + reqest;
//
//  if (!URI::FromString(uriString, false, _uri)) {
//    FATAL("Invalid reqest %s", STR(reqest));
//    return false;
//  }
//
//  return true;
//
//}


#endif /* HAS_PROTOCOL_CLI */
