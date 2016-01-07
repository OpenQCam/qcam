#include "streaming/outjpgstream.h"
//thelib
#include "streaming/streamstypes.h"
#include "streaming/streamsmanager.h"
#include "system/eventdefine.h"
#include "application/baseclientapplication.h"

OutJPGStream::OutJPGStream(StreamsManager *pStreamsManager, string name)
  : BaseOutStream(NULL, pStreamsManager, ST_OUT_JPG, name) {
}

OutJPGStream::~OutJPGStream() {
}

bool OutJPGStream::CheckJPGStatus() {
  uint64_t curTime;
  GETTIMESTAMP(curTime);
  list<uint64_t>::iterator i=_capList.begin();

  while (i != _capList.end()) {
    if ((curTime - LIST_VAL(i)) > 20000000) {
      _capList.erase(i++);
    }
    else
      break;
  }
  return true;
}

bool OutJPGStream::SnapShot(uint32_t delay) {
  double ts;

  GETTIMESTAMP(ts);
  uint64_t capTime= ts+delay;
  list<uint64_t>::iterator it=_capList.begin();

  while (it != _capList.end()) {
    if (LIST_VAL(it)> capTime) {
      break;
    }
    it++;
  }
  _capList.insert(it, capTime);

  FOR_LIST_ITERATOR(uint64_t, _capList, j) {
    DEBUG ("cap file:%llu", LIST_VAL(j));
  }
  return true;
}

bool OutJPGStream::FeedData (uint8_t *pData, uint32_t dataLength,
                             uint32_t procLength, uint32_t totalLength,
                             double absTs, bool isAudio) {

  if (_capList.empty()) {
    return true;
  }
  BaseClientApplication *pApp=
      const_cast<BaseClientApplication*>(GetStreamsManager()->GetApplication());
  list<uint64_t>::iterator i=_capList.begin();

  while (i != _capList.end()) {
    if (absTs<LIST_VAL(i)) {
      break;
    }
    else {
      uint8_t payload[4]={0};
      pApp->OnNotifyData(DATA_JPG, LIST_VAL(i), payload, 4, pData, dataLength);
      _capList.erase(i++);
    }
  }
  return true;
}

void OutJPGStream::SignalAttachedToInStream() {
  NYI;
}

void OutJPGStream::SignalDetachedFromInStream() {
  NYI;
}

void OutJPGStream::SignalStreamCompleted() {
  NYI;
}

bool OutJPGStream::SignalPlay(double &absoluteTimestamp, double &length) {
  NYI;
  return true;
}

bool OutJPGStream::SignalPause() {
  NYI;
  return true;
}

bool OutJPGStream::SignalResume() {
  NYI;
  return true;
}

bool OutJPGStream::SignalSeek(double &absoluteTimestamp) {
  NYI;
  return true;
}

bool OutJPGStream::SignalStop() {
  NYI;
  return true;
}

bool OutJPGStream::IsCompatibleWithType(uint64_t type) {
  NYI;
  return true;
}
