
#ifndef _AVBUFFERMANAGER_H
#define _AVBUFFERMANAGER_H

//common
#include "utils/buffering/avbuffer.h"

#define MAX_BUFSIZE (10*1024*1024)

typedef enum _BUFRET_STATUS{
  BUFRET_ST_SIZE_DONE = 0x00,
  BUFRET_ST_UNDERFLOW = 0x01,
  BUFRET_ST_TS_DONE = 0x02,
  BUFRET_ST_UNDERFLOW_TSOVER = 0x03,
  BUFRET_ST_ERROR = 0xff
} BUFRET_STATUS;

class AVBuffer;

class AVBufferListManager {
  private:
    list<AVBuffer*> _bufList;
    list<AVBuffer*> _IFrameList;
    uint32_t _freeSize;
    uint32_t _numOfKeyFrames;
    AVBuffer* _startTimeStamp;
    uint32_t _fileDuration;
    AVBuffer* GetNewAVBuffer(uint8_t *pData, uint32_t dataLength,
                             double timeStamp, bool isKeyFrame, bool isAudio);
    bool RecycleIFrameList();

  public:
    AVBufferListManager(uint32_t freeSize);
    ~AVBufferListManager();

    bool PutData (uint8_t* pData, uint32_t dataLength, double timeStamp,
                  bool isKeyFrame, bool isAudio);
    BUFRET_STATUS GetBufferList (double startTS, uint32_t maxSize,
                                 list<AVBuffer*>& bufferList,
                                 list<AVBuffer*>& iBufferList);
    bool GetStartTS (double& startTS);
    void Recycle(list<AVBuffer*>& recycledBuffer);
    void ClearAll();
    void SetFileDuration(uint32_t fileDuration);
    uint32_t GetFileDuration();
};

#endif
