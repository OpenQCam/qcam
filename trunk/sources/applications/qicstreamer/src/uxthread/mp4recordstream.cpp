/*
 *  Copyright (C) 2013,  Tim Hsieh (mitmai@gmail.com)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET

#include "uxthread/mp4recordstream.h"
#include "uxthread/udsrecordthread.h"
#include "protocols/unixdomainsocket/udsrecordprotocol.h"
#include "streaming/streamstypes.h"
#include "streaming/baseinstream.h"
#include "streaming/streamsmanager.h"
#include "system/thread/threadmanager.h"

using namespace app_qicstreamer;

MP4RecordStream::MP4RecordStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager,
    string name, uint32_t threadId)
  : BaseOutStream (pProtocol, pStreamsManager, ST_OUT_RECORD_MP4, name)
  , _threadId(threadId)
{
  _pUDSProtocol = (UDSRecordProtocol*) pProtocol;
}


MP4RecordStream:: ~MP4RecordStream()
{
}

void MP4RecordStream::SignalAttachedToInStream() {
}

void MP4RecordStream::SignalDetachedFromInStream() {
}

void MP4RecordStream::SignalStreamCompleted() {
}

bool MP4RecordStream::IsCompatibleWithType(uint64_t type) {
  return true;
}

bool MP4RecordStream::FeedData(uint8_t *pData, uint32_t dataLength, uint32_t processedLength,
    uint32_t totalLength, double absoluteTimestamp, bool isAudio) {

  _currentTS = absoluteTimestamp;
  if(_startRecordingTS == 0) {
    _startRecordingTS = _currentTS;
  }
  if(isAudio) {
    return _pUDSProtocol->SendData(UDS_AUDIO, pData, dataLength, absoluteTimestamp);
  }
  return _pUDSProtocol->SendData(UDS_VIDEO, pData, dataLength, absoluteTimestamp);

}
bool MP4RecordStream::SignalPlay(double &absoluteTimestamp, double &length) {
  NYIR;
}

bool MP4RecordStream::SignalPause() {
  NYIR;
}

bool MP4RecordStream::SignalResume() {
  NYIR;
}

bool MP4RecordStream::SignalSeek(double &absoluteTimestamp) {
  NYIR;
}

bool MP4RecordStream::SignalStop() {
  Variant message;
  message["type"] = UDSRecordThread::CM_STOP;
  return _pUDSProtocol->SendMessage(message);
}


bool MP4RecordStream::OnMessage(BaseUDSMessageProtocol *pProtocol, Variant &message)
{
  UDSRecordThread *pThread = NULL;
  if(message.HasKey("type")) {
    switch((uint8_t) message["type"]) {
      //case UDSRecordThread::MM_THREAD_ID:
      //  if(message.HasKey("id")) {
      //    _threadId = (uint32_t)message["id"];
      //    pThread = (UDSRecordThread*)ThreadManager::FindThreadById(_threadId);
      //    if(pThread == NULL) {
      //      FATAL("Unable to find thread by Id(%d)", _threadId);
      //    }else{
      //      pThread->SetUDSMessageProtocol(pProtocol);
      //    }
      //  }
      //  break;
      //case UDSRecordThread::MM_LINK_STREAM:
      //  if(message.HasKey("streamName") ){
      //    BaseInStream *pInStream =
      //      (BaseInStream*)_pStreamsManager->FindFirstByTypeByName(ST_IN_WSNET_AVC, message["streamName"]);
      //    if(pInStream != NULL){
      //      pInStream->Link(this, true);
      //    } else {
      //      WARN("Unable to find stream(%s)", STR((string)message["streamName"]));
      //    }
      //  }
      //  break;
      case UDSRecordThread::MM_ACK_STOP:
      case UDSRecordThread::MM_ACK_FINISH:
        UnLink(true);
        pThread = (UDSRecordThread*)ThreadManager::FindThreadById(_threadId);
        if(pThread != NULL) {
          pThread->Free();
        }else{
          FATAL("Unable to free this thread(%d)", _threadId);
        }
        break;
      default:
        WARN("Unsupported message %d", (uint8_t)message["type"]);
        break;
    }
  }
  return true;
}

bool MP4RecordStream::OnData(UDS_MESSAGE_TYPE type, uint8_t *pData, uint32_t length, uint64_t ts)
{
  NYIR;
}


bool MP4RecordStream::OnEvent(select_event &event)
{
  INFO("current ts %"PRIu64" start ts %"PRIu64, _currentTS, _startRecordingTS);
  return true;
}

#endif
