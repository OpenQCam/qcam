
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


#include "audio/baseaudioplaystream.h"

BaseAudioPlayStream::BaseAudioPlayStream(StreamsManager *pStreamsManager, uint64_t type, string name)
: BaseOutStream(NULL, pStreamsManager, type, name) {
}

BaseAudioPlayStream::~BaseAudioPlayStream() {
}

bool BaseAudioPlayStream::SignalPlay(double &absoluteTimestamp, double &length)
{
  return true;
}
bool BaseAudioPlayStream::SignalPause()
{
  return true;
}

bool BaseAudioPlayStream::SignalResume() {
  return true;
}

bool BaseAudioPlayStream::SignalSeek(double &absoluteTimestamp) {
  return true;
}

bool BaseAudioPlayStream::SignalStop() {
  return true;
}

bool BaseAudioPlayStream::FeedData(uint8_t *pData, uint32_t dataLength,
                                   uint32_t processedLength, uint32_t totalLength,
                                   double absoluteTimestamp, bool isAudio) {
  return true;
}

void BaseAudioPlayStream::ReadyForSend() {
}

bool BaseAudioPlayStream::IsCompatibleWithType(uint64_t type) {
  return true;
}

void BaseAudioPlayStream::SignalAttachedToInStream() {
  NYI;
}
void BaseAudioPlayStream::SignalDetachedFromInStream()
{
  NYI;
}

void BaseAudioPlayStream::SignalStreamCompleted()
{
  NYI;
}
