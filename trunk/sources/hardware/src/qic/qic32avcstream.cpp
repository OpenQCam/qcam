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
// Author  : Jennifer Chao (jennifer.chao@quantatw.com)
// ------------------------------------------------------------------
// Purpose :
// ==================================================================
*/

#ifdef QIC1832

#include "qic/qic32avcstream.h"
#include "streaming/baseoutstream.h"
#include "protocols/baseprotocol.h"
#include "hardwaremanager.h"
#include "video/videodefine.h"
#include "qic/qic32avcvideocapture.h"



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





QIC32AVCStream::QIC32AVCStream(StreamsManager *pStreamsManager, string name)
: BaseVideoStream (pStreamsManager, ST_IN_CAM_AVC, name),
  _requestKeyFrame(false),
  _isInitVideo(false),
  _currentTS(0),
  _lastFrameTS(0),
  _pAVCCaptureInstance(0)
{
  _pStreamCapabilities = new StreamCapabilities();
  _pAVCCaptureInstance = (QIC32AVCVideoCapture *)HardwareManager::GetHardwareInstance(HT_VIDEO_AVC);
}

QIC32AVCStream::~QIC32AVCStream()
{
  if(_pStreamCapabilities) delete _pStreamCapabilities;
}

StreamCapabilities * QIC32AVCStream::GetCapabilities()
{
  return _pStreamCapabilities;
}

bool QIC32AVCStream::IsCompatibleWithType(uint64_t type) {
  return TAG_KIND_OF(type, ST_OUT_NET_RTMP_4_RTMP)
      || TAG_KIND_OF(type, ST_OUT_FILE_RTMP)
      || TAG_KIND_OF(type, ST_OUT_NET_RTP)
      || TAG_KIND_OF(type, ST_OUT_FILE_MP4)
      || TAG_KIND_OF(type, ST_OUT_NET_RAW)
      || TAG_KIND_OF(type, ST_OUT_RECORD_AVC)
      ;
}


void QIC32AVCStream::OnDeviceData(DeviceData &data)
{
  //jennifer
  //FATAL("Stream name ; %s", STR(_name));
  uint8_t naluType = 0;
  uint8_t spsLength = 0;
  LinkedListNode<BaseOutStream *> *pTemp;

  if(_status == VSS_STOP) return;

  naluType = data.buffer[4] & 0x1F;

  if(data.ts){
    if(_lastFrameTS == 0){
      _lastFrameTS = data.ts;
    }
    _currentTS += (data.ts - _lastFrameTS);
    //INFO("type: %d - cts %f, pts %"PRIu64", offset %"PRIu64, naluType, _currentTS, data.ts, _lastFrameTS);
    _lastFrameTS = data.ts;
  }else{ // data.ts == 0, resolution changed
    GETTIMESTAMP(_currentTS);
    _lastFrameTS = 0;
  }

  FATAL("naluType : %d",naluType);
  //IOBuffer buffer;
  //FATAL("%s",STR(buffer.Dumpbuffer(data.buffer, data.length));
  switch(naluType){
    case 5:
      if(_requestKeyFrame){
        ResendSPSPPS();
        _requestKeyFrame = false;
      }
      pTemp = _pOutStreams;
      while (pTemp != NULL) {
        switch(pTemp->info->GetType()){
          case ST_OUT_NET_RTP:
            INFO("RTP");
            if(!pTemp->info->FeedData(data.buffer+4, data.length-4, 0, data.length-4, _currentTS, false)){
              FATAL("Unable to feed OS: %p", pTemp->info);
            }
            break;
          case ST_OUT_NET_RTMP_4_RTMP:
            _dataBuffer.ReadFromBuffer(RTMPIFrameHeader.header, RTMPIFrameHeader.length);
            _dataBuffer.ReadFromBuffer(data.buffer, data.length);
            pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false);
            _dataBuffer.IgnoreAll();
            break;
          case ST_OUT_FILE_MP4:
          case ST_OUT_RECORD_AVC:
            if(!pTemp->info->FeedData(data.buffer, data.length, 0, data.length, _currentTS, false)){
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
      pTemp = _pOutStreams;
      while (pTemp != NULL) {
        switch(pTemp->info->GetType()){
          case ST_OUT_NET_RTP:
            // Ignore P frame in order to prevent incomplete sequence
            if(_requestKeyFrame){
               pTemp = pTemp->pPrev;
               continue;
            }
            if(!pTemp->info->FeedData(data.buffer+4, data.length-4, 0, data.length-4, _currentTS, false)){
              FATAL("Unable to feed OS: %p", pTemp->info);
            }
            break;
          case ST_OUT_NET_RTMP_4_RTMP:
            _dataBuffer.ReadFromBuffer(RTMPPFrameHeader.header, RTMPPFrameHeader.length);
            _dataBuffer.ReadFromBuffer(data.buffer, data.length);
            pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false);
            _dataBuffer.IgnoreAll();
            break;
          case ST_OUT_FILE_MP4:
          case ST_OUT_RECORD_AVC:
            if(!pTemp->info->FeedData(data.buffer, data.length, 0, data.length, _currentTS, false)){
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
      if(!_isInitVideo){
        INFO("init video sps/pps");
        // Update timestamp
        //GETTIMESTAMP(_currentTS);
        spsLength = data.buffer[3];
        INFO("%x %x ", spsLength, data.buffer[spsLength+7]);
        _pStreamCapabilities->InitVideoH264(data.buffer+4, spsLength, data.buffer+spsLength+8, data.buffer[spsLength+7]);
        _isInitVideo = true;
        ResendSPSPPS();
        //INFO("%s", STR(_pStreamCapabilities->avc._SPSInfo));
      }
      break;

    default:
      break;
  } // nalu type switch



}

void QIC32AVCStream::ResendSPSPPS()
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

void QIC32AVCStream::SignalOutStreamAttached(BaseOutStream *pOutStream)
{
  //IOBuffer tempBuffer;
  //double ats;
  //GETTIMESTAMP(ats);
  RequestKeyFrame();
  INFO("stream attached");
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


void QIC32AVCStream::SignalOutStreamDetached(BaseOutStream *pOutStream)
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



void QIC32AVCStream::RequestKeyFrame()
{
  //FIXME(Tim): RequestKeyFrame should rely on handwaremanager to get related instance
  if(_pAVCCaptureInstance->IsStarted()){
    _requestKeyFrame = true;
    _pAVCCaptureInstance->GetKeyFrame();
  }
}

void QIC32AVCStream::UnlinkLiveStream() {
  vector<BaseOutStream *> outStreams = GetOutStreams();
  FOR_VECTOR(outStreams, i) {
    if(outStreams[i]->GetType() == ST_OUT_NET_RTMP_4_RTMP){
      BaseProtocol *pProtocol = outStreams[i]->GetProtocol();
      if(pProtocol != NULL){
        pProtocol->EnqueueForDelete();
      }
    }
  }
}

//void QIC32AVCStream::OnNotifyMessage (NOTIFICATION_MSG_TYPE type, string message)
void QIC32AVCStream::OnDeviceMessage (DeviceMessage &msg)
{
  switch(msg.type){
    case VCM_STARTED:
      //_currentTS = 0;
      _isInitVideo = false;
      RequestKeyFrame();
      Play(0, 0);
      break;
    case VCM_STOPPED:
      Stop();
      break;
    case VCM_RESOLUTION_CHANGED:
      //_currentTS = 0;
      UnlinkLiveStream();
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
