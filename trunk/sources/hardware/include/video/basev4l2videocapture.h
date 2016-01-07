/* ==================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 * (C) COPYRIGHT Quanta Computer Inc. ALL RIGHTS RESERVED
 *
 * ==================================================================
 * ------------------------------------------------------------------
 *  Date    :
 *  Version :
 *  Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 *  Purpose :
 * ==================================================================
 */

#ifndef _BASEV4L2VIDEOCAPTURE_H
#define _BASEV4L2VIDEOCAPTURE_H

#include "system/systemmanager.h"
#include "video/videodefine.h"
#include "qic/videocapturefilterscapability.h"
#include "basehardwareabstractdevice.h"
#include "system/systemdefine.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define REQ_WIDTH     1280
#define REQ_HEIGHT    720

class StreamsManager;
class DeviceVideoStream;

class BaseV4L2VideoCapture
: public BaseHardwareAbstractDevice {
  protected:
    enum { kDefaultBufferCount = 6 };
    enum { kDefaultAVCBufferCount = 20 };

    static VideoCaptureFilters *_pVideoCaptureFilters;
    static VideoCaptureFiltersCapability *_pVideoCaptureFiltersCapability;
    V4L2Buffer *_pV4L2Buffers; // V4L2 buffer array
    VIDEO_CAPTURE_STATUS _status;
    uint32_t _numOfV4L2Buffers;
    uint32_t _reqBufferCount;
    bool _isInit;

    // stream operation functions
    map<uint16_t, DeviceVideoStream *> _deviceVideoStreams;
    map<uint16_t, DeviceVideoStream *> _pCurrentDeviceStream;
    map<uint16_t, DeviceVideoStream *> _pCurrentRelayStream;
    StreamsManager *_pSM;

  protected:
    BaseV4L2VideoCapture(HardwareType hwType, int32_t deviceFD);
    bool InitVideoCaptureFilters();

  public:
    virtual ~BaseV4L2VideoCapture();

    virtual bool InitializeDevice();

    virtual uint8_t CreateVideoStreams(StreamsManager *pSM, string streamName) = 0;
    virtual bool CreateRelayStream(StreamsManager *pSM, string streamName);
    //virtual bool DeleteVideoStreams() = 0;
    virtual bool SelectDeviceStream(string streamName);
    virtual DeviceVideoStream* GetCurrentDeviceStream();
    virtual uint8_t GetNumOfDeviceStreams();

    virtual bool CustomizeVideoSettings() = 0;

    virtual bool StartCapture() = 0;
    virtual bool StopCapture() = 0;

    virtual bool IsStarted();

    // UVC well-defined ISP function
    virtual bool SetResolution(uint32_t width, uint32_t height);
    virtual uint32_t GetWidth();
    virtual uint32_t GetHeight();

    virtual bool SetFrameRate(uint32_t frameRate);
    virtual uint32_t GetFrameRate() = 0;

    virtual bool SetBrightness(int32_t value);
    virtual int32_t GetBrightness();

    virtual bool SetContrast(int32_t value);
    virtual int32_t GetContrast();

    virtual bool SetHUE(int32_t value);
    virtual int32_t GetHUE();

    virtual bool SetSaturation(int32_t value);
    virtual int32_t GetSaturation();

    virtual bool SetSharpness(int32_t value);
    virtual int32_t GetSharpness();

    virtual bool SetGamma(int32_t value);
    virtual int32_t GetGamma();

    virtual bool SetBacklightCompensation(int32_t value);
    virtual int32_t GetBacklightCompensation();

    Variant GetCaptureFiltersConstraint();
    virtual bool SetCaptureFiltersToDefault();
    Variant GetCurrentCaptureFilterValues();
    VideoCaptureFilters* GetCaptureFilters();

    virtual string GetFirmwareVersion();

    virtual bool SetFlip(int8_t value);
    virtual int32_t SetFlipMode(uint8_t vertical, uint8_t horizontal);
    virtual int32_t GetFlipMode(uint8_t *vertical, uint8_t *horizontal);

    virtual bool SetPowerLineFrequency(int32_t value);
    virtual int32_t GetPowerLineFrequency();

    virtual bool SetExposure(uint32_t mode, int32_t value);
    virtual int32_t GetExposure(uint32_t mode);

    // FIXME(Jennifer): gain and zoom are not working, don't use
    virtual bool SetGain(uint32_t mode, int32_t value);
    virtual int32_t GetGain();

    virtual bool SetZoom(int32_t value);
    virtual int32_t GetZoom();

    //virtual bool SetPanTilt(int16_t pan, int16_t tilt);
    //virtual void GetPanTilt(int16_t &pan, int16_t &tilt);
    //TODO(jennifer) : remove this

    virtual bool SetFocus(uint32_t mode, int32_t value);
    virtual int32_t GetFocus();

    virtual bool SetWhiteBalance(uint32_t mode, int32_t value);
    virtual int32_t GetWhiteBalance();

  protected:

    //bool Open();
    virtual bool Close();

    void SetStatus(VIDEO_CAPTURE_STATUS status);
    VIDEO_CAPTURE_STATUS GetStatus();
    /**
    * @brief  V4L2 memory mapping for video buffer. Refer to V4L2 API
    *
    * @returns - true for sucess, otherwise false
    */
    bool InitMmap();

    /**
    * @brief  V4L2 memory ummaping for video buffer
    *
    * @returns - true for success and false for ummap error
    */
    bool UnInitMmap();

    int32_t xioctl(int32_t req, void *arg);
    bool v4l2_s_ctrl(uint32_t id, int32_t value);
    int32_t v4l2_g_ctrl(uint32_t id);
    bool v4l2_query_ctrl(uint32_t id, Limitation *value);
    bool v4l2_querycap(struct v4l2_capability &cap);
    bool v4l2_s_fmt(uint32_t width, uint32_t height, uint32_t fmt);
    bool v4l2_g_fmt(uint32_t fmt);
    bool v4l2_reqbufs(uint32_t &reqBufferCount);
    bool v4l2_querybuf(V4L2Buffer &buffer, uint32_t index);
    bool v4l2_streamon();
    bool v4l2_streamoff();
    bool v4l2_qbuf(struct v4l2_buffer &buf);
    bool v4l2_dqbuf(struct v4l2_buffer &buf);

};

#endif /* _BASEV4L2VIDEOCAPTURE_H */
