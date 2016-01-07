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

#ifndef _MJPG4HTTPSTREAM_H
#define _MJPG4HTTPSTREAM_H

#include "protocols/avhttpstream/streaming/baseouthttpstream.h"
#include "video/videodefine.h"
#include "interface/deviceinterface.h"
//#include "qic/qichwcamera.h"


typedef enum _HTTPAUTH_STATE {
  HTTP_UNAUTHED=0,
  HTTP_AUTHING,
  HTTP_AUTH_SUCCESS,
  HTTP_AUTH_FAIL,
} HTTP_AUTH_STATE;

class InboundAVHTTPStreamProtocol;
class MJPG4HTTPStream
: public BaseOutHTTPStream, IDeviceObserver
{
  private:
    InboundAVHTTPStreamProtocol *_pAVProtocol;
    uint32_t _frameCounter;
    uint32_t _frameRate;
    uint32_t _interval;
    bool _isAttached;
//    HTTP_AUTH_STATE _httpAUTH;

  public:
    MJPG4HTTPStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager, string name);
    virtual ~MJPG4HTTPStream();

    virtual void SignalAttachedToInStream();
    virtual void SignalDetachedFromInStream();
    virtual void SignalStreamCompleted();
    virtual bool IsCompatibleWithType(uint64_t type);
    virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
        uint32_t processedLength, uint32_t totalLength,
        double absoluteTimestamp, bool isAudio);
    void SetFrameRate(uint32_t fps);
    uint32_t GetFrameRate();
    void SetAuthState(HTTP_AUTH_STATE authState);
    // DeviceObserver message callback
    virtual void OnDeviceMessage(DeviceMessage &msg);
};

#endif /* _MJPG4HTTPSTREAM_H */

#endif
