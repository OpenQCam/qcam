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
 *  Purpose : MJPG Interface class
 * ==================================================================
 */

#include "video/basemjpgvideocapture.h"


BaseMJPGVideoCapture::BaseMJPGVideoCapture(HardwareType hwType, int32_t deviceFD)
: BaseV4L2VideoCapture(hwType, deviceFD)
{
}

BaseMJPGVideoCapture::~BaseMJPGVideoCapture() {
}
