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
 *  Purpose : AVC Interface class
 * ==================================================================
 */

#include <math.h>
#include "video/baseavcvideocapture.h"

#define ONE_SECOUND_EQUIVALENT 10000000

BaseAVCVideoCapture::BaseAVCVideoCapture(HardwareType hwType, int32_t deviceFD)
: BaseV4L2VideoCapture(hwType, deviceFD)
{
  _numOfV4L2Buffers=kDefaultAVCBufferCount;

  uint32_t i;
  // Create array of frame interval mapping from frame rate
  // First element;
  _fpsToFrameInterval.push_back(0);
  _fpsToFrameIntervalForGOP.push_back(0);

  for(i=1; i<31; i++){
    // The unit for this frame interval is in 100ns
    _fpsToFrameInterval.push_back((uint32_t)floor((1.0/i)*ONE_SECOUND_EQUIVALENT));
    // The unit for this frame interval is in ms
    _fpsToFrameIntervalForGOP.push_back((uint16_t)floor((1.0/i)*1000));
  }
}

BaseAVCVideoCapture::~BaseAVCVideoCapture() {
}
