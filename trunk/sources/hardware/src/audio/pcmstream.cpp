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
#include "audio/pcmstream.h"
//thelib
#include "audio/baseaudiodevice.h"
#include "streaming/streamsmanager.h"
#include "streaming/streamstypes.h"
#include "streaming/wsoutnetaudiostream.h"
#include "protocols/avhttpstream/avstreamdefine.h"
#include "protocols/rtmp/streaming/baseoutnetrtmpstream.h"
//#include "protocols/http/websocket/soundtest.h"
#include "protocols/http/basehttpwsprotocol.h"
#include "system/eventdefine.h"
#include "application/baseclientapplication.h"

//hardware
#include "hardwaremanager.h"
#include "system/systemmanager.h"

PCMStream::PCMStream(StreamsManager *pStreamsManager, string name)
    : BaseAudioStream(pStreamsManager, ST_IN_AUDIO_PCM, name),
      _isInitAudio(false)
{
  device_parameters deviceparams = reinterpret_cast<BaseAudioDevice*>(HardwareManager::GetHardwareInstance(HT_MIC))->GetDeviceParameters();
  _pStreamCapabilities = new StreamCapabilities();
  _RTMPAudioHeader = 0x32;

  if (deviceparams.sample_rate==11025) {
    _RTMPAudioHeader |= (1<<2);
  }
  else if (deviceparams.sample_rate==22050) {
    _RTMPAudioHeader |= (2<<2);
  }
  else if (deviceparams.sample_rate==44100) {
    _RTMPAudioHeader |= (3<<2);
  }

  if (deviceparams.n_channels==2) {
    _RTMPAudioHeader |= (1);
  }

  if (!_isInitAudio) {
    //_pStreamCapabilities->InitAudioPCM(22069/*FIXME*/);
    _pStreamCapabilities->InitAudioPCM(deviceparams.sample_rate, deviceparams.n_channels);
    _isInitAudio = true;
  }

}


PCMStream::~PCMStream()
{
  delete _pStreamCapabilities;
}

StreamCapabilities * PCMStream::GetCapabilities()
{
  return _pStreamCapabilities;
}

bool PCMStream::Play() {
	LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;
	while (pTemp != NULL) {
		if (!pTemp->info->SignalAudioPlay()) {
			WARN("Unable to signal play on an outbound stream");
		}
		pTemp = pTemp->pPrev;
	}
	return true;
}

bool PCMStream::Stop() {
  LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;
	while (pTemp != NULL) {
		if (!pTemp->info->SignalAudioStop()) {
			WARN("Unable to signal play on an outbound stream");
		}
		pTemp = pTemp->pPrev;
	}
	return true;
}

// IDeviceObserver
void PCMStream::OnDeviceMessage(DeviceMessage &msg) {

  if (msg.type == ACM_RMSVALUE) {
    LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;
    while (pTemp != NULL) {
      if (pTemp->info->GetType()==ST_OUT_WSNET_AUDIO){
        WSOutNetAudioStream *pStream=reinterpret_cast<WSOutNetAudioStream*>(pTemp->info);
        string jsonRMS="{\"rms\":"+msg.info+"}";
        pStream->FeedMSGData((uint8_t*)(jsonRMS.c_str()), jsonRMS.length());
      }
      pTemp = pTemp->pPrev;
    }
    return;
  }

  switch(msg.type){
    case ACM_STARTED:
      Play();
      break;
    case ACM_STOPPED:
      Stop();
      break;
    default:
      break;
  }
}

// IDeviceCarrier
void PCMStream::OnDeviceData(DeviceData &data) {
  uint64_t current_ts;
  LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;

  current_ts = SystemManager::GetSyncTime();
  while (pTemp != NULL) {
    switch(pTemp->info->GetType()){
      case ST_OUT_NET_RTP:
        if (!pTemp->info->FeedData(data.buffer, data.length, 0, data.length, current_ts, true)) {
          FATAL("Unable to feed OS: %p", pTemp->info);
        }
        break;
      case ST_OUT_NET_RTMP_4_RTMP:
        _dataBuffer.ReadFromByte(_RTMPAudioHeader);
        _dataBuffer.ReadFromBuffer(data.buffer, data.length);
        if (!pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), current_ts, true)) {
          FATAL("Unable to feed OS: %p", pTemp->info);
        }
        _dataBuffer.IgnoreAll();
        break;
      default:
        if (!pTemp->info->FeedData(data.buffer, data.length, 0, data.length, current_ts, true)) {
          FATAL("Unable to feed OS: %p", pTemp->info);
        }
        break;
    }
    pTemp = pTemp->pPrev;
  }
}

void PCMStream::SignalOutStreamAttached(BaseOutStream *pOutStream) {
}

uint32_t PCMStream::GetHeader(IOBuffer& headerBuf, uint32_t payloadLen)
{
  uint8_t *pChar;
  uint16_t bitwidth = 16;
  uint32_t totalLen = payloadLen+36;

  device_parameters deviceparams = reinterpret_cast<BaseAudioDevice*>
      (HardwareManager::GetHardwareInstance(HT_MIC))->GetDeviceParameters();

  //magic number
  headerBuf.ReadFromString("RIFF");

  //len
  if (payloadLen==0) {
    totalLen=0xFFFFFFFF;
    payloadLen=totalLen-36;
  }

  pChar = (uint8_t*)(&totalLen);
  headerBuf.ReadFromByte(*(pChar+0));
  headerBuf.ReadFromByte(*(pChar+1));
  headerBuf.ReadFromByte(*(pChar+2));
  headerBuf.ReadFromByte(*(pChar+3));

  headerBuf.ReadFromString("WAVEfmt ");
  // subchunksize
  headerBuf.ReadFromByte(0x10);
  headerBuf.ReadFromByte(0x00);
  headerBuf.ReadFromByte(0x00);
  headerBuf.ReadFromByte(0x00);
  // pcm
  headerBuf.ReadFromByte(0x01);
  headerBuf.ReadFromByte(0x00);
  // channel
  pChar = (uint8_t*)(&deviceparams.n_channels);
  headerBuf.ReadFromByte(*(pChar+0));
  headerBuf.ReadFromByte(*(pChar+1));

  // sample rate
  pChar = (uint8_t*)(&deviceparams.sample_rate);
  headerBuf.ReadFromByte(*(pChar+0));
  headerBuf.ReadFromByte(*(pChar+1));
  headerBuf.ReadFromByte(*(pChar+2));
  headerBuf.ReadFromByte(*(pChar+3));

  // SampleRate * NumChannels * BitsPerSample/8
  uint32_t bitrate = (deviceparams.sample_rate*deviceparams.n_channels * bitwidth)/8;
  pChar = (uint8_t*)&bitrate;
  headerBuf.ReadFromByte(*(pChar+0));
  headerBuf.ReadFromByte(*(pChar+1));
  headerBuf.ReadFromByte(*(pChar+2));
  headerBuf.ReadFromByte(*(pChar+3));

  // NumChannels * BitsPerSample/8
  uint16_t channelbitwidth = (deviceparams.n_channels * bitwidth)/8;
  pChar = (uint8_t*)&channelbitwidth;
  headerBuf.ReadFromByte(*(pChar+0));
  headerBuf.ReadFromByte(*(pChar+1));

  //BitsPerSample
  pChar = (uint8_t*)&bitwidth;
  headerBuf.ReadFromByte(*(pChar+0));
  headerBuf.ReadFromByte(*(pChar+1));
  headerBuf.ReadFromString("data");

  pChar = (uint8_t*)(&payloadLen);
  headerBuf.ReadFromByte(*(pChar+0));
  headerBuf.ReadFromByte(*(pChar+1));
  headerBuf.ReadFromByte(*(pChar+2));
  headerBuf.ReadFromByte(*(pChar+3));

  return payloadLen;
}

uint32_t PCMStream::GetAudioDataTh() {
  return (1024*8);
}
