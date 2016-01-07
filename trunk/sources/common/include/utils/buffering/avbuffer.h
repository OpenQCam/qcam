/* A light weight buffer for audio/video recording with timestamp */


#ifndef _AVBUFFER_H
#define _AVBUFFER_H
#include "platform/platform.h"
#include "utils/misc/file.h"
#include "utils/misc/mmapfile.h"
#include "utils/misc/crypto.h"


//#define GETAVAILABLEBYTESCOUNT(x)     ((x)._published - (x)._consumed)
//#define GETIBPOINTER(x)     ((uint8_t *)((x)._pBuffer + (x)._consumed))

class DLLEXP AVBuffer {
public:
  uint8_t *_pBuffer;
  uint32_t _size;
  uint32_t _published;
  uint32_t _consumed;
  uint32_t _minChunkSize;
  double _timeStamp;
  bool _isKeyframe;
  bool _isAudio;

public:
  AVBuffer();
  AVBuffer(uint32_t bufSize, double timeStamp, bool isKeyframe, bool isAudio);
  virtual ~AVBuffer();

  void Initialize(uint32_t expected);
  bool ReadFromeBuffer(const uint8_t *pBuffer, const uint32_t size, bool isKeyframe);
  bool IgnoreAll ();
  bool MoveData();
  bool EnsureSize(uint32_t expected);
  bool IsKeyFrame() { return _isKeyframe;}
  bool IsAudio() { return _isAudio;}
  uint32_t getBufferSize() { return _size;}
  double getTimeStamp() { return _timeStamp;}

protected:
  void Cleanup();
  void Recycle();
};
#endif /* _AVBUFFER_H */
