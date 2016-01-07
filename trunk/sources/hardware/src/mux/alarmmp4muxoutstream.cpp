#include "mux/alarmmp4muxoutstream.h"
//thelib
#include "common.h"
#include "system/nvramdefine.h"
#include "system/systemdefine.h"
#include "netio/select/iohandlermanager.h"
#include "mediaformats/mp4/writemp4document.h"
//hardware
#include "storage/basestoragedevice.h"
#include "avbuffermanager.h"
//mov writer
#include "mediaformats/qmp4/writemovdocument.h"

#define SECS_THROUGH_1904_TO_1970 2082844800

/* use new mp4 writer to support pcm track */
#define USE_NEW_MP4_WRITER 1

AlarmMP4MuxOutStream::AlarmMP4MuxOutStream(StreamsManager *pStreamsManager,
                                           string name,
                                           PushHandler *pPushHandler)
: BaseMP4MuxOutStream (pStreamsManager, name),
  _videoBufList (MAX_BUFSIZE),
  _pPushHandler(pPushHandler)
{
  uint32_t fileDuration=stringToNum<uint32_t>
      (SystemManager::GetNVRam(NVRAM_EVENT_FILE_DURATION));
  _videoBufList.SetFileDuration(fileDuration);
  _minReqSize=stringToNum<uint64_t>(SystemManager::GetNVRam(NVRAM_EVENT_TMP_MINSIZE));
  _tmpPath=SystemManager::GetNVRam(NVRAM_EVENT_TMP_PATH)+"/";
  _targetPath=SystemManager::GetNVRam(NVRAM_EVENT_PATH)+"/";
#if USE_NEW_MP4_WRITER
  _writeMov = new WriteMovDocument();
#endif
}

AlarmMP4MuxOutStream::~AlarmMP4MuxOutStream() {
#if USE_NEW_MP4_WRITER
  if (_writeMov) {
    delete _writeMov;
  }
#endif
}

bool AlarmMP4MuxOutStream::StartRecord(uint32_t maxFrames, evtfile_info_t info) {
  bool ret=false;

  _firstFrameTime=0;
  _lastFrameTime=0;
  _evtInfo=info;

#if USE_NEW_MP4_WRITER
  _previousVideoTS = 0;

  if (_writeMov) {
    int err = 0;
    time_t currTime = time(NULL) + SECS_THROUGH_1904_TO_1970;

    /*
    ** FIXME
    ** frame rate: 30
    ** reserve 40 seconds duration: 30*40
    */
    _vidTrakInfo.type = QMP4_TRAK_TYPE_VIDEO_AVC;
    _vidTrakInfo.info.general.max_packets = 30*40;
    _vidTrakInfo.info.general.creation_sec_time_since_1904 = currTime;
    _vidTrakInfo.info.general.sample_time_scale = 1000;
    _vidTrakInfo.info.general.samples_per_chunk = 1;
    _vidTrakInfo.info.general.track_id = 1;
    _vidTrakInfo.info.video.width = _pVideoStream->GetCapabilities()->avc._width;
    _vidTrakInfo.info.video.height = _pVideoStream->GetCapabilities()->avc._height;

    /*
    ** FIXME
    ** sample rate: 22050
    ** sample number for each xfer chunk: 576
    ** reserve 40 seconds duration: 22050*40/576
    */
    _audTrakInfo.type = QMP4_TRAK_TYPE_AUDIO_PCM;
    _audTrakInfo.info.general.max_packets = 44100*40/512;//22050*40/576;
    _audTrakInfo.info.general.creation_sec_time_since_1904 = currTime;
    _audTrakInfo.info.general.sample_time_scale = 44100;//22050
    _audTrakInfo.info.general.samples_per_chunk = 512;//576;
    _audTrakInfo.info.general.track_id = 2;
    _audTrakInfo.info.audio.channel_num = 2;

    err = _writeMov->Open(&_vidTrakInfo,
                          &_audTrakInfo,
                          QMP4_FILE_TYPE_MOV,
                          STR(_tmpPath+_evtInfo.fileName));

    if (_pVideoStream &&
        _pVideoStream->GetCapabilities()->avc._pSPS &&
        _pVideoStream->GetCapabilities()->avc._pPPS) {
      _writeMov->UpdateSPSR(_pVideoStream->GetCapabilities()->avc._pSPS,
                            _pVideoStream->GetCapabilities()->avc._spsLength,
                            _pVideoStream->GetCapabilities()->avc._pPPS,
                            _pVideoStream->GetCapabilities()->avc._ppsLength);
    } else {
      DEBUG("no registered VideoStream");
    }

    if (!err) {
      ret = true;
    }
    else {
      DEBUG("err:%d", err);
    }
  }
#else
  ret= BaseMP4MuxOutStream::StartRecord(_tmpPath+_evtInfo.fileName, maxFrames);
#endif
  return ret;
}

bool AlarmMP4MuxOutStream::StopRecord() {
  bool ret=false;

#if USE_NEW_MP4_WRITER
  if (_writeMov) {
    _writeMov->Close();
  }
#else
  UpdateInfo(time(NULL));
  if (_mp4file) {
    _mp4file->Close();
    if (_updated) {
      delete _mp4file;
      ret=true;
    }
  }
  _mp4file=NULL;
#endif

  _muxState=MUX_STOP_RECORD;
  return ret;
}

bool AlarmMP4MuxOutStream::FeedData(uint8_t *pData,  uint32_t dataLength,
                                    uint32_t processedLength, uint32_t totalLength,
                                    double absoluteTimestamp, bool isAudio) {
  bool isKeyFrame=false;
  if (dataLength >=8 && ((pData[4]&0x1f)==5) && !isAudio) {
    isKeyFrame = true;
  }

  //Check state and write stream to file
  _videoBufList.PutData(pData, dataLength, absoluteTimestamp, isKeyFrame, isAudio);

  if (_firstFrameTime==0) {
    if (!_videoBufList.GetStartTS(_firstFrameTime))
      return true;
  }

  if (_pPushHandler->IsPushState() && !isAudio) {
    list<AVBuffer*> videoFeedBuffer;
    list<AVBuffer*> videoIBuffer;


    BUFRET_STATUS ret = _videoBufList.GetBufferList(_firstFrameTime, 16*1024, videoFeedBuffer, videoIBuffer);

    if (ret != BUFRET_ST_ERROR) {
      if (ret == BUFRET_ST_UNDERFLOW_TSOVER) {
        DEBUG ("Req I frame in alert stream");
        _pPushHandler->OnStreamEvent(REQ_IFRAME);
      }

      FOR_LIST_ITERATOR(AVBuffer*, videoFeedBuffer, it) {
        AVBuffer* pBuffer= LIST_VAL(it);
        uint8_t* pData = reinterpret_cast<uint8_t*>(GETIBPOINTER(*pBuffer));
        uint32_t dataLength = pBuffer->getBufferSize();
        double absTS = pBuffer->getTimeStamp();

        //DEBUG("%02x %02x %02x %02x %02x %02x %02x %02x, dataLength=%d absTS=%llu",
        //      pData[0], pData[1], pData[2], pData[3],
        //      pData[4], pData[5], pData[6], pData[7],
        //      dataLength, (uint64_t)absTS);

#if USE_NEW_MP4_WRITER
        if (_writeMov) {
          if (pBuffer->IsAudio()) {
            _writeMov->AddAudData(pData, dataLength, dataLength/2);
          } else {
            if (dataLength >=8 && ((pData[4]&0x1f)==5)) {
              isKeyFrame = true;
            } else {
              isKeyFrame = false;
            }

            _writeMov->AddVidData(pData,
                                  dataLength,
                                  _previousVideoTS ? ((uint64_t)absTS - _previousVideoTS) : 0,
                                  isKeyFrame);

            _previousVideoTS = (uint64_t)absTS;
          }
        }
#else
        BaseMP4MuxOutStream::FeedData(pData, dataLength, 0, dataLength,
                                      absTS, pBuffer->IsAudio());
#endif
      }

     if (ret == BUFRET_ST_TS_DONE) {
        Variant info;
        info[INFO_EVT_ID] = _evtInfo.eventId;
        info[INFO_EVT_TYPE] = _evtInfo.eventType;
        info[INFO_SRC_FILE] = _evtInfo.fileName;
        info[INFO_SRC_DIR] = _tmpPath;
        info[INFO_TAR_DIR] = _targetPath;
        info[INFO_TAR] = true;
        //DEBUG ("firstFrameTime:%f, curframeTS:%f", _firstFrameTime, videoFeedBuffer.back()->getTimeStamp());
        _pPushHandler->OnStreamEvent(BUFFER_WRITE_DONE, &info);
      }
    }
    _videoBufList.Recycle(videoFeedBuffer);
  }
  return true;
}

bool AlarmMP4MuxOutStream::GetStartTS(double& ts)
{
  if (_firstFrameTime != 0)  {
    ts = _firstFrameTime;
    return true;
  }
  return false;
}

void AlarmMP4MuxOutStream::SetFileDuration(double duration) {
  _videoBufList.SetFileDuration(duration);
}

double AlarmMP4MuxOutStream::GetFileDuration() {
  return _videoBufList.GetFileDuration();
}

void AlarmMP4MuxOutStream::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                         uint32_t dataLength) {
  BaseMP4MuxOutStream::OnCmdFinished(msgId, pData, dataLength);
  if (_pPushHandler) {
    _pPushHandler->OnStreamEvent(BUFFER_MERGE_DONE);
  }
}

bool AlarmMP4MuxOutStream::SignalPlay(double &absTS, double &length) {
  NYI;
  return true;
}

bool AlarmMP4MuxOutStream::SignalStop() {
  NYI;
  return true;
}

bool AlarmMP4MuxOutStream::SignalAudioPlay() {
  NYI;
  return true;
}

bool AlarmMP4MuxOutStream::SignalAudioStop() {
  NYI;
  return true;
}
