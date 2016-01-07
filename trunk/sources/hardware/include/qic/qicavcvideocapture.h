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

#ifdef QIC1816

#ifndef _QICAVCVIDEOCAPTURE_H
#define _QICAVCVIDEOCAPTURE_H

#include "video/baseavcvideocapture.h"

class QICAVCVideoCapture
: public BaseAVCVideoCapture
{
  private:
    static BaseHardwareAbstractDevice *_pDeviceInstance;
  private:
    QICAVCVideoCapture(int32_t fd);

  public:
    virtual ~QICAVCVideoCapture();

    // Device singleton Interface
    static BaseHardwareAbstractDevice* GetInstance(string devNode);
    // BaseHardwareAbstractDevice
    virtual bool OnReadEvent();
    virtual bool InitializeDevice();

    //virtual BaseVideoStream* CreateVideoStream(StreamsManager *pSM, string streamName, uint16_t deviceStreamId);
    virtual uint8_t CreateVideoStreams(StreamsManager *pSM, string streamName);
    virtual bool StartCapture();
    virtual bool StopCapture();

    virtual bool SetResolution(uint32_t width, uint32_t height);
    virtual bool SetFrameRate(uint32_t fps);

    virtual bool SetPanTilt(int16_t pan, int16_t tilt);
    virtual void GetPanTilt(int16_t &pan, int16_t &tilt);


    //uint8_t SetSliceSize(uint8_t slicesize);
    //uint8_t GetSliceSize();

    // BaseAVCVideoCapture
    virtual bool SetBitRate(uint32_t bitrate);
    virtual uint32_t GetBitRate();
    virtual bool SetGOP(uint32_t gop);
    virtual uint32_t GetGOP();
    virtual bool GetKeyFrame();

    uint32_t GetNumOfProfiles();
    bool GetProfile(int idx, int *level, int *profile, int *constraint_flags);
    bool SetProfileLevel(int level, int profile, int constraint_flags);
    bool GetProfileLevel(int *level, int *profile, int *constraint_flags);

    void GetFirmwareVersion(string &version);



};
#endif /* _QICAVCVIDEOCAPTURE_H */

#endif
