#include "outjpgfilestream.h"
//thelib
#include "system/systemdefine.h"
#include "system/nvramdefine.h"
#include "streaming/streamstypes.h"
#include "streaming/streamsmanager.h"
//hardware
#include "pushmediator.h"

OutJPGFileStream::OutJPGFileStream(StreamsManager *pStreamsManager,
                                   PushHandler *pHandler, string name)
  : BaseOutStream(NULL, pStreamsManager, ST_OUT_FILE_JPG, name),
    _bCaptured(true),
    _captureTime(0),
    _eventId(""),
    _eventType("")
{
  _pPushHandler=pHandler;
  _srcDir=SystemManager::GetNVRam(NVRAM_EVENT_TMP_PATH)+"/";
  _tarDir=SystemManager::GetNVRam(NVRAM_EVENT_PATH)+"/";
}

OutJPGFileStream::~OutJPGFileStream() {
}

bool OutJPGFileStream::CheckJPGStatus() {
  if (_bCaptured) {
    return true;
  }
  uint64_t curTime;
  GETTIMESTAMP(curTime);
  if ((curTime - _captureTime) > 200) {
    Variant info;

    info[INFO_EVT_ID]=_eventId;
    info[INFO_EVT_TYPE]=_eventType;
    _pPushHandler->OnStreamEvent(CAP_JPGFILE_FAILURE, &info);
    _bCaptured = true;
    return false;
  }
  return true;
}

bool OutJPGFileStream::SnapShot(uint32_t delay,
                                string file,
                                string eventId,
                                string eventType) {
  double ts;

  GETTIMESTAMP(ts);
  _captureTime= ts+delay;
  _srcFile=file;
  _eventId=eventId;
  _eventType = eventType;
  _bCaptured=false;
  return true;
}

void OutJPGFileStream::SignalAttachedToInStream() {
  NYI;
}

void OutJPGFileStream::SignalDetachedFromInStream() {
  NYI;
}

void OutJPGFileStream::SignalStreamCompleted() {
  NYI;
}

bool OutJPGFileStream::SignalPlay(double &absoluteTimestamp, double &length) {
  NYI;
  return true;
}

bool OutJPGFileStream::SignalPause() {
  NYI;
  return true;
}

bool OutJPGFileStream::SignalResume() {
  NYI;
  return true;
}

bool OutJPGFileStream::SignalSeek(double &absoluteTimestamp) {
  NYI;
  return true;
}

bool OutJPGFileStream::SignalStop() {
  NYI;
  return true;
}

bool OutJPGFileStream::IsCompatibleWithType(uint64_t type) {
  NYI;
  return true;
}

bool OutJPGFileStream::FeedData (uint8_t *pData, uint32_t dataLength,
                                 uint32_t procLength, uint32_t totalLength,
                                 double absTs, bool isAudio) {
  if (_bCaptured)
    return true;

  if (absTs > _captureTime) {
    Variant info;
    info[INFO_EVT_ID]=_eventId;
    info[INFO_EVT_TYPE]=_eventType;
    info[INFO_SRC_FILE]=_srcFile;
    info[INFO_SRC_DIR]=_srcDir;
    info[INFO_TAR_DIR]=_tarDir;
    FILE *fp=fopen(STR(_srcDir+_srcFile), "wb");
    if (fp) {
      fwrite (pData, 1, dataLength, fp);
      fclose(fp);
    }
    else {
      _pPushHandler->OnStreamEvent(CAP_JPGFILE_FAILURE, &info);
      return true;
    }
    _pPushHandler->OnStreamEvent(CAP_JPGFILE_SUCCESS, &info);
    _bCaptured=true;
  }

  return true;
}
