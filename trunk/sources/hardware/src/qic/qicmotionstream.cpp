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

#include "qic/qicmotionstream.h"
#include "streaming/baseoutstream.h"
#include "protocols/baseprotocol.h"
#include "hardwaremanager.h"
#include "video/videodefine.h"
#include "qic/qic32avcvideocapture.h"
#include "protocols/avhttpstream/avstreamdefine.h"
#include "system/eventdefine.h"
#include "netio/select/uxdomainmanager.h"

QICMotionStream::QICMotionStream(StreamsManager *pStreamsManager, string name)
: BaseInStream(NULL, pStreamsManager, ST_IN_CAM_MOTION, name),
  _isInitVideo(false),
  _currentTS(0),
  _lastFrameTS(0),
  _pMotionCaptureInstance(0),
  _status(VSS_UNKNOWN)
{
  _pMotionCaptureInstance = (QICMotionCapture *)HardwareManager::GetHardwareInstance(HT_VIDEO_MOTION);
  _pSystemInfoStream = new InMSGStream(GetStreamsManager(), QIC_STREAM_SYSTEMMSG);
}

QICMotionStream::~QICMotionStream()
{
}

bool QICMotionStream::IsCompatibleWithType(uint64_t type) {
  return TAG_KIND_OF(type, ST_OUT_NET_RTMP_4_RTMP)
      || TAG_KIND_OF(type, ST_OUT_FILE_RTMP)
      || TAG_KIND_OF(type, ST_OUT_NET_RTP)
      || TAG_KIND_OF(type, ST_OUT_FILE_MP4)
      || TAG_KIND_OF(type, ST_OUT_NET_RAW)
      || TAG_KIND_OF(type, ST_OUT_RECORD_AVC)
      ;
}


void QICMotionStream::OnDeviceData(DeviceData &Ddata)
{
  uint8_t motionCount=Ddata.length/4;

  uint32_t myx=uint32_t(Ddata.buffer[0])*16;
  uint32_t myy=uint32_t(Ddata.buffer[1])*16;
  uint32_t myw=uint32_t(Ddata.buffer[2])*16;
  uint32_t myh=uint32_t(Ddata.buffer[3])*16;


  Variant myOutputROI;
  for(uint8_t validROICount=0;validROICount<motionCount;validROICount++) {
    myOutputROI["ROIs"][validROICount]["x"]=myx;
    myOutputROI["ROIs"][validROICount]["y"]=myy;
    myOutputROI["ROIs"][validROICount]["width"]=myw;
    myOutputROI["ROIs"][validROICount]["height"]=myh;
  }
  myOutputROI["ROIs"].IsArray(true);
  myOutputROI["ROIcount"]=motionCount;

  uint16_t subType=0x201;
  uint64_t eventInfo;
  string payload;
  myOutputROI.SerializeToJSON(payload);;
INFO("myobj is %s",STR(payload));
  UnixDomainSocketManager::SendResponseToIpcEvent(payload);

  if (_pSystemInfoStream) {
    IOBuffer data;
    uint32_t msgtype=(ENTOHS(subType)<<16)|ENTOHS(CLOUD_MSG_EVENT);
    data.ReadFromBuffer((uint8_t*)&msgtype, sizeof(msgtype));
    data.ReadFromBuffer((uint8_t*)&eventInfo, sizeof(eventInfo));
    data.ReadFromBuffer((uint8_t*)STR(payload), payload.length());
    _pSystemInfoStream->FeedMSGData(GETIBPOINTER(data), GETAVAILABLEBYTESCOUNT(data), false);
  }

}

void QICMotionStream::ResendSPSPPS()
{

  //double ats;
  LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;
  while(pTemp != NULL){
    //_pCameraInstance->SetGOP(30); // FIXME: Iframe will lose even we request I frame because of udp protocol.
    if(pTemp->info->GetType() == ST_OUT_NET_RTP){
      /*if(!pTemp->info->FeedData(_pStreamCapabilities->avc._pSPS, _pStreamCapabilities->avc._spsLength, 0, _pStreamCapabilities->avc._spsLength, _currentTS, false)){
        FATAL("Unable to feed OS: %p", pTemp->info);
      }
      if(!pTemp->info->FeedData(_pStreamCapabilities->avc._pPPS, _pStreamCapabilities->avc._ppsLength, 0, _pStreamCapabilities->avc._ppsLength, _currentTS, false)){
        FATAL("Unable to feed OS: %p", pTemp->info);
      }*/
      //if(!pTemp->info->FeedData(pBuf, length, 0, length, ats, false)){
      //  FATAL("Unable to feed OS: %p", pTemp->info);
      //}
    }else if(pTemp->info->GetType() == ST_OUT_NET_RTMP_4_RTMP){

      //_dataBuffer.ReadFromBuffer(RTMPEOSHeader.header, RTMPEOSHeader.length);
      if(!pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false)){
        FATAL("Unable to feed OS: %p", pTemp->info);
      }
      _dataBuffer.IgnoreAll();

      //_dataBuffer.ReadFromBuffer(RTMPInitHeader.header, RTMPInitHeader.length);
      _dataBuffer.ReadFromByte(0x1);
      //_dataBuffer.ReadFromByte(uint8_t ((_pStreamCapabilities->avc._spsLength>>8) & 0xFF));
      //_dataBuffer.ReadFromByte(uint8_t (_pStreamCapabilities->avc._spsLength & 0xFF));
      //_dataBuffer.ReadFromBuffer(_pStreamCapabilities->avc._pSPS, _pStreamCapabilities->avc._spsLength);
      _dataBuffer.ReadFromByte(0x1);
      //_dataBuffer.ReadFromByte(uint8_t ((_pStreamCapabilities->avc._ppsLength>>8) & 0xFF));
      //_dataBuffer.ReadFromByte(uint8_t (_pStreamCapabilities->avc._ppsLength & 0xFF));
      //_dataBuffer.ReadFromBuffer(_pStreamCapabilities->avc._pPPS, _pStreamCapabilities->avc._ppsLength);

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
/*
void QICMotionStream::SignalOutStreamAttached(BaseOutStream *pOutStream)
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


void QICMotionStream::SignalOutStreamDetached(BaseOutStream *pOutStream)
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

*/

void QICMotionStream::RequestKeyFrame()
{
  //FIXME(Tim): RequestKeyFrame should rely on handwaremanager to get related instance
  /*if(_pAVCCaptureInstance->IsStarted()){
    _requestKeyFrame = true;
    _pAVCCaptureInstance->GetKeyFrame();
  }*/
}

void QICMotionStream::UnlinkLiveStream() {
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
void QICMotionStream::OnDeviceMessage (DeviceMessage &msg)
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

StreamCapabilities * QICMotionStream::GetCapabilities()
{
  return _pStreamCapabilities;
}

VideoStreamStatus QICMotionStream::GetStatus()
{
  return _status;
}

bool QICMotionStream::SignalPlay(double &absoluteTimestamp, double &length)
{
  _status = VSS_START;
  return true;
}
bool QICMotionStream::SignalPause()
{
  _status = VSS_STOP;
  return true;
}
bool QICMotionStream::SignalResume()
{
  _status = VSS_START;
  return true;
}
bool QICMotionStream::SignalSeek(double &absoluteTimestamp)
{
  return true;
}
bool QICMotionStream::SignalStop()
{
  _status = VSS_STOP;
  return true;
}
bool QICMotionStream::FeedData(uint8_t *pData, uint32_t dataLength,
    uint32_t processedLength, uint32_t totalLength,
    double absoluteTimestamp, bool isAudio)
{
  return true;
}

void QICMotionStream::ReadyForSend()
{
}

void QICMotionStream::SignalOutStreamAttached(BaseOutStream *pOutStream)
{
}
void QICMotionStream::SignalOutStreamDetached(BaseOutStream *pOutStream)
{
}

#endif
