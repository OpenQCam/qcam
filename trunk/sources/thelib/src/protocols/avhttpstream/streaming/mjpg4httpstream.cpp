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

#include "protocols/avhttpstream/streaming/mjpg4httpstream.h"
#include "protocols/baseprotocol.h"
#include "protocols/avhttpstream/inboundavhttpstreamprotocol.h"
#include "system/eventdefine.h"
#include "streaming/streamsmanager.h"
#include "application/baseclientapplication.h"


#define MAX_OUTPUT_BUFFER_SIZE (256*1024*2) //256k*2

MJPG4HTTPStream::MJPG4HTTPStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager,
  string name)
: BaseOutHTTPStream (pProtocol, pStreamsManager, ST_OUT_HTTP_MJPG, name),
  _pAVProtocol(0),
  _frameCounter(0),
  _frameRate(30),
  _interval(1),
  _isAttached(false)
//  _httpAUTH(HTTP_UNAUTHED)
{
  _pAVProtocol = reinterpret_cast<InboundAVHTTPStreamProtocol *>(_pProtocol);
}


MJPG4HTTPStream:: ~MJPG4HTTPStream() {
  if (_isAttached) {
    const_cast<BaseClientApplication*>
        (_pStreamsManager->GetApplication())->OnNotifyEvent(CLOUD_MSG_EVENT,
                                                            EVT_IN_MJPG_DISCONNECTED,
                                                            0);
    _isAttached=false;
  }
}

//void MJPG4HTTPStream::SetAuthState(HTTP_AUTH_STATE authState) {
//  _httpAUTH=authState;
//}

void MJPG4HTTPStream::SignalAttachedToInStream() {
  IOBuffer *pBuffer = _pAVProtocol->GetProtocolOutputBuffer();

  DEBUG ("mjpg instream attached");
    _pAVProtocol->SetStatusCode(200);
    pBuffer->ReadFromString(format("%s\r\n", STR(_pAVProtocol->GetOutputFirstLine())));
    pBuffer->ReadFromString(HTTP_HEADERS_SERVER": "HTTP_HEADERS_SERVER_US"\r\n");
    pBuffer->ReadFromString(HTTP_HEADERS_X_POWERED_BY": "HTTP_HEADERS_X_POWERED_BY_US"\r\n");
    pBuffer->ReadFromString("Connection: close\r\n");
    pBuffer->ReadFromString("Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n");
    pBuffer->ReadFromString("Pragma: no-cache\r\n");
    pBuffer->ReadFromString("Expires: Mon, 3 Jan 2000 00:00:00 GMT\r\n");
    pBuffer->ReadFromString(HTTP_HEADERS_ACCESS_ORIGIN": *\r\n");
    pBuffer->ReadFromString(HTTP_HEADERS_CONTENT_TYPE": multipart/x-mixed-replace;boundary=cameramjpgboundary\r\n");
    pBuffer->ReadFromString("\r\n");
    pBuffer->ReadFromString("--cameramjpgboundary\r\n");
    _pAVProtocol->EnqueueForOutbound();

    const_cast<BaseClientApplication*>
        (_pStreamsManager->GetApplication())->OnNotifyEvent(CLOUD_MSG_EVENT,
                                                            EVT_IN_MJPG_CONNECTED,
                                                            0);
    _isAttached=true;
}

void MJPG4HTTPStream::SignalDetachedFromInStream() {
  Variant data;

  if (_isAttached) {
    const_cast<BaseClientApplication*>
        (_pStreamsManager->GetApplication())->OnNotifyEvent(CLOUD_MSG_EVENT,
                                                            EVT_IN_MJPG_DISCONNECTED,
                                                            0);
    _isAttached=false;
  }
}

void MJPG4HTTPStream::SignalStreamCompleted() {
}

bool MJPG4HTTPStream::IsCompatibleWithType(uint64_t type) {
  return true;
}

bool MJPG4HTTPStream::FeedData(uint8_t *pData, uint32_t dataLength, uint32_t processedLength,
      uint32_t totalLength, double absoluteTimestamp, bool isAudio) {

//  if (_httpAUTH==HTTP_AUTH_SUCCESS) {
    _frameCounter++;
    IOBuffer *pBuffer = _pAVProtocol->GetProtocolOutputBuffer();
    if(GETAVAILABLEBYTESCOUNT(*pBuffer) > MAX_OUTPUT_BUFFER_SIZE){
      WARN("Congestion! start to drop frame now %d", GETAVAILABLEBYTESCOUNT(*pBuffer));
      pBuffer->IgnoreAll();
      _interval++;
      INFO("interval %d", _interval);
      return true;
    }

    // try to increase frame rate every 10secs
    if((_frameCounter%300) == 0 && (_interval > 1)){
      _interval--;
    }

    // skip frame
    if((_frameCounter%_interval) != 0){
      return true;
    }

    pBuffer->ReadFromString("Content-Type: image/jpeg\r\n");
    pBuffer->ReadFromString(format("Content-Length: %d\r\n", dataLength));
    pBuffer->ReadFromString("\r\n");
    pBuffer->ReadFromBuffer(pData, dataLength);
    pBuffer->ReadFromString("\r\n--cameramjpgboundary\r\n");
    return _pAVProtocol->EnqueueForOutbound();

//  }
//  return _pAVProtocol->EnqueueForOutbound();
}

void MJPG4HTTPStream::SetFrameRate(uint32_t fps) {
  if( (fps > 30) || (fps == 0)){
    return;
  }
  _frameRate = fps;
  _interval = 30/_frameRate;
  INFO("fps = %d, interval=%d", _frameRate, _interval);
}

uint32_t MJPG4HTTPStream::GetFrameRate() {
  return _frameRate;
}


void MJPG4HTTPStream::OnDeviceMessage(DeviceMessage &msg) {
}
/*
void MJPG4HTTPStream::OnNotifyMessage(NOTIFICATION_MSG_TYPE type, string message) {

  INFO("%s", STR(message));
}
*/

#endif
