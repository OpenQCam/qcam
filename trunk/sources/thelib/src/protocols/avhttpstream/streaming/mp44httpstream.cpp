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

#ifdef HAS_PROTOCOL_AVHTTPSTREAM

#include "protocols/avhttpstream/streaming/mp44httpstream.h"
#include "protocols/baseprotocol.h"
#include "protocols/avhttpstream/inboundavhttpstreamprotocol.h"

MP44HTTPStream::MP44HTTPStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager,
  string name, uint64_t size)
: BaseOutHTTPStream (pProtocol, pStreamsManager, ST_OUT_HTTP_MP4, name),
  _pAVProtocol(0),
  _fileSize(size)
{
  _pAVProtocol = reinterpret_cast<InboundAVHTTPStreamProtocol *>(_pProtocol);
}


MP44HTTPStream:: ~MP44HTTPStream() {
}

void MP44HTTPStream::SignalAttachedToInStream() {

  //INFO("http mjpg stream attached");
 // _pAVProtocol->SendHttpResponseHeader(206, _fileSize, offset);
#if 0
  IOBuffer *pBuffer = _pAVProtocol->GetProtocolOutputBuffer();
  _pAVProtocol->SetStatusCode(206);
  pBuffer->ReadFromString(format("%s\r\n", STR(_pAVProtocol->GetOutputFirstLine())));
  pBuffer->ReadFromString(HTTP_HEADERS_SERVER": "HTTP_HEADERS_SERVER_US"\r\n");
  pBuffer->ReadFromString(HTTP_HEADERS_X_POWERED_BY": "HTTP_HEADERS_X_POWERED_BY_US"\r\n");
  pBuffer->ReadFromString(HTTP_HEADERS_ACCESS_ORIGIN": *\r\n");

  //FIXME: fix keep-alive issues for rtmp playback
  //pBuffer->ReadFromString("Connection: Close\r\n");
  pBuffer->ReadFromString("Connection: Keep-Alive\r\n");
  pBuffer->ReadFromString("Keep-Alive: timeout=5, max=100\r\n");
  pBuffer->ReadFromString("Accept-Ranges: bytes\r\n");
  pBuffer->ReadFromString(format("Content-Range: bytes 0-%"PRIu64"/%"PRIu64"\r\n", _fileSize-1, _fileSize));
  pBuffer->ReadFromString(HTTP_HEADERS_CONTENT_TYPE": video/mp4\r\n");
  pBuffer->ReadFromString(format("%s: %"PRIu64" \r\n\r\n", HTTP_HEADERS_CONTENT_LENGTH, _fileSize));
  _pAVProtocol->EnqueueForOutbound();
#endif
}
void MP44HTTPStream::SignalDetachedFromInStream() {
}

void MP44HTTPStream::SignalStreamCompleted() {
}

bool MP44HTTPStream::IsCompatibleWithType(uint64_t type) {
  return true;
}

bool MP44HTTPStream::FeedData(uint8_t *pData, uint32_t dataLength, uint32_t processedLength,
      uint32_t totalLength, double absoluteTimestamp, bool isAudio) {

  IOBuffer *pBuffer = _pAVProtocol->GetProtocolOutputBuffer();
  pBuffer->ReadFromBuffer(pData, dataLength);
  return _pAVProtocol->EnqueueForOutbound();
}

#endif
