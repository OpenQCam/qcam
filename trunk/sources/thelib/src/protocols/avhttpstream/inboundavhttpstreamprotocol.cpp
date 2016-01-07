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

#include "protocols/avhttpstream/inboundavhttpstreamprotocol.h"

#include "application/baseclientapplication.h"
#include "streaming/baseinstream.h"
#include "protocols/avhttpstream/avstreamdefine.h"
#include "protocols/avhttpstream/baseavhttpstreamappprotocolhandler.h"
#include "protocols/avhttpstream/streaming/infile4httpstream.h"
#include "protocols/avhttpstream/streaming/baseouthttpstream.h"

//TODO: a file cache for inboundavhttpstreamprotocol
InboundAVHTTPStreamProtocol::InboundAVHTTPStreamProtocol()
: InboundHTTPProtocol(PT_INBOUND_AV_HTTP_STREAM),
  _pProtocolHandler(0),
  _pOutStream(0),
  _pInStream(0)
{
  _isAuthed=false;
}

InboundAVHTTPStreamProtocol::~InboundAVHTTPStreamProtocol() {
  WARN ("inbound avhttp stream protocol dtor");
  if(_pInStream != NULL) {
    _pInStream->UnLink(_pOutStream);
    delete _pInStream;
  }

  if(_pOutStream != NULL){
    delete _pOutStream;
  }

}

void InboundAVHTTPStreamProtocol::SetApplication(BaseClientApplication *pApplication) {
  BaseProtocol::SetApplication(pApplication);
  if (pApplication != NULL) {
    _pProtocolHandler = (BaseAVHTTPStreamAppProtocolHandler *)pApplication->GetProtocolHandler(this);
  } else {
    _pProtocolHandler = NULL;
  }
}

bool InboundAVHTTPStreamProtocol::SignalDataProcessed() {

  string uriString=(string)_headers[HTTP_FIRST_LINE][HTTP_URL];
  if (!_isAuthed) {
    if (!_headers.HasKeyChain(V_STRING, true, 2, HTTP_FIRST_LINE, HTTP_URL)) {
      return false;
    }

    if (!GetApplication()->IsAuthedSuccess("http://localhost"+uriString)) {
      return false;
    }
    _isAuthed=true;
  }

  if (ParseURL(uriString)) {
    return _pProtocolHandler->ProcessMessage(this, _message);
  }
  return false;
}

IOBuffer * InboundAVHTTPStreamProtocol::GetOutputBuffer() {
  if (GETAVAILABLEBYTESCOUNT(_outputBuffer) > 0) {
    return &_outputBuffer;
  }
  return NULL;
}

IOBuffer* InboundAVHTTPStreamProtocol::GetProtocolOutputBuffer() {
  return &_outputBuffer;
}

bool InboundAVHTTPStreamProtocol::EnqueueForOutbound() {
  return BaseProtocol::EnqueueForOutbound();
}

void InboundAVHTTPStreamProtocol::SendHttpResponseHeader (uint32_t statusCode, uint64_t fileSize, uint64_t offset)
{
  SetStatusCode(statusCode);
  _outputBuffer.ReadFromString(format("%s\r\n", STR(GetOutputFirstLine())));
  _outputBuffer.ReadFromString(HTTP_HEADERS_SERVER": "HTTP_HEADERS_SERVER_US"\r\n");
  _outputBuffer.ReadFromString(HTTP_HEADERS_X_POWERED_BY": "HTTP_HEADERS_X_POWERED_BY_US"\r\n");
  _outputBuffer.ReadFromString(HTTP_HEADERS_ACCESS_ORIGIN": *\r\n");
  _outputBuffer.ReadFromString("Connection: Keep-Alive\r\n");
  _outputBuffer.ReadFromString("Keep-Alive: timeout=10, max=100\r\n");
  _outputBuffer.ReadFromString("Accept-Ranges: bytes\r\n");
  DEBUG ("%lld-%lld/%lld", offset, fileSize-1, fileSize);
  _outputBuffer.ReadFromString(format("Content-Range: bytes %"PRIu64"-%"PRIu64"/%"PRIu64"\r\n", offset, fileSize-1, fileSize));
  _outputBuffer.ReadFromString(HTTP_HEADERS_CONTENT_TYPE": video/mp4\r\n");
  _outputBuffer.ReadFromString(format("%s: %"PRIu64" \r\n\r\n", HTTP_HEADERS_CONTENT_LENGTH, fileSize));

  EnqueueForOutbound();
}

//bool InboundAVHTTPStreamProtocol::SendDataToProtocol(uint8_t *pBuf, uint32_t length) {
//  return true;
//}

bool InboundAVHTTPStreamProtocol::IsRegisteredOutStream(string filePath) {
  if (_pOutStream && !_filePath.compare(filePath)) {
    DEBUG ("out stream registered");
    return true;
  }
  return false;
}


bool InboundAVHTTPStreamProtocol::RePlayStream(uint32_t playoffset) {
  InFile4HTTPStream* pInStream =(reinterpret_cast<InFile4HTTPStream*>( _pOutStream->GetInStream()));
  double ats = 0.0, pts = -1.0;
  if (pInStream) {
    double doffset=(double)playoffset;
    pInStream->SignalSeek(doffset);
    pInStream->SignalPlay(ats, pts);
  }
  else
    return false;
  return true;
}

void InboundAVHTTPStreamProtocol::RegisterOutStream(BaseOutHTTPStream *pStream) {
  if (pStream){
    _pOutStream = pStream;
  }
}

void InboundAVHTTPStreamProtocol::RegisterInStream(BaseInStream *pStream, string filePath) {
  if (pStream) {
    _pInStream = pStream;
    _filePath = filePath;
  }
}

bool InboundAVHTTPStreamProtocol::ParseURL(string url) {

  string uriString = "https://localhost" + url;
  URI uri;

  if (!URI::FromString(lowerCase(uriString), false, uri)) {
    FATAL("Invalid URL %s", STR(uriString));
    return false;
  }
  _message.Reset(false);
  vector<string> elements;
  split(uri.documentPath(), "/", elements);
  if(elements.size() < 3){
    FATAL("Invalid URL %s", STR(uriString));
    return false;
  }
  //Sanity Check
  if (elements[1]=="" || elements[2]=="")
    return false;

  _message[QIC_STREAM_API_VERSION] = elements[1];
  _message[QIC_STREAM_REQUEST] = elements[2];
  _message[QIC_STREAM_NAME] = uri.document();
  _message[QIC_STREAM_PARAMETERS] = uri.parameters();

  return true;
}
