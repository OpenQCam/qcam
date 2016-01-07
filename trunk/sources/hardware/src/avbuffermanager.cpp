
#include "common.h"
#include "avbuffermanager.h"

#define FILEDURATION (_fileDuration-300)

AVBufferListManager::AVBufferListManager (uint32_t freeSize)
: _freeSize(freeSize),
  _numOfKeyFrames(0),
  _startTimeStamp(NULL)
{
}

AVBufferListManager::~AVBufferListManager()
{
  //destory all buffer list
  ClearAll();
}

inline bool AVBufferListManager::RecycleIFrameList()
{
  bool release1stKeyframe = false;

  //recycle all video buffer beginning from keyframe
  FOR_LIST_ITERATOR_NOINC(AVBuffer*, _bufList, it) {
    AVBuffer* pBuffer = LIST_VAL(it);
    //recycle process is done
    if (release1stKeyframe && pBuffer->IsKeyFrame())
      break;
    if (!release1stKeyframe && (!pBuffer->IsKeyFrame())) {
      FATAL ("Error, first frame in video buffer is not key frame");
      return false;
    }

    if (pBuffer->IsKeyFrame() && !release1stKeyframe) {
      release1stKeyframe=true;
      _numOfKeyFrames -= 1;
      _IFrameList.pop_front();
    }
    _freeSize += pBuffer->getBufferSize();
    delete _bufList.front();
    it++;
    _bufList.pop_front();

    if (_bufList.size() == 0) {
      FATAL("_videoBuf.size == 0");
      return false;
    }
  }//FOR_LIST_ITERATOR

  return true;
}

inline AVBuffer* AVBufferListManager::GetNewAVBuffer(uint8_t* pData, uint32_t dataLength,
    double timeStamp, bool isKeyFrame, bool isAudio)
{
  AVBuffer *pBuffer=NULL;
  if (_freeSize >= dataLength) {
    pBuffer = new AVBuffer(dataLength, timeStamp, isKeyFrame, isAudio);
    if (pBuffer != NULL) {
      if (pBuffer->ReadFromeBuffer(pData, dataLength, isKeyFrame)) {
        _freeSize -= dataLength;
      }
      else {
        FATAL ("alloc AVBuffer fail");
        delete pBuffer;
        return NULL;
      }
    }
  }
  else {
    FATAL ("Unable to get a free buffer after buffer freeing");
    return NULL;
  }

  return pBuffer;
}

void AVBufferListManager::SetFileDuration(uint32_t fileDuration) {
  _fileDuration = fileDuration;
}

uint32_t AVBufferListManager::GetFileDuration() {
  return _fileDuration;
}
static uint32_t avBufferCounter=0;
bool AVBufferListManager::PutData (uint8_t* pData, uint32_t dataLength,
                                   double timeStamp, bool isKeyFrame, bool isAudio) {

  if (_bufList.size()==0 && !isKeyFrame) {
    avBufferCounter++;
    if(avBufferCounter%300==0)
    	DEBUG ("skip this frame and wait first key frame");
    return true;
  }

  //check by size
  while (_freeSize< dataLength) {
    //recycle all buffer beginning from keyframe, check number of keyframe
    DEBUG ("keyframes too less:%d, frontTS:%f, RearTS:%f, periodTS:%f", _numOfKeyFrames, _bufList.front()->getTimeStamp(),
        _bufList.back()->getTimeStamp(), _bufList.back()->getTimeStamp()-_bufList.front()->getTimeStamp());

    if (_numOfKeyFrames <= 1) {
      FOR_LIST_ITERATOR_NOINC(AVBuffer*, _bufList, it) {
        ++it;
        if (it == _bufList.end())
          break;

        AVBuffer* pBuffer = LIST_VAL(it);
        _freeSize += pBuffer->getBufferSize();
        delete pBuffer;
        it = _bufList.erase(it);
      }
    }
    else {
      if (!RecycleIFrameList())
        return false;
    }
  }

  //check by timestamp
  if (_IFrameList.size() >= 2) {
    list<AVBuffer*>::iterator it=_IFrameList.begin();
    advance (it, 1);
    double localStartTS= LIST_VAL(it)->getTimeStamp();
    if ((timeStamp - localStartTS) > FILEDURATION) {
      if (!RecycleIFrameList())
        return false;
    }
  }

  //append buffer to buffer list
  AVBuffer *pBuffer = GetNewAVBuffer(pData, dataLength, timeStamp, isKeyFrame, isAudio);
  if (pBuffer) {
    _bufList.push_back(pBuffer);
    if (isKeyFrame) {
      _numOfKeyFrames++;
      _IFrameList.push_back(pBuffer);
      //DEBUG ("_timeStampList size:%d", _timeStampList.size());
    };
  }
  return true;
}

void AVBufferListManager::Recycle(list<AVBuffer*>& recycledBuffer)
{
  while (!recycledBuffer.empty()) {
    AVBuffer *pAVBuffer=recycledBuffer.front();
    _freeSize += pAVBuffer->getBufferSize();
    delete pAVBuffer;
    recycledBuffer.pop_front();
  }
}

BUFRET_STATUS AVBufferListManager::GetBufferList (double startTS, uint32_t maxSize, list<AVBuffer*>& feedBufList, list<AVBuffer*>& iBufList)
{
  uint32_t size=0;

  while ( _IFrameList.size()>=2) {
    //Get an buffer list beginning from IFrame
    AVBuffer* pBuffer=_bufList.front();
    AVBuffer* pIBuffer=_IFrameList.front();
    iBufList.push_back(pIBuffer);
    _IFrameList.pop_front();
    size += pBuffer->getBufferSize();
    feedBufList.push_back(pBuffer);
    _bufList.pop_front();

    pBuffer=_bufList.front();
    while (!pBuffer->IsKeyFrame()) {
      size += pBuffer->getBufferSize();
      feedBufList.push_back(pBuffer);
      _bufList.pop_front();
      pBuffer=_bufList.front();
    }

    //prefer ts done than size done
    if ((feedBufList.back()->getTimeStamp() - startTS) >= FILEDURATION) {
      //DEBUG ("feedBufList:%f, startTS:%f, diff:%f", feedBufList.back()->getTimeStamp(), startTS, feedBufList.back()->getTimeStamp() - startTS);
      return BUFRET_ST_TS_DONE;
    }

    if (size>=maxSize)
      return BUFRET_ST_SIZE_DONE;
 }

  //can not get a buffer list, only 1 key frame in buffer list
  //need to get more key frame
  if (!_bufList.empty() && feedBufList.empty()) {
    double sTS=_bufList.front()->getTimeStamp();
    double eTS=_bufList.back()->getTimeStamp();
    if (eTS-sTS >= FILEDURATION)  {
      return BUFRET_ST_UNDERFLOW_TSOVER;
    }
  }

  return BUFRET_ST_UNDERFLOW;
}

bool AVBufferListManager::GetStartTS (double& startTS)
{
  if (!_bufList.empty()) {
    startTS = _bufList.front()->getTimeStamp();
    return true;
  }
  return false;
}


void AVBufferListManager::ClearAll()
{
  while (!_bufList.empty()) {
    delete _bufList.front();
    _bufList.pop_front();
  }
  while (!_IFrameList.empty())
    _IFrameList.pop_front();

  _numOfKeyFrames=0;
  _freeSize = MAX_BUFSIZE;
//  if (_isAudio)
//    _freeSize = MAX_AUDIOSIZE;

  DEBUG ("buf list size:%d", _bufList.size());
  DEBUG ("timestamp lsit size:%d", _IFrameList.size());
}
