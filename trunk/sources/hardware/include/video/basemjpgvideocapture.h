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
 *  Author  : Jennifer Chao (jennifer.chao@quantatw.com)
 *  Purpose : Base MJPG Video Capture Class
 * ==================================================================
 */

#ifndef _BASEMJPGVIDEOCAPTURE_H
#define _BASEMJPGVIDEOCAPTURE_H

#include "video/basev4l2videocapture.h"

class BaseMJPGVideoCapture
: public BaseV4L2VideoCapture
{
  protected:
    BaseMJPGVideoCapture(HardwareType hwType, int32_t deviceFD);

  public:
    virtual ~BaseMJPGVideoCapture();

    virtual bool VerifyResolution(uint16_t width, uint16_t height) = 0;

    virtual bool SetDefaultResolution(string width, string height) = 0;
    virtual Variant GetDefaultResolution() = 0;

    virtual Variant GetConstraints() = 0;
};
#endif /* _BASEMJPGVIDEOCAPTURE_H */

