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


#include "video/basevideostream.h"

BaseVideoStream::BaseVideoStream(StreamsManager *pStreamsManager,
    uint64_t type, string name)
: BaseInStream(NULL, pStreamsManager, type, name),
  _status(VSS_UNKNOWN)
{
}

BaseVideoStream::~BaseVideoStream()
{
}

VideoStreamStatus BaseVideoStream::GetStatus()
{
  return _status;
}

bool BaseVideoStream::SignalPlay(double &absoluteTimestamp, double &length)
{
  _status = VSS_START;
  return true;
}
bool BaseVideoStream::SignalPause()
{
  _status = VSS_STOP;
  return true;
}
bool BaseVideoStream::SignalResume()
{
  _status = VSS_START;
  return true;
}
bool BaseVideoStream::SignalSeek(double &absoluteTimestamp)
{
  return true;
}
bool BaseVideoStream::SignalStop()
{
  _status = VSS_STOP;
  return true;
}
bool BaseVideoStream::FeedData(uint8_t *pData, uint32_t dataLength,
    uint32_t processedLength, uint32_t totalLength,
    double absoluteTimestamp, bool isAudio)
{
  return true;
}

void BaseVideoStream::ReadyForSend()
{
}

void BaseVideoStream::SignalOutStreamAttached(BaseOutStream *pOutStream)
{
}
void BaseVideoStream::SignalOutStreamDetached(BaseOutStream *pOutStream)
{
}

