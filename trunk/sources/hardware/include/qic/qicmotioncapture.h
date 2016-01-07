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

#ifdef QIC1832

#ifndef _QICMOTIONCAPTURE_H
#define _QICMOTIONCAPTURE_H

#include "hardwaretypes.h"
#include "basehardwareabstractdevice.h"
#include "qic/qic_xuctrl.h"

#include "system/systemmanager.h"


class QICMotionCapture
: public BaseHardwareAbstractDevice
{
  protected:
    static BaseHardwareAbstractDevice *_pDeviceInstance;
    bool _isSetFormat;
    uint32_t _fcnt;
    uint32_t _lfcnt;
    double _lastSendTime;
    uint32_t _errCount;
    uint8_t _FPSControl;
    unsigned long _CtrlTs;

    virtual bool Close();
  protected:
    QICMotionCapture(int32_t fd);
    bool rectEqual(rect_t rect1, rect_t rect2);

  public:
    virtual ~QICMotionCapture();
    bool StartCapture();
    bool StopCapture();
    bool GetROIINFO(Variant &getMessage);
    bool SetROIINFO(Variant setMessage);
    bool SetFPS(uint8_t fpsCtrl);
    
    int getMotion(rect_t *motion, unsigned long &ts);

    // Device singleton Interface
    static BaseHardwareAbstractDevice* GetInstance(string devNode);

    virtual void CheckStatus(double ts);

    // BaseHardwareAbstractDevice
    virtual bool OnReadEvent();
    virtual bool InitializeDevice();

    virtual uint8_t CreateMotionStream(StreamsManager *pSM, string streamName);

};
#endif /* _QICMOTIONCAPTURE_H */

#endif
