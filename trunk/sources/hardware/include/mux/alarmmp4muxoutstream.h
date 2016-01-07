
#ifndef _ALARMMP4MUXOUTSTREAM_H
#define _ALRAMMP4MUXOUTSTREAM_H

//thelib
#include "netio/select/iotimer.h"
//hardware
#include "mux/basemp4muxoutstream.h"
#include "mediaformats/qmp4/writemovdocument.h"
#include "pushmediator.h"
#include "avbuffermanager.h"


class AlarmMP4MuxOutStream
: public BaseMP4MuxOutStream {
private:
  string _tmpPath;
  string _targetPath;
  uint64_t _minReqSize;
  evtfile_info_t _evtInfo;

public:
  AlarmMP4MuxOutStream(StreamsManager *pStreamsManager, string name,
                       PushHandler *pPushHandler);
  virtual ~AlarmMP4MuxOutStream();
  virtual bool StartRecord(uint32_t maxFrames, const evtfile_info_t evtInfo);
  virtual bool StopRecord();
  virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
                        uint32_t processedLength, uint32_t totalLength,
                        double absTS, bool isAudio);

  virtual void OnCmdFinished(uint32_t msgId, uint8_t *pData,
                             uint32_t dataLength);
  bool GetStartTS(double& ts);

  void SetFileDuration(double fileDuration);
  double GetFileDuration();

  virtual bool SignalStop();
  virtual bool SignalPlay(double &absoluteTimestamp, double &length);
  virtual bool SignalAudioStop();
  virtual bool SignalAudioPlay();

private:
  AVBufferListManager _videoBufList;
  PushHandler *_pPushHandler;
  uint32_t _numOfFrames;

  uint64_t _previousVideoTS;
  QMP4_TRAK_INFO _vidTrakInfo;
  QMP4_TRAK_INFO _audTrakInfo;
  WriteMovDocument *_writeMov;
};


#endif //_ALRAMMP4MUXOUTSTREAM_H
