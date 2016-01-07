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

#ifndef _QICMJPGVIDEOCAPTURE_H
#define _QICMJPGVIDEOCAPTURE_H

#include "video/basemjpgvideocapture.h"

class QICMJPGStream;

class QICMJPGVideoCapture
: public BaseMJPGVideoCapture
//jennifer
//: public BaseV4L2VideoCapture
{
  private:
    static BaseHardwareAbstractDevice *_pDeviceInstance;
    bool _isSetFormat;
    uint32_t _fcnt;
    uint32_t _lfcnt;

  private:
    QICMJPGVideoCapture(int32_t fd);
    virtual bool Close();

  public:
    virtual ~QICMJPGVideoCapture();

    // Device singleton Interface
    static BaseHardwareAbstractDevice* GetInstance(string devNode);

    // BaseHardwareAbstractDevice
    virtual bool OnReadEvent();
    virtual bool InitializeDevice();

    virtual void CheckStatus(double ts);
    virtual uint8_t CreateVideoStreams(StreamsManager *pSM, string streamName);

    bool CustomizeVideoSettings();

    virtual bool StartCapture();
    virtual bool StopCapture();

    virtual bool VerifyResolution(uint16_t width, uint16_t height);

    virtual bool SetResolution(uint32_t width, uint32_t height);
    bool SetDefaultResolution(string width, string height);
    Variant GetDefaultResolution();

    virtual bool SetFrameRate(uint32_t fps);
    virtual uint32_t GetFrameRate();

    virtual Variant GetConstraints();

};

#endif /* _QICMJPGVIDEOCAPTURE_H */
