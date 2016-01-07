
#ifndef _SPEAKER_H
#define _SPEAKER_H

#include "common.h"
#include <sys/ioctl.h>
#include <sys/socket.h>
//#include <char/i2c_drv.h>
//#include <char/i2s/i2s_ctrl.h>
#include <fcntl.h>
#include <signal.h>
#include <fstream>
#include "audio/baseaudiodevice.h"

typedef enum {
  ST_SPK_CLOSED,
  ST_SPK_START_SKIP,
  ST_SPK_START_NORMAL,
  ST_SPK_WAIT_STOPPED,
  ST_SPK_STOPPED,
} ST_SPEAKER;

class PCMPlayStream;
class AudioFileStream;
class BaseInStream;
class StreamsManager;

class Speaker
: public BaseAudioDevice {

  private:
    static BaseHardwareAbstractDevice *_pDeviceInstance;

    PCMPlayStream* _pPlayStream;
    AudioFileStream* _pFileStream;
    uint8_t _txbuffer[3072];
    uint8_t _dummy[3072];
    int32_t _skipFrame;
    int32_t _dummyFrame;
    ST_SPEAKER _state;
    uint32_t _count;
    bool _bReadyToStop;
    bool _lastMicStatus;

  private:
    Speaker(int32_t fd);

  public:
    // Device singleton Interface
    static BaseHardwareAbstractDevice* GetInstance(string devNode);
    virtual ~Speaker();

    // BaseHardwareAbstractDevice
    virtual bool InitializeDevice();
    virtual bool OnWriteEvent();
    virtual bool IsStart();
    virtual bool Open();
    virtual bool Close();
    virtual void Start();
    virtual void Stop();
    virtual int32_t PutData(uint8_t *pData, uint32_t size);
    virtual void CheckStatus(double ts);

    //for play device only
    virtual bool Play();
    virtual void SetVolume(uint32_t volume);
    virtual int32_t GetVolume();
    virtual bool LinkFileStream(string fullName);
    virtual bool SignalStop();
    virtual bool LinkNetStream(BaseInStream *pStream);
    virtual bool CreateStream(StreamsManager* pStreamManager);
    virtual uint32_t GetAudioFrameSize();
};

#endif  /* _SPEAKER_H */
