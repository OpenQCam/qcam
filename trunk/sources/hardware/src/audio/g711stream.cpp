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
//thelib
#include "streaming/streamstypes.h"
#include "hardwaremanager.h"
#include "protocols/http/basehttpwsprotocol.h"
//hardware
#include "audio/g711stream.h"
#include "audio/baseaudiodevice.h"
#include "streaming/baseoutstream.h"


#define IS_BIG_ENDIAN (*(uint16_t*)"\1\0">>8)

#define SIGN_BIT    (0x80)
#define QUANT_MASK  (0xf)
#define NSEGS       (8)
#define SEG_SHIFT   (4)
#define SEG_MASK    (0x70)
#define BIAS        (0x84)
#define CLIP         8159
static const short seg_aend[8] = {0x1F, 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF};
static const short seg_uend[8] = {0x3F, 0x7F, 0xFF, 0x1FF,0x3FF, 0x7FF, 0xFFF, 0x1FFF};
const int32_t cBias = 0x84;
const int32_t cClip = 32635;

/* Temperary here, should having a G711Encoder Class */
static short search(short val, const short *table, short size)
{
    short i;
    for (i = 0; i < size; i++)
    {
        if (val <= *table++)
        return (i);
    }
    return (size);
}

unsigned char linear2alaw(short pcm_val)    /* 2's complement (16-bit range) */
{
  short       mask;
  short       seg;
  unsigned char   aval;

  pcm_val = pcm_val >> 3;
  if (pcm_val >= 0) {
      mask = 0xD5;        /* sign (7th) bit = 1 */
  }
  else {
    mask = 0x55;        /* sign bit = 0 */
    pcm_val = -pcm_val - 1;
  }

  seg = search(pcm_val, seg_aend, 8);
  if (seg >= 8)
    return (unsigned char) (0x7F ^ mask);
  else {
    aval = (unsigned char) seg << SEG_SHIFT;
    if (seg < 2)
      aval |= (pcm_val >> 1) & QUANT_MASK;
    else
      aval |= (pcm_val >> seg) & QUANT_MASK;
    return (aval ^ mask);
  }
}

/*
 *  ----------------      --------------
 *  00000001wxyza           000wxyz
 *  0000001wxyzab           001wxyz
 *  000001wxyzabc           010wxyz
 *  00001wxyzabcd           011wxyz
 *  0001wxyzabcde           100wxyz
 *  001wxyzabcdef           101wxyz
 *  01wxyzabcdefg           110wxyz
 *  1wxyzabcdefgh           111wxyz
 */

inline unsigned char linear2ulaw(short pcm_val)
{
  short mask;
  short seg;
  unsigned char uval;

  /* Get the sign and the magnitude of the value. */
  pcm_val = pcm_val >> 2;

  if (pcm_val < 0) {
    pcm_val = -pcm_val;
    mask = 0x7F;
  }
  else
    mask = 0xFF;

  if (pcm_val > CLIP)
    pcm_val = CLIP;

  pcm_val += (BIAS >> 2);
  /* Convert the scaled magnitude to segment number. */
  seg = search(pcm_val, seg_uend, 8);

  if (seg >= 8)
    return (unsigned char) (0x7F ^ mask);
  else {
    uval = (unsigned char) (seg << 4) | ((pcm_val >> (seg + 1)) & 0xF);
    return (uval ^ mask);
  }
}

G711Stream::G711Stream(StreamsManager *pStreamsManager, string name)
: BaseAudioStream(pStreamsManager, ST_IN_AUDIO_G711, name)
{
  _pStreamCapabilities = new StreamCapabilities();
  _pStreamCapabilities->InitAudioG711();
}


G711Stream::~G711Stream()
{
  delete _pStreamCapabilities;
}

StreamCapabilities * G711Stream::GetCapabilities()
{
  return _pStreamCapabilities;
}

// IDeviceObserver
void G711Stream::OnDeviceMessage(DeviceMessage &msg) {
  string rmsValue=msg.info;

  if (msg.type == ACM_RMSVALUE) {
    LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;
    while (pTemp != NULL) {
      if (pTemp->info->GetType() == ST_OUT_WSNET_AUDIO){
        //FIXME(Recardo): should use a better method to send rms value
        BaseHTTPWSProtocol* pWSProtocol=reinterpret_cast<BaseHTTPWSProtocol*>(pTemp->info->GetProtocol());
        pWSProtocol->EnqueueForWSOutbound ((uint8_t*)rmsValue.c_str(), rmsValue.length(), true, WS_OPCODE_TEXT_FRAME);
      }
      pTemp = pTemp->pPrev;
    }
  }
}

//void G711Stream::SendData(uint8_t *buf, uint32_t size, struct timeval *ts)
void G711Stream::OnDeviceData(DeviceData &data)
{
  double current_ts;
  uint32_t size=data.length>>1;
  uint8_t ulaw[size];

  GETTIMESTAMP(current_ts);
  //encode
  for (uint32_t i=0; i<(size); i++) {
    short sample = (data.buffer[i*2+1]<<8)|(data.buffer[i*2]);
    ulaw[i]=linear2ulaw(sample);
  }

  LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;
  while (pTemp != NULL) {
    switch(pTemp->info->GetType()){
      case ST_OUT_NET_RTMP_4_RTMP:
        _dataBuffer.ReadFromByte(0x8D);
        _dataBuffer.ReadFromBuffer(ulaw, size);
        if(!pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), current_ts, true)){
          FATAL("Unable to feed OS: %p", pTemp->info);
        }
        _dataBuffer.IgnoreAll();
        break;
      case ST_OUT_NET_AV:
      case ST_OUT_WSNET_AUDIO:
        if (!pTemp->info->FeedData(ulaw, size, 0, size, current_ts, true)) {
          FATAL("Unable to feed OS: %p", pTemp->info);
        }
        break;
      case ST_OUT_NET_RTP:
        if (!pTemp->info->FeedData(ulaw, size, 0, size, current_ts, true)) {
          FATAL("Unable to feed OS: %p", pTemp->info);
        }
        break;
      default:
        break;
    }
    pTemp = pTemp->pPrev;
  }
}

void G711Stream::SignalOutStreamAttached(BaseOutStream *pOutStream)
{
  _dataBuffer.IgnoreAll();
  GetHeader(_dataBuffer, 0);

  if (pOutStream->GetType() == ST_OUT_WSNET_AUDIO) {
    _dataBuffer.IgnoreAll();
  }

  if (!pOutStream->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, true)) {
    FATAL("Unable to feed OS: %p", pOutStream);
  }
  _dataBuffer.IgnoreAll();
}

uint32_t G711Stream::GetHeader (IOBuffer& headerBuf, uint32_t payloadlen)
{
  uint8_t *pChar;
  uint16_t bitwidth = 8;
  uint32_t totalLen = payloadlen + 36;

  device_parameters deviceparams = reinterpret_cast<BaseAudioDevice*>(HardwareManager::GetHardwareInstance(HT_MIC))->GetDeviceParameters();
  //magic number
  headerBuf.ReadFromString("RIFF");
  //len
  if (payloadlen == 0) {
    totalLen=0xFFFFFFFF;
    payloadlen = totalLen-36;
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
  // u-law
  headerBuf.ReadFromByte(0x07);
  headerBuf.ReadFromByte(0x00);
  // 2 channel
  pChar = (uint8_t*)(&deviceparams.n_channels);
  headerBuf.ReadFromByte(*(pChar+0));
  headerBuf.ReadFromByte(*(pChar+1));

  // 44.1k, sample rate
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
  if (payloadlen != 0) {
    pChar = (uint8_t*)(&payloadlen);
    headerBuf.ReadFromByte(*(pChar+0));
    headerBuf.ReadFromByte(*(pChar+1));
    headerBuf.ReadFromByte(*(pChar+2));
    headerBuf.ReadFromByte(*(pChar+3));
  }
  else {
    headerBuf.ReadFromByte(0xFF);
    headerBuf.ReadFromByte(0xFF);
    headerBuf.ReadFromByte(0xFF);
    headerBuf.ReadFromByte(0xFF);
  }

  return payloadlen;
}

uint32_t G711Stream::GetAudioDataTh() {
  return (1024*8);
}

uint32_t G711Stream::GetDataGuardRange() {
  return (0);
}
