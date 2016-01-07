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

#include "qic/qicmjpgstream.h"
#include "video/videodefine.h"
#include "streaming/streamstypes.h"
#include "streaming/baseoutstream.h"

QICMJPGStream::QICMJPGStream(StreamsManager *pStreamsManager, string name)
: BaseVideoStream(pStreamsManager, ST_IN_CAM_MJPG, name)
{
  _pStreamCapabilities = new StreamCapabilities();
  _pStreamCapabilities->InitVideoMJPG();
}

QICMJPGStream::~QICMJPGStream() {
  delete _pStreamCapabilities;
}

StreamCapabilities * QICMJPGStream::GetCapabilities()
{
  return _pStreamCapabilities;
}


// QIC Data Callback
//void QICMJPGStream::FeedData (IOBuffer *buf, struct timeval *ts)
//bool QICMJPGStream::FeedData(IOBuffer &buf, struct timeval &ts)

void QICMJPGStream::OnDeviceData(DeviceData &data)
{
  //INFO("Get MJPG data %d", data.length);

  if(_status == VSS_STOP) return;
  LinkedListNode<BaseOutStream *> *pTemp;
  pTemp = _pOutStreams;
  while (pTemp != NULL) {
    switch(pTemp->info->GetType()){
      case ST_OUT_FILE_JPG:
      case ST_OUT_JPG:
      case ST_OUT_NET_MJPGHTTP:
      case ST_OUT_HTTP_MJPG:
      case ST_OUT_NET_AV:
      case ST_OUT_WSNET_MJPG:
      default:
        if(!pTemp->info->FeedData(data.buffer, data.length, 0, data.length, data.ts, false)){
          FATAL("Unable to feed OS: %p", pTemp->info);
        }
        break;
    } //switch
    pTemp = pTemp->pPrev;
  } // while

  //buf.IgnoreAll();
  //return true;
}

// QIC Message Callback
void QICMJPGStream::OnDeviceMessage (DeviceMessage &msg)
{
  switch(msg.type){
    case VCM_STOPPED:
      Stop();
      break;
   case VCM_STARTED:
      Play(0, 0);
      break;
    case VCM_RESOLUTION_CHANGED:
      break;
    case VCM_FPS_CHANGED:
      break;
    case VCM_PROFILE_CHANGED:
      break;
    default:
    break;
  }
}

bool QICMJPGStream::IsCompatibleWithType(uint64_t type)
{
  return TAG_KIND_OF(type, ST_OUT_NET_MJPGHTTP) ||
      TAG_KIND_OF(type, ST_OUT_HTTP_MJPG)||
      TAG_KIND_OF(type, ST_OUT_HTTP_SNAPSHOT)||
      TAG_KIND_OF(type, ST_OUT_NET_AV)||
      TAG_KIND_OF(type, ST_OUT_WSNET_MJPG)||
      TAG_KIND_OF(type, ST_OUT_FILE_JPG)||
      TAG_KIND_OF(type, ST_OUT_IVA_JPEG)||
      TAG_KIND_OF(type, ST_OUT_JPG);

}

void QICMJPGStream::SignalOutStreamAttached(BaseOutStream *pOutStream)
{
}
void QICMJPGStream::SignalOutStreamDetached(BaseOutStream *pOutStream)
{
}
