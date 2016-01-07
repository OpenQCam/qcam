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
 * ==================================================================
 */
#ifndef _DEVICEVIDEOSTREAM_H
#define _DEVICEVIDEOSTREAM_H

#include "videodefine.h"
#include "qic/videocapturecapability.h"

class BaseVideoStream;
class VideoCaptureCapability;

class DeviceVideoStream {
  private:
    BaseVideoStream *_pVideoStream;
    VideoCaptureCapability* _pVideoCapability;
    VideoConfig _config;
  public:
    DeviceVideoStream(BaseVideoStream *pVideoStream);
    ~DeviceVideoStream();

    uint32_t GetCarrierId();
    uint32_t GetObserverId();
    BaseVideoStream* GetVideoStream();
    uint32_t GetVideoStreamId();
    string GetVideoStreamName();

    VideoCaptureCapability* GetCapability();
    VideoConfig* GetConfig();

    void PrintCurrentConfig();
};

#endif /* _DEVICEVIDEOSTREAM_H */

