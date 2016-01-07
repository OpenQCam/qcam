#ifndef _PUSHMEDIATOR_H
#define _PUSHMEDIATOR_H

//common
#include "utils/buffering/avbuffer.h"
//thelib
#include "interface/deviceinterface.h"
#include "netio/netio.h"
//hardware
#include "system/systemmanager.h"
#include "gpio/basegpiodevice.h"

#define PRE_VIDEO_DURATION 3 //recording immediately once alert triggered
#define PUSHVIDEO_MAXFRAMES 900
enum ALERT_STATE {
  NO_ALERT = 0x00,
  ALERT_WAIT_TIME,
  ALERT_WRITE_DONE,
  ALERT_WRITING,
};

enum STREAM_EVENT_TYPE {
  BUFFER_WRITE_DONE = 0x00,
  REQ_IFRAME,
  VIDEO_STOPPED,
  VIDEO_STARTED,
  CHANGE_RESOLUTION,
  BUFFER_MERGE_DONE,
  CAP_JPGFILE_FAILURE,
  CAP_JPGFILE_SUCCESS,
  CAP_JPGNET_SUCCESS,
};

class AVBuffer;
class BaseVideoStream;
class BaseAudioStream;
class AlarmMP4MuxOutStream;
class OutJPGFileStream;
class StreamsManager;
class BaseStorageDevice;
class BaseMemoryDevice;

typedef struct _evtfile_info_t {
  uint16_t eventType;
  string eventId;
  string fileName;
} evtfile_info_t;


//Carries the information about event alarm
typedef struct _pushinfo_t {
    ALERT_STATE alarmState;
    int32_t alarmTick;
    int32_t preDuration;
    int32_t alarmDuration;
    uint16_t minorJPGNo;
    uint16_t minorVideoNo;
    bool enabled;
    string eventId;
    uint16_t eventType;

    _pushinfo_t(void) {
      enabled=false;
      alarmState=NO_ALERT;
      preDuration=PRE_VIDEO_DURATION;
      minorJPGNo=0;
      minorVideoNo=0;
    }

    void Reset() {
      alarmState=NO_ALERT;
      minorJPGNo=0;
      minorVideoNo=0;
      alarmTick=alarmDuration;
      preDuration=PRE_VIDEO_DURATION;
    }
 } pushinfo_t;

class DLLEXP PushHandler
: public IOTimer,
  public IDeviceObserver,
  public ISystemCommandCallback {

  private:
    AlarmMP4MuxOutStream *_pMP4Stream;
    OutJPGFileStream *_pJPGFileStream;
    pushinfo_t _pushInfo;
    string _tmpPath;
    string _tmpDirPath;
    string _dirMetaPath;
    string _targetPath;
    uint32_t _maxVideoClips;
    BaseStorageDevice *_pStorageDevice;
    BaseMemoryDevice *_pMemoryDevice;
    uint64_t _minReqSize;
    uint32_t _firstDuration;
    uint32_t _secondDuration;
    uint32_t _lastMSGId;
    bool IsHardwareReady();
    bool CanTriggerAlarm();

  public:
    PushHandler (StreamsManager *pStreamManager, BaseVideoStream *pAVCStream,
                 BaseVideoStream *pMJPGInStream);
    virtual ~PushHandler();
    void EnableEvent();
    void DisableEvent();
    bool CheckScheduleStatus(uint32_t wday, uint32_t hour, string nvramKey);
    virtual bool Initialize();
    virtual bool OnEvent(select_event &event);
    virtual bool OnStreamEvent(STREAM_EVENT_TYPE event, Variant* info=NULL);
    virtual bool IsPushState();
    virtual void OnDeviceMessage(DeviceMessage &msg);
    virtual void OnCmdFinished(uint32_t msgId, uint8_t *pData, uint32_t length);

    void SetFileDuration(uint32_t fileDuration);
    uint32_t GetFileDuration();

};
#endif /* _PUSHMEDIATOR_H */
