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
// Author  : Recardo.Cheng
// ------------------------------------------------------------------
// Purpose :
// ==================================================================
*/

#ifdef HAS_CODEC_MP3STREAM
/* MP3 Codec */

#include "hardwaremanager.h"
#include "audio/mp3stream.h"
#include "streaming/streamstypes.h"
#include "streaming/baseoutstream.h"

//#define MP3STREAM_DEBUG
#ifdef MP3STREAM_DEBUG
FILE *pfOutputMP3= NULL;
FILE *pfPCM=NULL;
#endif
int16_t *pcmMP3buf;
int16_t *pcmMP3bufPt;
unsigned long samplesInput;
int bytesWritten;
double mp3Duration, ats;


// RTMP Header for MP3
static uint8_t RTMPMP3SequenceHeader[4] = { 0xAF, 0x0, 0x11, 0x90 }; /* TODO: modify the header data */
static uint8_t RTMPMP3RawHeader[2] = { 0xAF, 0x1 }; /* TODO: modify the header data */

MP3Stream::MP3Stream(StreamsManager *pStreamsManager, string name)
: BaseAudioStream(pStreamsManager, ST_IN_AUDIO_MP3, name)
{
  BaseAudioDevice *pAudioCapDevice= reinterpret_cast<BaseAudioDevice*> (HardwareManager::GetHardwareInstance(HT_MIC));
  _pMP3streamCapabilities= new StreamCapabilities();

  //pcmbuf = new uint8_t (PCM_PAGE_SIZE);
  //mp3 config
  L3_set_config_mpeg_defaults(&_mp3config.mpeg);
  //wave config
  _sampleRate = pAudioCapDevice->GetSampleRate();
  _numOfChannel = pAudioCapDevice->GetNumberOfChannels();
  _bitsPerSample = pAudioCapDevice->GetBitsPerSample();
  _bitRate = _mp3config.mpeg.bitr;//

  //set wave type, TODO: removed
  _mp3config.wave.type = WAVE_RIFF_PCM;
  _mp3config.wave.channels = _numOfChannel; //mono
  _mp3config.wave.samplerate = _sampleRate;
  _mp3config.wave.bits = _bitsPerSample;

  //config sample rate index, and bit rate
  _mp3config.mpeg.samplerate_index = L3_find_samplerate_index(_sampleRate, _mp3config.mpeg.type);
  if ( _mp3config.mpeg.samplerate_index < 0)
    FATAL ("invalid sample rate in mp3stream ctor"); //TODO: should throw an execption

  _mp3config.mpeg.bitrate_index = L3_find_bitrate_index(_mp3config.mpeg.bitr, _mp3config.mpeg.type);
  if (_mp3config.mpeg.bitrate_index < 0)
    FATAL ("invliad bit rate in mpstream ctor");


  open_bit_stream_w(&_bs, BITSSIZE);
  memset ((char*) &_side_info, 0, sizeof(L3_side_info_t));
  L3_subband_initialise();
  L3_mdct_initialise();
  L3_loop_initialise();
  _mp3config.mpeg.mode_gr = (_mp3config.mpeg.type==1)? 2: 1;
  _samplesInput = _mp3config.mpeg.samples_per_frame = (_mp3config.mpeg.type==1)? 1152:576;
  //config->mpeg.total_frames = ();
  _mp3config.mpeg.bits_per_slot = 8;
  _sideinfo_len = 32;

  if (_mp3config.mpeg.type==1) {
    if (_numOfChannel==1)
      _sideinfo_len += 136;
    else
      _sideinfo_len += 256;
  }
  else {  //mpeg2
    if (_numOfChannel==1)
      _sideinfo_len += 72;
    else
      _sideinfo_len += 136;
  }

  if (_mp3config.mpeg.crc) _sideinfo_len += 16;

  _avg_slots_per_frame   = ((double)_samplesInput /
                           ((double)_sampleRate/1000)) *
                          ((double)_bitRate /
                           (double)_mp3config.mpeg.bits_per_slot);

  _whole_slots_per_frame = (int)_avg_slots_per_frame;
  _frac_slots_per_frame = _avg_slots_per_frame - (double)_whole_slots_per_frame;
  _slot_lag = -_frac_slots_per_frame;

  if (_frac_slots_per_frame==0)
    _mp3config.mpeg.padding = 0;

  DEBUG ("mp3 stream info: _sideinfo_len:%d sampleInput:%d bitRate:%d sampleRate:%d avg slots per frame:%d",
    _sideinfo_len,
    _samplesInput,
    _bitRate,
    _sampleRate,
    _avg_slots_per_frame);


  GETTIMESTAMP(ats);
  mp3Duration=(1000.0/(double)_sampleRate)*(double)(_samplesInput/_numOfChannel); //m second
  _pMP3streamCapabilities->aac.InitAACCapability(_sampleRate, _numOfChannel, _bitsPerSample, (samplesInput/_numOfChannel),
                                                 (_bitRate* 1000 *_numOfChannel), mp3Duration);  //bitrate: 2 channel
  _pMP3streamCapabilities->audioCodecId = CODEC_AUDIO_MP3;

#ifdef MP3STREAM_DEBUG
  if((pfOutputMP3=fopen("MICOut.mp3", "wb")) == NULL){
    FATAL("Open output file 'MICOut.mp3' fail... !!\n");
    exit(1);
  }
  if((pfPCM=fopen("MICOut.wav", "wb")) == NULL){
    FATAL("Open output file 'MICOut.mp3' fail... !!\n");
    exit(1);
  }
  DEBUG("Initial MP3Stream End..!!\n");
#endif


}

MP3Stream::~MP3Stream()
{

  INFO("Close MP3Stream...\n");
#ifdef MP3STREAM_DEBUG
  if (pfOutputMP3!=NULL)
    fclose(pfOutputMP3);
  if (pfPCM==NULL)
    fclose(pfPCM);
#endif
  close_bit_stream (&_bs);
}

StreamCapabilities *MP3Stream::GetCapabilities()
{
  return _pMP3streamCapabilities;
}


uint32_t MP3Stream::GetHeader(IOBuffer &headerBuf, uint32_t payloadlen)
{
  NYI;
  return payloadlen;
}

uint32_t MP3Stream::GetAudioDataTh() {
  return (1024*8);
}

uint32_t MP3Stream::GetDataGuardRange() {
  return (512);
}

// IDeviceObserver
void MP3Stream::OnDeviceMessage(DeviceMessage &msg) {
}
// IDeviceCarrier
void MP3Stream::OnDeviceData(DeviceData &data) {

  //uint32_t i,pcmLen= size>>1;
  uint32_t i,pcmLen= data.length>>1;
  int32_t mean_bits;
  double pe[2][2];
  //temp buffer for short
  short *shortBuf[2];
  char bsbuf[BITSSIZE];
  L3_scalefac_t scalefactor;
  L3_psy_ratio_t ratio;
  int bssize;


  memcpy (pcmbuf, data.buffer, data.length);

  shortBuf[0] = (short*) (pcmbuf);
    if (_frac_slots_per_frame) {
      if (_slot_lag> (_frac_slots_per_frame-1.0)) {
        _slot_lag -= _frac_slots_per_frame;
        _mp3config.mpeg.padding = 0;
     }
      else {
        _slot_lag += (1-_frac_slots_per_frame);
        _mp3config.mpeg.padding = 1;
      }
    }

    _mp3config.mpeg.bits_per_frame= 8*(_whole_slots_per_frame + _mp3config.mpeg.padding);
    mean_bits = (_mp3config.mpeg.bits_per_frame - _sideinfo_len)/_mp3config.mpeg.mode_gr;


    /* polyphase filtering */
    for (int gr=0; gr<_mp3config.mpeg.mode_gr; gr++)
      for (int channel=_numOfChannel; channel--;)
        for (int i=0; i<18; i++) {
          L3_window_filter_subband(&shortBuf[0], &_l3_sb_sample[channel][gr+1][i][0], channel);
        }

    L3_mdct_sub (_l3_sb_sample, _mdct_freq, &_mp3config);
    L3_iteration_loop (pe, _mdct_freq, &ratio, &_side_info, _l3_enc, mean_bits, &scalefactor, &_mp3config);
    L3_format_bitstream (_l3_enc, &_side_info, &scalefactor, &_bs, _mdct_freq, NULL, 0, &_mp3config);

    bssize = getbsbuf (&_bs, bsbuf);

    //Send outstream
#ifdef MP3STREAM_DEBUG
    fwrite (bsbuf, 1, bssize, pfOutputMP3);
#endif

    ats += mp3Duration;
    LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;


    while (pTemp != NULL) {
      switch(pTemp->info->GetType()){
        case ST_OUT_NET_RTMP_4_RTMP:
          _dataBuffer.ReadFromBuffer(RTMPMP3RawHeader, sizeof(RTMPMP3RawHeader));
          _dataBuffer.ReadFromBuffer((uint8_t*)bsbuf, bssize);
          if(!pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), ats, true)){
            FATAL("Unable to feed OS: %p", pTemp->info);
          }
          _dataBuffer.IgnoreAll();
          break;
        default:
          if (!pTemp->info->FeedData((uint8_t*)bsbuf, bssize, 2, 0, ats, true)) {
            FATAL("Unable to feed OS: %p", pTemp->info);
          }
          break;
      }
      pTemp = pTemp->pPrev;
    }


}



#if 0
/* you can not change the buffer in SendData */
void MP3Stream::SendData(uint8_t *buf, uint32_t size, struct timeval *ts)
{
  uint32_t i,pcmLen= size>>1;
  int32_t mean_bits;
  double pe[2][2];
  //temp buffer for short
  short *shortBuf[2];
  char bsbuf[BITSSIZE];
  L3_scalefac_t scalefactor;
  L3_psy_ratio_t ratio;
  int bssize;


  memcpy (pcmbuf, buf, size);

  shortBuf[0] = (short*) (pcmbuf);
    if (_frac_slots_per_frame) {
      if (_slot_lag> (_frac_slots_per_frame-1.0)) {
        _slot_lag -= _frac_slots_per_frame;
        _mp3config.mpeg.padding = 0;
     }
      else {
        _slot_lag += (1-_frac_slots_per_frame);
        _mp3config.mpeg.padding = 1;
      }
    }

    _mp3config.mpeg.bits_per_frame= 8*(_whole_slots_per_frame + _mp3config.mpeg.padding);
    mean_bits = (_mp3config.mpeg.bits_per_frame - _sideinfo_len)/_mp3config.mpeg.mode_gr;


    /* polyphase filtering */
    for (int gr=0; gr<_mp3config.mpeg.mode_gr; gr++)
      for (int channel=_numOfChannel; channel--;)
        for (int i=0; i<18; i++) {
          L3_window_filter_subband(&shortBuf[0], &_l3_sb_sample[channel][gr+1][i][0], channel);
        }

    L3_mdct_sub (_l3_sb_sample, _mdct_freq, &_mp3config);
    L3_iteration_loop (pe, _mdct_freq, &ratio, &_side_info, _l3_enc, mean_bits, &scalefactor, &_mp3config);
    L3_format_bitstream (_l3_enc, &_side_info, &scalefactor, &_bs, _mdct_freq, NULL, 0, &_mp3config);

    bssize = getbsbuf (&_bs, bsbuf);

    //Send outstream
#ifdef MP3STREAM_DEBUG
    fwrite (bsbuf, 1, bssize, pfOutputMP3);
#endif

    ats += mp3Duration;
    LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;


    while (pTemp != NULL) {
      switch(pTemp->info->GetType()){
        case ST_OUT_NET_RTMP_4_RTMP:
          _dataBuffer.ReadFromBuffer(RTMPMP3RawHeader, sizeof(RTMPMP3RawHeader));
          _dataBuffer.ReadFromBuffer((uint8_t*)bsbuf, bssize);
          if(!pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), ats, true)){
            FATAL("Unable to feed OS: %p", pTemp->info);
          }
          _dataBuffer.IgnoreAll();
          break;
        default:
          if (!pTemp->info->FeedData((uint8_t*)bsbuf, bssize, 2, 0, ats, true)) {
            FATAL("Unable to feed OS: %p", pTemp->info);
          }
          break;
      }
      pTemp = pTemp->pPrev;
    }

}
#endif

void MP3Stream::SignalOutStreamAttached(BaseOutStream *pOutStream)
{
  if (pOutStream->GetType() == ST_OUT_NET_RTMP_4_RTMP) {
    if(!pOutStream->FeedData(RTMPMP3SequenceHeader, sizeof(RTMPMP3SequenceHeader), 0, sizeof(RTMPMP3SequenceHeader), 0, true)){
        FATAL("Unable to feed OS: %p", pOutStream);
    }else{
      INFO("feed mp3 header");
    }
  }
  else {
    DEBUG ("SignalOutStreamAttached with unknown outstream type");
  }
}


#endif //HAS_CODEC_MP3STREAM
