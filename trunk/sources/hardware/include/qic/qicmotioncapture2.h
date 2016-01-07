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
 *  Author  : S.J.Hung
 *  Purpose : read motion detection results.
 * ==================================================================
 */

#ifdef QIC1832

#ifndef _QICMOTIONCAPTURE2_H
#define _QICMOTIONCAPTURE2_H

#include "qic/qicmotioncapture.h"


struct MotionBox{
  uint64_t tsCache[256];
  uint8_t  currentIndex;
};

class QICMotionCapture2
: public QICMotionCapture
{
  private:
    uint64_t _eventInMiliSec;
    uint8_t  _eventCountThreshold;
    uint8_t  _eventBufferSize;
    MotionBox _motionBox;
    QICMotionCapture2(int32_t fd);
  public:
    virtual ~QICMotionCapture2();

    uint64_t getEventInMiliSec(void);
    bool     setEventInMiliSec(uint64_t msec);
    uint8_t  getEventCountThreshold(void);
    bool     setEventCountThreshold(uint8_t count);
    uint8_t  getEventBufferSize(void);
    bool     setEventBufferSize(uint8_t size);

    int getMotion(rect_t *motion, unsigned long &ts);

    // Device singleton Interface
    static BaseHardwareAbstractDevice* GetInstance(string devNode, int pipe_read_fd);

    // BaseHardwareAbstractDevice
    virtual bool OnReadEvent();
};
#endif /* _QICMOTIONCAPTURE2_H */

#endif
