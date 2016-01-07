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

#include "video/devicevideostream.h"
#include "video/basevideostream.h"

DeviceVideoStream::DeviceVideoStream(BaseVideoStream *pVideoStream)
: _pVideoStream(pVideoStream)
{
   _pVideoCapability  = new VideoCaptureCapability();
}

DeviceVideoStream::~DeviceVideoStream()
{
  delete _pVideoCapability;
}

uint32_t DeviceVideoStream::GetCarrierId()
{
  return _pVideoStream->GetCarrierId();
}

uint32_t DeviceVideoStream::GetObserverId()
{
  //FIXME(Tim): Need to correct Recardo's modification about observer design
  // or ObserverId will conflict when performing designated notification
  return _pVideoStream->GetObserverId();
}

BaseVideoStream* DeviceVideoStream::GetVideoStream()
{
  return _pVideoStream;
}

uint32_t DeviceVideoStream::GetVideoStreamId()
{
  return _pVideoStream->GetUniqueId();
}

string DeviceVideoStream::GetVideoStreamName()
{
  return _pVideoStream->GetName();
}

VideoCaptureCapability* DeviceVideoStream::GetCapability()
{
  return _pVideoCapability;
}

VideoConfig* DeviceVideoStream::GetConfig()
{
  return &_config;
}

void DeviceVideoStream::PrintCurrentConfig()
{
  INFO("Current stream is : %s", STR(GetVideoStreamName()));
  INFO("width : %d", _config.width);
  INFO("height : %d", _config.height);
  INFO("bitrate : %d", _config.bitrate);
  INFO("fps : %d", _config.fps);
  INFO("gop : %d", _config.gop);
}
