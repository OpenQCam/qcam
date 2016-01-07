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

#ifdef QIC1816

#include "qic/qicavcstream.h"
#include "streaming/baseoutstream.h"
#include "hardwaremanager.h"
#include "video/videodefine.h"
#include "qic/qicavcvideocapture.h"

// frame header for RTMP
typedef struct {
  uint8_t header[10];
  uint8_t length;
} RTMPFrameHeader;

// TODO : RTMP header for main profile and high profile
static RTMPFrameHeader RTMPInitHeader = {
  { 0x17, 0x0, 0x0, 0x0, 0x0, 0x1, 0x42, 0xE0, 0x1F, 0x3 },
  10,
};

static RTMPFrameHeader RTMPIFrameHeader = {
  { 0x17, 0x1, 0x0, 0x0, 0x0 },
  5,
};

static RTMPFrameHeader RTMPPFrameHeader = {
 { 0x27, 0x1, 0x0, 0x0, 0x0 },
 5,
};

static RTMPFrameHeader RTMPEOSHeader = {
 { 0x17, 0x2, 0x0, 0x0, 0x0 },
 5,
};




//static FILE *fp;

QICAVCStream::QICAVCStream(StreamsManager *pStreamsManager, string name)
: BaseVideoStream (pStreamsManager, ST_IN_CAM_AVC, name),
  _requestKeyFrame(false),
  _isInitVideo(false),
  _currentTS(0),
  _lastTS(0),
  _pAVCCaptureInstance(0)
{
  //fp = fopen("/tmp/avc.dump", "w");
  _pStreamCapabilities = new StreamCapabilities();
  _pAVCCaptureInstance = (QICAVCVideoCapture *)HardwareManager::GetHardwareInstance(HT_VIDEO_AVC);

}

QICAVCStream::~QICAVCStream()
{
  //fclose(fp);
  if(_pStreamCapabilities) delete _pStreamCapabilities;
}

StreamCapabilities * QICAVCStream::GetCapabilities()
{
  return _pStreamCapabilities;
}

bool QICAVCStream::IsCompatibleWithType(uint64_t type) {
  return TAG_KIND_OF(type, ST_OUT_NET_RTMP_4_RTMP)
      || TAG_KIND_OF(type, ST_OUT_FILE_RTMP)
      || TAG_KIND_OF(type, ST_OUT_NET_RTP)
      || TAG_KIND_OF(type, ST_OUT_FILE_MP4)
      || TAG_KIND_OF(type, ST_OUT_NET_RAW)
      || TAG_KIND_OF(type, ST_OUT_RECORD_AVC)
      ;
}

int QICAVCStream::FindAUD(uint8_t *data, uint32_t length){
  uint8_t state=0;
  bool find_one = false;
  uint32_t offset = 0;
  uint32_t offset_lastsc = 0; // last start code offset
  _naluIndex = 0;

  static const uint8_t fsm[4][6] = {
    {1, 2, 3, 3, 0, 0},
    {0, 0, 0, 4, 0, 0},
    {0, 0, 0, 0, 5, 0},
    {0, 0, 0, 0, 0, 0}
  };
  if(length < 6) return -1;

  while(offset < length){
    switch(data[offset++]){
    case 0:
      state = fsm[0][state];
      break;
    case 1:
      state = fsm[1][state];
      break;
    case 9:
      state = fsm[2][state];
      break;
    default:
      state = fsm[3][state];
      break;
    }
    switch(state){
      case 5: // found AUD
        if(find_one){
          return (offset-5);
        }else{
          find_one = true;
        }
        break;
      case 4: // found start code
        //FINEST("find start code %d", offset-4);
        if(find_one){
          _naluSet[_naluIndex].start = data+offset_lastsc;
          _naluSet[_naluIndex].length = (offset-4) - offset_lastsc - 4; // remove start code 4bytes
          _naluSet[_naluIndex].type = *(data+offset_lastsc+4) & 0x1F;
          _naluIndex++;
        }
        offset_lastsc = offset-4; // update offset for last start code
        break;
      default:
        continue;
        break;
    }
  }

  return -1;
}


// QIC Data Callback
//void QICAVCStream::SendData(IOBuffer *buf, struct timeval *ts)
//bool QICAVCStream::FeedData(IOBuffer &buf, struct timeval &ts)
void QICAVCStream::OnDeviceData(DeviceData &data)
{
  int32_t len;
  uint32_t frameTSOffset;
  //static double ats = 0;
  //static uint32_t lastTS = 0;
  NALU *pNALU;
  LinkedListNode<BaseOutStream *> *pTemp;
  _videoBuffer.ReadFromBuffer(data.buffer, data.length);

  //uint32_t i;
  while ( (len = FindAUD(GETIBPOINTER(_videoBuffer), GETAVAILABLEBYTESCOUNT(_videoBuffer))) > 0) {
    //INFO("%s", STR(buf->ToString()));

    for(int i=0; i<_naluIndex; ++i){
      pNALU = &_naluSet[i];
      // fill length to form ES stream
      //INFO("%d", pNALU->type);
      switch(pNALU->type){
        case 9: // AUD
          break;
        case 6: // SEI
          frameTSOffset = ((pNALU->start[26] << 24) + (pNALU->start[27] << 16) + (pNALU->start[28] << 8) + pNALU->start[29])/10;
          //INFO("ts= %d", frameTSOffset);
          if(_currentTS == 0){
            _lastTS = frameTSOffset;
            GETTIMESTAMP(_currentTS);
          } else{
            _currentTS += ((double)(frameTSOffset - _lastTS));
            //INFO("%f %d %d", _currentTS, _lastTS, frameTSOffset);
            _lastTS = frameTSOffset;
          }
          break;
        case 5:
          pNALU->start[0] = (uint8_t) (pNALU->length >> 24);
          pNALU->start[1] = (uint8_t) (pNALU->length >> 16);
          pNALU->start[2] = (uint8_t) (pNALU->length >> 8);
          pNALU->start[3] = (uint8_t) (pNALU->length);

          if(_requestKeyFrame){
            ResendSPSPPS();
            _requestKeyFrame = false;
          }

          pTemp = _pOutStreams;
          while (pTemp != NULL) {
            switch(pTemp->info->GetType()){
              case ST_OUT_NET_RTP:
                //INFO("RTP");
                if(!pTemp->info->FeedData(pNALU->start+4, pNALU->length, 0, pNALU->length, _currentTS, false)){
                  FATAL("Unable to feed OS: %p", pTemp->info);
                }
                break;
              case ST_OUT_NET_RTMP_4_RTMP:
                _dataBuffer.ReadFromBuffer(RTMPIFrameHeader.header, RTMPIFrameHeader.length);
                _dataBuffer.ReadFromBuffer(pNALU->start, pNALU->length+4);
                pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false);
                _dataBuffer.IgnoreAll();
                break;
              case ST_OUT_FILE_MP4:
              case ST_OUT_RECORD_AVC:
                if(!pTemp->info->FeedData(pNALU->start, pNALU->length+4, 0, pNALU->length+4, _currentTS, false)){
                  FATAL("Unable to feed OS: %p", pTemp->info);
                }
                break;
              default:
                break;
            } //switch
            pTemp = pTemp->pPrev;
          } // while
          break;

        case 1:
          // fill length to form ES stream
          pNALU->start[0] = (uint8_t) (pNALU->length >> 24);
          pNALU->start[1] = (uint8_t) (pNALU->length >> 16);
          pNALU->start[2] = (uint8_t) (pNALU->length >> 8);
          pNALU->start[3] = (uint8_t) (pNALU->length);

          pTemp = _pOutStreams;
          while (pTemp != NULL) {
            switch(pTemp->info->GetType()){
              case ST_OUT_NET_RTP:
                // Ignore P frame in order to prevent incomplete sequence
                if(_requestKeyFrame){
                   pTemp = pTemp->pPrev;
                   continue;
                }
                if(!pTemp->info->FeedData(pNALU->start+4, pNALU->length, 0, pNALU->length, _currentTS, false)){
                  FATAL("Unable to feed OS: %p", pTemp->info);
                }
                break;
              case ST_OUT_NET_RTMP_4_RTMP:
                _dataBuffer.ReadFromBuffer(RTMPPFrameHeader.header, RTMPPFrameHeader.length);
                _dataBuffer.ReadFromBuffer(pNALU->start, pNALU->length+4);
                pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false);
                _dataBuffer.IgnoreAll();
                break;
              case ST_OUT_FILE_MP4:
              case ST_OUT_RECORD_AVC:
                if(!pTemp->info->FeedData(pNALU->start, pNALU->length+4, 0, pNALU->length+4, _currentTS, false)){
                  FATAL("Unable to feed OS: %p", pTemp->info);
                }
                break;
              default:
                break;
            } // end switch
            pTemp = pTemp->pPrev;
          } // while
          break;

        case 7: // SPS
          break;

        case 8: // PPS
          if(!_isInitVideo){
            _pStreamCapabilities->InitVideoH264(_naluSet[i-1].start+4, _naluSet[i-1].length , _naluSet[i].start+4, _naluSet[i].length);
            //INFO("Level : %d, Profile : %d",(uint32_t)_pStreamCapabilities->avc._SPSInfo["level_idc"],(uint32_t)_pStreamCapabilities->avc._SPSInfo["profile_idc"]);
            //INFO("Constraint0 : %d, Constraint1 : %d, Constraint2 : %d",(uint32_t)_pStreamCapabilities->avc._SPSInfo["constraint_set0_flag"],(uint32_t)_pStreamCapabilities->avc._SPSInfo["constraint_set1_flag"],(uint32_t)_pStreamCapabilities->avc._SPSInfo["constraint_set2_flag"]);
            _isInitVideo = true;
            ResendSPSPPS();
            //INFO("%s", STR(_pStreamCapabilities->avc._SPSInfo));
          }
          break;

        default:
          break;
      } // nalu type switch

    } //for

    _videoBuffer.Ignore(len);
  } //while

  //INFO("Get AVC data %d", length);
  // feed data to outstream here
  //return true;


}

void QICAVCStream::ResendSPSPPS()
{

  //double ats;
  LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;
  while(pTemp != NULL){
    //_pCameraInstance->SetGOP(30); // FIXME: Iframe will lose even we request I frame because of udp protocol.
    if(pTemp->info->GetType() == ST_OUT_NET_RTP){
      if(!pTemp->info->FeedData(_pStreamCapabilities->avc._pSPS, _pStreamCapabilities->avc._spsLength, 0, _pStreamCapabilities->avc._spsLength, _currentTS, false)){
        FATAL("Unable to feed OS: %p", pTemp->info);
      }
      if(!pTemp->info->FeedData(_pStreamCapabilities->avc._pPPS, _pStreamCapabilities->avc._ppsLength, 0, _pStreamCapabilities->avc._ppsLength, _currentTS, false)){
        FATAL("Unable to feed OS: %p", pTemp->info);
      }
      //if(!pTemp->info->FeedData(pBuf, length, 0, length, ats, false)){
      //  FATAL("Unable to feed OS: %p", pTemp->info);
      //}
    }else if(pTemp->info->GetType() == ST_OUT_NET_RTMP_4_RTMP){

      _dataBuffer.ReadFromBuffer(RTMPEOSHeader.header, RTMPEOSHeader.length);
      if(!pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false)){
        FATAL("Unable to feed OS: %p", pTemp->info);
      }
      _dataBuffer.IgnoreAll();

      _dataBuffer.ReadFromBuffer(RTMPInitHeader.header, RTMPInitHeader.length);
      _dataBuffer.ReadFromByte(0x1);
      _dataBuffer.ReadFromByte(uint8_t ((_pStreamCapabilities->avc._spsLength>>8) & 0xFF));
      _dataBuffer.ReadFromByte(uint8_t (_pStreamCapabilities->avc._spsLength & 0xFF));
      _dataBuffer.ReadFromBuffer(_pStreamCapabilities->avc._pSPS, _pStreamCapabilities->avc._spsLength);
      _dataBuffer.ReadFromByte(0x1);
      _dataBuffer.ReadFromByte(uint8_t ((_pStreamCapabilities->avc._ppsLength>>8) & 0xFF));
      _dataBuffer.ReadFromByte(uint8_t (_pStreamCapabilities->avc._ppsLength & 0xFF));
      _dataBuffer.ReadFromBuffer(_pStreamCapabilities->avc._pPPS, _pStreamCapabilities->avc._ppsLength);

      if(!pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false)){
        FATAL("Unable to feed OS: %p", pTemp->info);
      }
      _dataBuffer.IgnoreAll();
    } else{
      // do speciifc tasks here
    }
    pTemp = pTemp->pPrev;
  }



}

void QICAVCStream::SignalOutStreamAttached(BaseOutStream *pOutStream)
{
  //IOBuffer tempBuffer;
  //double ats;
  //GETTIMESTAMP(ats);
  RequestKeyFrame();
  //_pCameraInstance->SetGOP(30); // FIXME: Iframe will lose even we request I frame because of udp protocol.
  if(pOutStream->GetType() == ST_OUT_NET_RTP){
    if(!pOutStream->FeedData(_pStreamCapabilities->avc._pSPS, _pStreamCapabilities->avc._spsLength, 0, _pStreamCapabilities->avc._spsLength, _currentTS, false)){
      FATAL("Unable to feed OS: %p", pOutStream);
    }
    if(!pOutStream->FeedData(_pStreamCapabilities->avc._pPPS, _pStreamCapabilities->avc._ppsLength, 0, _pStreamCapabilities->avc._ppsLength, _currentTS, false)){
      FATAL("Unable to feed OS: %p", pOutStream);
    }
    //if(!pOutStream->FeedData(pBuf, length, 0, length, ats, false)){
    //  FATAL("Unable to feed OS: %p", pOutStream);
    //}
  }else if(pOutStream->GetType() == ST_OUT_NET_RTMP_4_RTMP){

    _dataBuffer.ReadFromBuffer(RTMPInitHeader.header, RTMPInitHeader.length);
    _dataBuffer.ReadFromByte(0x1);
    _dataBuffer.ReadFromByte(uint8_t ((_pStreamCapabilities->avc._spsLength>>8) & 0xFF));
    _dataBuffer.ReadFromByte(uint8_t (_pStreamCapabilities->avc._spsLength & 0xFF));
    _dataBuffer.ReadFromBuffer(_pStreamCapabilities->avc._pSPS, _pStreamCapabilities->avc._spsLength);
    _dataBuffer.ReadFromByte(0x1);
    _dataBuffer.ReadFromByte(uint8_t ((_pStreamCapabilities->avc._ppsLength>>8) & 0xFF));
    _dataBuffer.ReadFromByte(uint8_t (_pStreamCapabilities->avc._ppsLength & 0xFF));
    _dataBuffer.ReadFromBuffer(_pStreamCapabilities->avc._pPPS, _pStreamCapabilities->avc._ppsLength);

    if(!pOutStream->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false)){
      FATAL("Unable to feed OS: %p", pOutStream);
    }
    _dataBuffer.IgnoreAll();
  } else{
    //if(!pOutStream->FeedData(pBuf, length, 0, length, ats, false)){
    //  FATAL("Unable to feed OS: %p", pOutStream);
    //}
  }


}


void QICAVCStream::SignalOutStreamDetached(BaseOutStream *pOutStream)
{
  //double ats;
  //GETTIMESTAMP(ats);

  if(pOutStream->GetType() == ST_OUT_NET_RTP){
    //if(!pOutStream->FeedData(pBuf, length, 0, length, ats, false)){
    //  FATAL("Unable to feed OS: %p", pOutStream);
    //}
    INFO("RTSP out stream detached from avcstream");
  }else if(pOutStream->GetType() == ST_OUT_NET_RTMP_4_RTMP){
    //_dataBuffer.ReadFromBuffer(RTMPEOSHeader.header, RTMPEOSHeader.length);
    //if(!pOutStream->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), ats, false)){
    //  FATAL("Unable to feed OS: %p", pOutStream);
    //}
    //_dataBuffer.IgnoreAll();
    INFO("RTMP outstream detached from avcstream");
  } else{
    //if(!pOutStream->FeedData(pBuf, length, 0, length, ats, false)){
    //  FATAL("Unable to feed OS: %p", pOutStream);
    //}
  }

}



void QICAVCStream::RequestKeyFrame()
{
  //FIXME(Tim): RequestKeyFrame should rely on handwaremanager to get related instance
  _requestKeyFrame = true;
  _pAVCCaptureInstance->GetKeyFrame();
}


//void QICAVCStream::OnNotifyMessage (NOTIFICATION_MSG_TYPE type, string message)
void QICAVCStream::OnDeviceMessage (DeviceMessage &msg)
{
  switch(msg.type){
    case VCM_RESOLUTION_CHANGED:
      _currentTS = 0;
      _isInitVideo = false;
      RequestKeyFrame();
      break;
    case VCM_FPS_CHANGED:
      _isInitVideo = false;
      RequestKeyFrame();
      break;
    case VCM_PROFILE_CHANGED:
      _isInitVideo = false;
      RequestKeyFrame();
      break;
    default:
    break;
  }

}

#endif
