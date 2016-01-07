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

#ifdef HAS_CODEC_AACSTREAM
#include "audio/aacstream.h"
#include "streaming/streamstypes.h"
#include "streaming/baseoutstream.h"
#include <faac.h>

enum stream_format {
  RAW_STREAM = 0,
  ADTS_STREAM = 1,
};

#ifdef AACSTREAM_DEBUG
FILE      *pfOutputMIC= NULL;  //point to Capture (Microphone)
FILE      *pfOutputAAC= NULL;
#endif
faacEncHandle hEncoder;
faacEncConfigurationPtr myFormat;
short *pcmAACbuf;
unsigned char *bitbuf;
int16_t *pcmAACbufPt;
unsigned long samplesInput, maxBytesOutput;
unsigned int aac_in_count=0;
int bytesWritten, sampleRate, numOfChannel, bitsPerSample;
double aacDuration, ats;


// RTMP Header for AAC
static uint8_t RTMPAACSequenceHeader[4] = { 0x28, 0x0, 0x11, 0x90 };
static uint8_t RTMPAACRawHeader[2] = { 0xAF, 0x1 };

AACStream::AACStream(BaseProtocol *pProtocol,
		StreamsManager *pStreamsManager, string name, BaseAudioCaptureDevice *pAudioCapDevice)
: AudioStream(pProtocol, pStreamsManager, ST_IN_AUDIO_AAC, name, pAudioCapDevice)
{
  _pAACstreamCapabilities= new StreamCapabilities();

  INFO("Initial AACStream...!!\n");

#ifdef AACSTREAM_DEBUG
  if((pfOutputMIC=fopen("MICIn.pcm", "wb")) == NULL){
    FATAL("Open output file 'MICIn.pcm' fail... !!\n");
    exit(1);
  }
#endif

  sampleRate = pAudioCapDevice->GetSampleRate();
  numOfChannel = pAudioCapDevice->GetNumberOfChannels();
  //bitsPerSample = instance->GetBitsPerSample();
  bitsPerSample = 16;
  INFO("PCM input SampleRate=%d, Channel=%d, BitsPerChannel=%d...!!\n", sampleRate, numOfChannel, bitsPerSample);

  if((sampleRate==0)||(numOfChannel==0)||(bitsPerSample==0)){
    FATAL("Error, Audio Parameters Setting Fail in ALSA... !!\n");
    exit(1);
  }

  /* open the encoder library */
  hEncoder = faacEncOpen(sampleRate, numOfChannel, &samplesInput, &maxBytesOutput);
  if(!hEncoder){
    FATAL("Error, faacEncOpen fail... !!\n");
    exit(1);
  }

#ifdef AACSTREAM_DEBUG
  DEBUG("\nsamplesInput=%ld, maxBytesOutput=%ld... !!\n",samplesInput, maxBytesOutput);
#endif

  INFO("AACStream Input Framelength=%ld, Output MaxBits=%ld...!!\n",samplesInput/numOfChannel, maxBytesOutput*8);

  pcmAACbuf = (short *)malloc(samplesInput*sizeof(short)); //AAC input:frame = samplesInput/2 Channel, 16bits/channel
  pcmAACbufPt= pcmAACbuf;
  bitbuf = (unsigned char*)malloc(maxBytesOutput*sizeof(unsigned char));

  /* put the options in the configuration struct */
  myFormat = faacEncGetCurrentConfiguration(hEncoder);
  myFormat->aacObjectType = LOW;             //LOW, MAIN, LTP
#ifdef AACSTREAM_DEBUG
  myFormat->mpegVersion = MPEG2;             //MPEG2, MPEG4
  myFormat->outputFormat = ADTS_STREAM;      //RAW_STREAM = 0(MP4), ADTS_STREAM = 1(AAC)
#else
  myFormat->mpegVersion = MPEG4;             //MPEG2, MPEG4
  myFormat->outputFormat = RAW_STREAM;      //RAW_STREAM = 0(MP4), ADTS_STREAM = 1(AAC)
#endif
  myFormat->useTns = 0;                      //TNS->0:disable, TNS->1:enable
  myFormat->allowMidside = 1;                //M/S:1->enable, 0->disable
  myFormat->bandWidth = 0;                   //default
//  myFormat->inputFormat = FAAC_INPUT_FLOAT;  //32bits
  myFormat->inputFormat = FAAC_INPUT_16BIT;    //16bits

  myFormat->bitRate=32000; //The minimum bitrate is 32kbps: 1 channel

  if (!faacEncSetConfiguration(hEncoder, myFormat)) {
        FATAL("Error, Unsupported faac output format..!!\n");
	exit(1);
  }

  INFO("AACStream average bitrate (ABR) bitrate/channel=%ld...!!\n",myFormat->bitRate);
  INFO("AACStream variable bitrate (VBR) quantizer quality in percent=%ld...!!\n",myFormat->quantqual);
  INFO("AACStream use object type LC (Low Complexity, default)=%d...!!\n",myFormat->aacObjectType);
  INFO("AACStream disable TNS, temporal noise shaping...!!\n");
  INFO("AACStream enable mid/side coding...!!\n");

  GETTIMESTAMP(ats);
  aacDuration=(1000.0/(double)sampleRate)*(double)(samplesInput/numOfChannel); //m second
  _pAACstreamCapabilities->aac.InitAACCapability(sampleRate, numOfChannel, bitsPerSample, (samplesInput/numOfChannel),
                                                 (myFormat->bitRate*numOfChannel),aacDuration);  //bitrate: 2 channel
  INFO("AACStream Frame Length Duration Time=%fm second...!!\n",aacDuration );
  _pAACstreamCapabilities->audioCodecId = CODEC_AUDIO_AAC;


#ifdef AACSTREAM_DEBUG
  if((pfOutputAAC=fopen("MICOut.aac", "wb")) == NULL){
    FATAL("Open output file 'MICOut.aac' fail... !!\n");
    exit(1);
  }
  DEBUG("Initial AACStream End..!!\n");
#endif


}

AACStream::~AACStream()
{

  INFO("Close AACStream...!!\n");
#ifdef AACSTREAM_DEBUG
  if(pfOutputMIC!=NULL) {
    fclose(pfOutputMIC);
  }

  if(pfOutputAAC!=NULL) {
    fclose(pfOutputAAC);
  }
#endif
  faacEncClose(hEncoder);

  free(pcmAACbuf);
  free(bitbuf);

}

StreamCapabilities * AACStream::GetCapabilities()
{
  return _pAACstreamCapabilities;
}


uint32_t AACStream::GetHeader(IOBuffer &headerBuf, uint32_t payloadlen)
{
  NYI;
	return payloadlen;
}

void AACStream::SendData(uint8_t *buf, uint32_t size, struct timeval *ts)
{

  int16_t *pcmInPt=(int16_t *)buf;
  uint32_t i,pcmLen= size>>1;

#ifdef AACSTREAM_DEBUG
  DEBUG("data size %d", size);
  //Write to MICIn.pcm file
  fwrite(pcmInPt, sizeof(int16_t), pcmLen, pfOutputMIC);
#endif

  for(i=0;i<pcmLen;i++){
    if((uint16_t)*pcmInPt == 0x8000)
	*pcmInPt=0x8001;
    *pcmAACbufPt++=*pcmInPt++;
    aac_in_count++;
    if(aac_in_count == samplesInput){
      pcmAACbufPt= pcmAACbuf;
#ifdef AACSTREAM_DEBUG
      DEBUG("aac_in_count=%d...!!\n",aac_in_count);
#endif
      aac_in_count=0;

      /* call the actual encoding routine */
      bytesWritten = faacEncEncode(hEncoder,(int32_t *)pcmAACbuf, samplesInput, bitbuf, maxBytesOutput);
#ifdef AACSTREAM_DEBUG
      DEBUG("AAC bytesWritten=%d...!!\n",bytesWritten);
#endif
      if (bytesWritten > 0){
#ifdef AACSTREAM_DEBUG
        DEBUG("AAC bytesWritten=%d...!!\n",bytesWritten);
        /* write bitstream to aac file */
        fwrite(bitbuf, 1, bytesWritten, pfOutputAAC);
#endif
        buf= bitbuf;
        size= bytesWritten;

        ats += aacDuration;
        LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;
        while (pTemp != NULL) {
          switch(pTemp->info->GetType()){
            case ST_OUT_NET_RTMP_4_RTMP:
              _dataBuffer.ReadFromBuffer(RTMPAACRawHeader, sizeof(RTMPAACRawHeader));
              _dataBuffer.ReadFromBuffer(buf, size);
              if(!pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), ats, true)){
                FATAL("Unable to feed OS: %p", pTemp->info);
              }
              _dataBuffer.IgnoreAll();
              break;
            default:
              if (!pTemp->info->FeedData(buf, size, 0, 0, ats, true)) {
                FATAL("Unable to feed OS: %p", pTemp->info);
              }
              break;
          }
          pTemp = pTemp->pPrev;
        }



      } else if (bytesWritten < 0) {
        FATAL("faacEncEncode fail...!!\n");
        exit(1);
      }
    }

  }

}

void AACStream::SignalOutStreamAttached(BaseOutStream *pOutStream)
{
  //FIXME(Recardo): according outbound stream type to send different header
  if(!pOutStream->FeedData(RTMPAACSequenceHeader, sizeof(RTMPAACSequenceHeader), 0, sizeof(RTMPAACSequenceHeader), 0, true)){
    FATAL("Unable to feed OS: %p", pOutStream);
  }else{
    INFO("feed aac header");
  }
}


uint32_t AACStream::GetAudioDataTh() {
  return (1024*8);
}

uint32_t AACStream::GetDataGuardRange() {
  return (512);
}

#endif //HAS_CODEC_AACSTREAM
