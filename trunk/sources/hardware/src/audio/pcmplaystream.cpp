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
// Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// ------------------------------------------------------------------
// Purpose :
// ==================================================================
*/
#include "audio/pcmplaystream.h"
//thelib
#include "streaming/streamsmanager.h"
#include "streaming/streamstypes.h"
#include "streaming/baseoutstream.h"
#include "streaming/inmsgstream.h"
#include "protocols/avhttpstream/avstreamdefine.h"
//hardware
#include "hardwaremanager.h"
#include "audio/speaker.h"

PCMPlayStream::PCMPlayStream(StreamsManager *pStreamsManager, string name)
    : BaseAudioPlayStream(pStreamsManager, ST_OUT_AUDIO_PCM, name),
    _pSpeakerDevice(NULL),
    _audioFrameSize(0)
{
  _pSpeakerDevice=reinterpret_cast<Speaker*>(HardwareManager::GetHardwareInstance(HT_SPEAKER));
  if (_pSpeakerDevice)
    _audioFrameSize=_pSpeakerDevice->GetAudioFrameSize();
}


PCMPlayStream::~PCMPlayStream()
{
}

StreamCapabilities *PCMPlayStream::GetCapabilities()
{
   NYI;
   return NULL;
}

void PCMPlayStream::OnDeviceMessage(DeviceMessage &msg) {
}

uint32_t PCMPlayStream::GetHeader(IOBuffer& headerBuf, uint32_t payloadLen) {
  NYI;
  return 0;
}

uint32_t PCMPlayStream::GetAudioDataTh() {
  return 0;
}

uint32_t PCMPlayStream::GetDataGuardRange() {
  return 0;
}


bool PCMPlayStream::FeedData(uint8_t *pData, uint32_t dataLength,
                             uint32_t procLength, uint32_t totalLength,
                             double absTS, bool isAudio) {
  if (_pSpeakerDevice){
    //DEBUG ("put data to speaker");
    _pSpeakerDevice->PutData(pData, dataLength);
  }
  return true;
}

bool PCMPlayStream::PullAudioData() {
  //pull data from inbound stream
  if (_pInStream) {
    _pInStream->FeedData(NULL,  _audioFrameSize, 0, 0, 0, false);
  }
  else {
    return false;
  }
  return true;
}

bool PCMPlayStream::SignalStop() {
  if (_pSpeakerDevice) {
    return _pSpeakerDevice->SignalStop();
  }
  return false;
}

void PCMPlayStream::SignalAttachedToInStream() {
  NYI;
}

void PCMPlayStream::SignalDetachedFromInStream() {
  //Do not call device close and stop...hell !!!
  NYI;
}
