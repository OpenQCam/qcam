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
 *  Purpose : Video Capture Capability Class for all video capture device
 * ==================================================================
 */


#ifndef _VIDEOCAPTURECAPABILITY_H
#define _VIDEOCAPTURECAPABILITY_H

#include "common.h"

#define AVAIL_SUB_STREAMS 4
#define AVAIL_RESOLUTION_SET 8

class VideoConstraints;
typedef struct{
  uint32_t maxBitrate;
  uint32_t minBitrate;
  uint32_t maxFPS;
  uint32_t minFPS;
  uint32_t maxGOP;
  uint32_t minGOP;
}VideoConstraint;

typedef struct{
  uint16_t width;
  uint16_t height;
  string name;
}Bitrate;

typedef struct{
  uint16_t width;
  uint16_t height;
  string name;
}Resolution;

typedef map<uint32_t,VideoConstraint> VideoConstraintMap;
class VideoCaptureCapability
{
  private:
    static const uint8_t _kConstraintIndex[AVAIL_SUB_STREAMS];
    static const uint16_t _kWidthArray[AVAIL_RESOLUTION_SET];
    static const uint16_t _kHeightArray[AVAIL_RESOLUTION_SET];
    static const string _kResolutionNameArray[AVAIL_RESOLUTION_SET];

    static const VideoConstraint videoConstraints[AVAIL_SUB_STREAMS];
    static VideoConstraintMap *_videoConstraintMap;
    VideoConstraints *_pVideoConstraint;

    map<uint32_t,Resolution> _resolution;
    map<uint32_t,Resolution> _deviceResolutionConstraint;

    uint32_t _maxBitrate;
    uint32_t _minBitrate;
    uint32_t _maxFPS;
    uint32_t _minFPS;
    uint32_t _maxGOP;
    uint32_t _minGOP;

    bool _isAVC;
  public:
    VideoCaptureCapability();
    ~VideoCaptureCapability();

    bool InitVideoCapability(uint16_t constraintIndex, bool isAVC);
    Variant GetConstraints();
    void GetResolutionConstraints(Resolution *resArray);
    void GetAVCVideoConstraints(VideoConstraint *otherConstraint);
    bool VerifyResolution(uint16_t width, uint16_t height);
    bool VerifyBitrate(uint32_t bitrate);
    bool VerifyFPS(uint32_t fps);
    bool VerifyGOP(uint32_t gop);

    void SetDeviceVideoConstraint(VideoConstraintMap *videoConstraintMap);
    void PrintDeviceVideoConstraint();
    bool PrintConstraintValues();
};
#endif /* _VIDEOCAPTURECAPABILITY_H */

