/*
// ==================================================================
// This confidential and proprietary software may be used only as
// authorized by a licensing agreement from Quanta Computer Inc.
//
//            (C) COPYRIGHT Quanta Computer Inc.
//                   ALL RIGHTS RESERVED
//
// ==================================================================
// ------------------------------------------------------------------
// Date    :
// Version :
// Author  : Recardo Cheng (recardo.cheng@quantatw.com)
// ------------------------------------------------------------------
// Purpose :
// ==================================================================
*/


#include "streaming/baseaudiostream.h"

BaseAudioStream::BaseAudioStream(StreamsManager *pStreamsManager, uint64_t type, string name)
: BaseInStream(NULL, pStreamsManager, type, name)
{
}

BaseAudioStream::~BaseAudioStream()
{
  _linkedStreams.clear();
}

bool BaseAudioStream::SignalPlay(double &absoluteTimestamp, double &length)
{
  return true;
}
bool BaseAudioStream::SignalPause()
{
  return true;
}
bool BaseAudioStream::SignalResume()
{
  return true;
}
bool BaseAudioStream::SignalSeek(double &absoluteTimestamp)
{
  return true;
}
bool BaseAudioStream::SignalStop()
{
  return true;
}
bool BaseAudioStream::FeedData(uint8_t *pData, uint32_t dataLength,
                               uint32_t processedLength, uint32_t totalLength,
                               double absoluteTimestamp, bool isAudio)
{
  return true;
}
void BaseAudioStream::ReadyForSend()
{
}

bool BaseAudioStream::IsCompatibleWithType(uint64_t type)
{
  return true;
}

void BaseAudioStream::SignalOutStreamAttached(BaseOutStream *pOutStream)
{
}

void BaseAudioStream::SignalOutStreamDetached(BaseOutStream *pOutStream)
{
}
