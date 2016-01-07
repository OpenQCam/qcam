
#include "mux/avmuxstream.h"
#include "streaming/baseoutstream.h"
#include "protocols/baseprotocol.h"
#include "hardwaremanager.h"
#include "video/videodefine.h"
#include "audio/baseaudiodevice.h"
#include "mux/basetsmuxoutstream.h"

static RTMPFrameHeader RTMPIFrameHeader = {
  { 0x17, 0x1, 0x0, 0x0, 0x0 },
  5,
};

static RTMPFrameHeader RTMPPFrameHeader = {
  { 0x27, 0x1, 0x0, 0x0, 0x0 },
  5,
};

AVMuxStream::AVMuxStream(StreamsManager *pStreamsManager, string name)
  : BaseMuxStream (pStreamsManager, ST_IN_AV_MUX, name) {
  HLS_IsEnable=false;
  _myOStream = (BaseOutStream *)new BaseTSMuxOutStream(pStreamsManager, name);
}

AVMuxStream::~AVMuxStream() {
}

void AVMuxStream::EnableHLS() {
  HLS_IsEnable=true;
}

bool AVMuxStream::IsCompatibleWithType(uint64_t type) {
  return TAG_KIND_OF(type, ST_OUT_NET_RTMP_4_RTMP)
    || TAG_KIND_OF(type, ST_OUT_NET_RTP)
    || TAG_KIND_OF(type, ST_OUT_FILE_MP4)
    ;
}

void AVMuxStream::OnDeviceAudioData(DeviceData& data) {
  LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;
  while (pTemp != NULL) {
    switch(pTemp->info->GetType()){
      case ST_OUT_NET_RTMP_4_RTMP:
        _dataBuffer.ReadFromByte(_RTMPAudioHeader);
        _dataBuffer.ReadFromBuffer(data.buffer, data.length);
        if (!pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, true)) {
          FATAL("Unable to feed OS: %p", pTemp->info);
        }
        _dataBuffer.IgnoreAll();
        break;
      default:
        if (!pTemp->info->FeedData(data.buffer, data.length, 0, data.length, _currentTS, true)) {
          FATAL("Unable to feed OS: %p", pTemp->info);
        }
        break;
    }
    pTemp = pTemp->pPrev;
  }
}

void AVMuxStream::OnDeviceVideoData(DeviceData& data) {
  //uint8_t naluType = 0;
  //uint8_t spsLength = 0;
  LinkedListNode<BaseOutStream *> *pTemp;
  uint8_t *pBuf;
  uint32_t bufLength;
  //cache frame data
  uint8_t *myfile;
  uint32_t mysize=0;
  if(HLS_IsEnable) {
    myfile=(uint8_t*) malloc(data.length);
    mysize=data.length;
    memcpy(myfile, data.buffer, data.length);
  }

  uint8_t naluAmount = ParseFrame(data.buffer, data.length, 0);
  //INFO("nalu amount = %d", naluAmount);
  int frameType=-1;
  //INFO("new frame !!\n");
  for(uint8_t i=0; i<naluAmount; ++i) {
    //INFO("length %d, type %x", _nalus[i].length, _nalus[i].type);
    switch (_nalus[i].type){
      case NALU_TYPE_IDR:
        if(frameType==-1)
          frameType=1;
        if(_requestKeyFrame){
          ResendSPSPPS();
          _requestKeyFrame = false;
        }
        pTemp = _pOutStreams;
        while (pTemp != NULL) {
          switch(pTemp->info->GetType()){
            case ST_OUT_NET_RTP:
            case ST_OUT_FILE_MP4:
            case ST_OUT_RECORD_AVC:
              //INFO("RTP");
              if(!pTemp->info->FeedData(_nalus[i].data, _nalus[i].length, 0,
                    _nalus[i].length, _currentTS, false)){
                FATAL("Unable to feed OS: %p", pTemp->info);
              }
              break;
            case ST_OUT_NET_RTMP_4_RTMP:
              pBuf = _nalus[i].data - RTMPIFrameHeader.length - 4;
              bufLength = _nalus[i].length + RTMPIFrameHeader.length + 4;
              memcpy(pBuf, RTMPIFrameHeader.header, RTMPIFrameHeader.length);
              if(!pTemp->info->FeedData(pBuf, bufLength, 0, bufLength, _currentTS, false)){
                FATAL("Unable to feed OS: %p", pTemp->info);
              }
              break;
            default:
              INFO("Cannot handle this stream type - %"PRIu64, pTemp->info->GetType());
              break;
          }
          pTemp = pTemp->pPrev;
        } // while
        break;
      case NALU_TYPE_SLICE:
        if(frameType==-1)
          frameType=0;
        pTemp = _pOutStreams;
        while (pTemp != NULL) {
          switch(pTemp->info->GetType()){
            case ST_OUT_NET_RTP:
            case ST_OUT_FILE_MP4:
            case ST_OUT_RECORD_AVC:
              // Ignore P frame in order to prevent incomplete sequence
              if(_requestKeyFrame){
                 pTemp = pTemp->pPrev;
                 continue;
              }
              if(!pTemp->info->FeedData(_nalus[i].data, _nalus[i].length, 0, _nalus[i].length, _currentTS, false)){
                FATAL("Unable to feed OS: %p", pTemp->info);
              }
              break;
            case ST_OUT_NET_RTMP_4_RTMP:
              pBuf = _nalus[i].data - RTMPPFrameHeader.length - 4;
              bufLength = _nalus[i].length + RTMPPFrameHeader.length + 4;
              memcpy(pBuf, RTMPPFrameHeader.header, RTMPPFrameHeader.length);
              if(!pTemp->info->FeedData(pBuf, bufLength, 0, bufLength, _currentTS, false)){
                FATAL("Unable to feed OS: %p", pTemp->info);
              }
              break;
            default:
              INFO("Cannot handle this stream type - %"PRIu64, pTemp->info->GetType());
              break;
          } // end switch
          pTemp = pTemp->pPrev;
        } // while
        break;

      case NALU_TYPE_SPS: // SPS
        if(!_isInitVideo){
          INFO("init video sps/pps");
          int j;
          bool found=false;
          for(j=i;j<naluAmount;j++)
          {
            if(_nalus[j].type == NALU_TYPE_PPS) {
              _pStreamCapabilities->InitVideoH264(_nalus[i].data, _nalus[i].length, _nalus[j].data, _nalus[j].length);
              found=true;
              _isInitVideo = true;
              ResendSPSPPS();
              break;
            }
          }
          if(!found)
            FATAL("Unable to find PPS in next NALU");
        }
        break;
      case NALU_TYPE_PPS:
      case NALU_TYPE_SEI:
        // do nothing
        break;
      default:
        WARN("Unable to deal with NALU type %x", _nalus[i].type);
        break;
    } // nalu type switch
  }// end of for
  if(HLS_IsEnable) {
  if (frameType==1) {
      bool isPframe=false;
      uint8_t buffercpy[6];
      buffercpy[0]=0x00;
      buffercpy[1]=0x00;
      buffercpy[2]=0x00;
      buffercpy[3]=0x01;
      buffercpy[4]=0x09;
      buffercpy[5]=isPframe?0x30:0x10;
      uint8_t *myOutFile=(uint8_t*) malloc(mysize+6);
      memcpy(myOutFile,buffercpy,6);
      memcpy(myOutFile+6,myfile,mysize);
      if (!_myOStream->FeedData(myOutFile, mysize+6, 0, mysize+6, 0, true)) {
          FATAL("Unable to feed TS:");
      }
      free(myOutFile);
      free(myfile);
  }else if(frameType==0){
      bool isPframe=true;
      uint8_t buffercpy[6];
      buffercpy[0]=0x00;
      buffercpy[1]=0x00;
      buffercpy[2]=0x00;
      buffercpy[3]=0x01;
      buffercpy[4]=0x09;
      buffercpy[5]=isPframe?0x30:0x10;
      uint8_t *myOutFile=(uint8_t*) malloc(mysize+6);
      memcpy(myOutFile,buffercpy,6);
      memcpy(myOutFile+6,myfile,mysize);
      if (!_myOStream->FeedData(myOutFile, mysize+6, 0, mysize+6, 0, false)) {
          FATAL("Unable to feed TS:");
      }
      free(myOutFile);
      free(myfile);
  }
  }//end of HLS_IsEnable
}
