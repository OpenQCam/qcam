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

#include "protocols/avhttpstream/streaming/snapshot4httpstream.h"
#include "protocols/baseprotocol.h"
#include "protocols/avhttpstream/inboundavhttpstreamprotocol.h"

Snapshot4HTTPStream::Snapshot4HTTPStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager,
  string name)
: BaseOutHTTPStream (pProtocol, pStreamsManager, ST_OUT_HTTP_SNAPSHOT, name),
  _pAVProtocol(0)
  ,_isSent(false)
{
  _pAVProtocol = reinterpret_cast<InboundAVHTTPStreamProtocol *>(_pProtocol);
  //_pCamera = reinterpret_cast<QICHWCamera *>(HardwareManager::GetHardwareInstance(HT_CAMERA));
  //_pCamera->RegisterMessageObserverCallback(this);
}


Snapshot4HTTPStream::~Snapshot4HTTPStream() {
}

void Snapshot4HTTPStream::SignalAttachedToInStream() {

  IOBuffer *pBuffer = _pAVProtocol->GetProtocolOutputBuffer();
  _pAVProtocol->SetStatusCode(200);
  pBuffer->ReadFromString(format("%s\r\n", STR(_pAVProtocol->GetOutputFirstLine())));
  pBuffer->ReadFromString(HTTP_HEADERS_SERVER": "HTTP_HEADERS_SERVER_US"\r\n");
  pBuffer->ReadFromString(HTTP_HEADERS_X_POWERED_BY": "HTTP_HEADERS_X_POWERED_BY_US"\r\n");
  pBuffer->ReadFromString("Connection: close\r\n");
  pBuffer->ReadFromString("Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n");
  pBuffer->ReadFromString("Pragma: no-cache\r\n");
  pBuffer->ReadFromString("Expires: Mon, 3 Jan 2000 00:00:00 GMT\r\n");
  pBuffer->ReadFromString(HTTP_HEADERS_ACCESS_ORIGIN": *\r\n");

}
void Snapshot4HTTPStream::SignalDetachedFromInStream() {
}

void Snapshot4HTTPStream::SignalStreamCompleted() {
}

bool Snapshot4HTTPStream::IsCompatibleWithType(uint64_t type) {
  return true;
}

bool Snapshot4HTTPStream::FeedData(uint8_t *pData, uint32_t dataLength, uint32_t processedLength,
      uint32_t totalLength, double absoluteTimestamp, bool isAudio) {

  if(_isSent) return true;

  IOBuffer *pBuffer = _pAVProtocol->GetProtocolOutputBuffer();

  pBuffer->ReadFromString("Content-Type: image/jpeg\r\n");
  pBuffer->ReadFromString(format("Content-Length: %d\r\n\r\n", dataLength));
  pBuffer->ReadFromBuffer(pData, dataLength);

  _isSent = true;
  return _pAVProtocol->EnqueueForOutbound();

}

void Snapshot4HTTPStream::OnDeviceMessage(DeviceMessage &msg) {
}
/*
void Snapshot4HTTPStream::OnNotifyMessage(NOTIFICATION_MSG_TYPE type, string message) {

  INFO("%s", STR(message));
}
*/

#endif
