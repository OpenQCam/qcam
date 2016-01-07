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


#include "uxthread/udsrecordthread.h"
#include "uxthread/mp4recordstream.h"

#define SECS_THROUGH_1904_TO_1970 2082844800
#define NALU_SPS 0x07
#define NALU_PPS 0x08
#define NALU_IFRAME 0x05
#define NALU_PFRAME 0x01
#define SOCKET_NAME "/tmp/sock.record"

using namespace app_qicstreamer;

UDSRecordThread::UDSRecordThread()
  : BaseUDSThread(THREAD_T_RECORD, SOCKET_NAME)
  , _pRecordStream(NULL)
  , _duration(0)
  , _pMovDocument(0)
  , _lastTS(0)
{
  _pMovDocument = new MP4DocumentWriter(this);
}

UDSRecordThread::~UDSRecordThread()
{
  if(_pMovDocument != NULL) {
    delete _pMovDocument;
  }
}

void UDSRecordThread::SetRecordStream(MP4RecordStream *pStream)
{
  _pRecordStream = pStream;
}

MP4RecordStream* UDSRecordThread::GetRecordStream()
{
  return _pRecordStream;
}

bool UDSRecordThread::StartRecording(Variant &config)
{
  //0. check essential parameters
  if(!config.HasKey("streamName") && !config.HasKey("fileName") &&
     !config.HasKey("duration") && !config.HasKey("videoInfo") &&
     !config.HasKey("audioInfo"))
  {
    WARN("streamName, fileName, duration, videoInfo and audioInfo are needed");
    return false;
  }

  //1. prepare parameters
  //_streamName = (string) config["streamName"];
  _fileName = (string) config["fileName"];
  _duration = (uint32_t) config["duration"];
  if(config.HasKey("iteration")) {
    _iteration = (uint32_t) config["iteration"];
  }else{
    _iteration = 0;
  }
  _lastTS = 0;

  memset(&_streamInfo, 0, sizeof(STREAM_INFO));
  _streamInfo.video.width = (uint16_t) config["videoInfo"]["width"];
  _streamInfo.video.height = (uint16_t) config["videoInfo"]["height"];
  _streamInfo.video.fps = (uint8_t) config["videoInfo"]["fps"];
  _streamInfo.audio.sampleRate = (uint32_t) config["audioInfo"]["sampleRate"];
  _streamInfo.audio.numOfChannels = (uint32_t) config["audioInfo"]["numOfChannels"];

  //2. initialize mp4 file
  return OpenNewMovFile();
}
bool UDSRecordThread::StopRecording()
{
  Variant message;
  message["type"] = UDSRecordThread::CM_STOP;
  return SendMessageToThread(message);
}

bool UDSRecordThread::SwitchToNextFile(string fileName)
{
  Variant message;
  message["type"] = UDSRecordThread::CM_NEXT;
  return SendMessageToThread(message);
}


void UDSRecordThread::OnConnected()
{
  // send message to inform main thread what the thread id is
  Variant message;
  message["type"] = UDSRecordThread::MM_THREAD_CREATED;
  message["id"] = GetId();
  SendMessageToMainThread(message);
}

// deal with all thread messages and ack main thread if needed
bool UDSRecordThread::OnMessage(BaseUDSMessageProtocol *pProtocol, Variant &inMsg)
{
  if(!inMsg.HasKey("type")) {
    return AckMessage(MM_ERROR_UNKNOWN);
  }
  switch((uint8_t)inMsg["type"]) {
    case CM_START:
      //if(OpenNewMovFile()){
      //  return AckMessage(MM_ACK_START);
      //} else {
      //  return AckMessage(MM_ERROR_START);
      //}
      //break;
    case CM_STOP:
      if(_pMovDocument == NULL) {
        return AckMessage(MM_ERROR_STOP);
      }
      if(_pMovDocument->Stop()){
        return AckMessage(MM_ACK_STOP);
      } else {
        return AckMessage(MM_ERROR_STOP);
      }
      break;
    case CM_NEXT:
      if(_pMovDocument == NULL) {
        return AckMessage(MM_ERROR_NEXT);
      }
      if(_pMovDocument->Next()){
        return AckMessage(MM_ACK_NEXT);
      } else {
        return AckMessage(MM_ERROR_NEXT);
      }
      break;
    default:
      return AckMessage(MM_ERROR_UNKNOWN);
      break;
  }
  // acking main thread is necessary
  return true;
}

void UDSRecordThread::OnFinish()
{
  INFO("record finished");
  AckMessage(MM_ACK_FINISH);
}

// ----------------------------------------------------------------------------
// thread private functions
// ----------------------------------------------------------------------------
bool UDSRecordThread::AckMessage(MT_MESSAGE msg)
{
  Variant message;
  message["type"] = msg;
  return SendMessageToMainThread(message);
}

bool UDSRecordThread::OnData(UDS_MESSAGE_TYPE type, uint8_t *pData, uint32_t length, uint64_t ts)
{
  switch(type) {
    case UDS_VIDEO:
      return ProcessVideoData(pData, length, ts);
      break;
    case UDS_AUDIO:
      return ProcessAudioData(pData, length, ts);
      break;
    default:
      WARN("Unknown type %d", type);
      break;
  }
  return false;
}

bool UDSRecordThread::ProcessVideoData(uint8_t *pData, uint32_t length, uint64_t ts)
{
  switch(pData[4] & 0x1F){
    case NALU_IFRAME:
      if(!_pMovDocument->AddVideoData(pData, length, (_lastTS) ? (ts-_lastTS) : 0 , true)){
        return AckMessage(MM_ERROR_VIDEO);
      }
      _lastTS = ts;
      break;
    case NALU_PFRAME:
      if(!_pMovDocument->AddVideoData(pData, length, (_lastTS) ? (ts-_lastTS) : 0 , false)){
        return AckMessage(MM_ERROR_VIDEO);
      }
      _lastTS = ts;
      break;
    case 0x7:
      _streamInfo.video.spsLength = (uint16_t) (length-4);
      memcpy(_streamInfo.video.sps, pData+4, _streamInfo.video.spsLength);
      INFO("SPS");
      break;
    case 0x8:
      _streamInfo.video.ppsLength = (uint16_t) (length-4);
      memcpy(_streamInfo.video.pps, pData+4, _streamInfo.video.ppsLength);
      INFO("PPS");
      if(_streamInfo.video.spsLength != 0 && _streamInfo.video.ppsLength != 0) {
        _pMovDocument->UpdateSPSPPS(_streamInfo.video.sps, _streamInfo.video.spsLength,
            _streamInfo.video.pps, _streamInfo.video.ppsLength);
      }
      break;
    default:
      return AckMessage(MM_ERROR_VIDEO);
      break;
  }
  return true;
}
bool UDSRecordThread::ProcessAudioData(uint8_t *pData, uint32_t length, uint64_t ts)
{
  INFO("audio data %d", length);
  return true;
}

// private functions
bool UDSRecordThread::OpenNewMovFile()
{
  // 1. prepare video and audio track information
  time_t currentTS = time(NULL) + SECS_THROUGH_1904_TO_1970;
  // create video track info
  memset(&_videoTrakInfo, 0, sizeof(QMP4_TRAK_INFO));
  _videoTrakInfo.type = QMP4_TRAK_TYPE_VIDEO_AVC;
  _videoTrakInfo.info.general.max_packets = _streamInfo.video.fps*(_duration+10); // 30fps * record duration
  _videoTrakInfo.info.general.creation_sec_time_since_1904 = currentTS;
  _videoTrakInfo.info.general.sample_time_scale = 1000;
  _videoTrakInfo.info.general.samples_per_chunk = 1;
  _videoTrakInfo.info.general.track_id = 1;
  _videoTrakInfo.info.video.width = _streamInfo.video.width;
  _videoTrakInfo.info.video.height = _streamInfo.video.height;

  // create audio track info
  memset(&_audioTrakInfo, 0, sizeof(QMP4_TRAK_INFO));
  _audioTrakInfo.type = QMP4_TRAK_TYPE_AUDIO_PCM;
  _audioTrakInfo.info.general.creation_sec_time_since_1904 = currentTS;
  _audioTrakInfo.info.general.max_packets = _streamInfo.audio.sampleRate*(_duration/576 + 10);
  _audioTrakInfo.info.general.sample_time_scale = _streamInfo.audio.sampleRate;
  _audioTrakInfo.info.general.samples_per_chunk = 576;
  _audioTrakInfo.info.general.track_id = 2;
  _audioTrakInfo.info.audio.channel_num = _streamInfo.audio.numOfChannels;

  //2. create mov file and initialize
  if(_pMovDocument->Initialize(&_videoTrakInfo, &_audioTrakInfo,
        QMP4_FILE_TYPE_MOV, _fileName, _duration, _iteration) < 0) {
    WARN("Initialize mp4 file failed");
    return false;
  }

  return true;
  // send message to observer so that message observer can hook
  // record stream with mp4 source stream
}

