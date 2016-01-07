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
 *  =================================================================
 */

#ifndef _VIDEODEFINE_H
#define _VIDEODEFINE_H

#include "common.h"
#include "qic/qic_xuctrl.h"

//typedef limitation Limitation;
#define VIDEO_BRIGHTNESS "brightness"
#define VIDEO_CONTRAST "contrast"
#define VIDEO_HUE "hue"
#define VIDEO_SATURATION "saturation"
#define VIDEO_SHARPNESS "sharpness"
#define VIDEO_GAMMA "gamma"
#define VIDEO_BACKLIGHTCOMPENSATION "backlightCompensation"

#define VIDEO_CONSTRAINT_MAX "max"
#define VIDEO_CONSTRAINT_MIN "min"
#define VIDEO_CONSTRAINT_DEFAULT "dflt"
#define VIDEO_CONSTRAINT_CURRENT "curr"

enum VideoCodecType {
  VC_UNKNOWN = 0,
  VC_MJPG,
  VC_AVC,
  VC_VP8
};

//Move to deviceinterface.h
//enum VideoCaptureMessageType {
//  VCM_STARTED = 0,
//  VCM_STOPPED,
//  VCM_RESOLUTION_CHANGED,
//  VCM_FPS_CHANGED,
//  VCM_BITRATE_CHANGED,
//  VCM_PROFILE_CHANGED,
//  VCM_KEYFRAME_DONE
//};

enum VIDEO_CAPTURE_STATUS {
  VCS_UNKNOWN = 0,
  VCS_IDLE,
  VCS_ON,
  VCS_OFF
};

enum CAMERA_FUNC_CTRL_MODE {
  CAMERA_FUNC_CTRL_EXPOSURE_AUTO      = 0,
  CAMERA_FUNC_CTRL_EXPOSURE_ABSOLUTE  = 1
};

typedef struct {
  signed int max;
  signed int min;
  signed int dflt;
}Limitation;
/*
struct VideoCaptureFilterCapability{
  //struct Limitation brightness;
  //struct Limitation contrast;
  //struct Limitation hue;
  //struct Limitation saturation;
  //struct Limitation sharpness;
  //struct Limitation gamma;
  //struct Limitation backlightCompensation;
  Limitation brightness;
  Limitation contrast;
  Limitation hue;
  Limitation saturation;
  Limitation sharpness;
  Limitation gamma;
  Limitation backlightCompensation;
  //struct Limitation autoWhiteBalance;
  //struct Limitation whiteBalanceTemperature;
  //struct Limitation powerlineCompensation;
  //struct Limitation autoExposure;
  //struct Limitation exposureRelative;
  //struct Limitation focusAbsolute;
  VideoCaptureFilterCapability(){
    memset(&brightness, 0, sizeof(struct Limitation));
    memset(&contrast, 0, sizeof(struct Limitation));
    memset(&hue, 0, sizeof(struct Limitation));
    memset(&saturation, 0, sizeof(struct Limitation));
    memset(&sharpness, 0, sizeof(struct Limitation));
    memset(&gamma, 0, sizeof(struct Limitation));
    memset(&backlightCompensation, 0, sizeof(struct Limitation));
    //memset(&autoWhiteBalance, 0, sizeof(struct Limitation));
    //memset(&whiteBalanceTemperature, 0, sizeof(struct Limitation));
    //memset(&powerlineCompensation, 0, sizeof(struct Limitation));
    //memset(&autoExposure, 0, sizeof(struct Limitation));
    //memset(&exposureRelative, 0, sizeof(struct Limitation));
    //memset(&focusAbsolute, 0, sizeof(struct Limitation));
  }
};
 */
struct VideoConfig {
  uint32_t width;
  uint32_t height;
  uint32_t fps;
  uint32_t bitrate;
  uint32_t gop;

  /*
  int16_t pan;
  int16_t tilt;
  struct v4l2_capability V4L2Capability;
  EncoderStreamFormat_t format;
  EncoderStreamSizeFormat_t sizeFormat;
  VideoCodecType videoCodec;
   */
  VideoConfig() {
    width = 0;
    height = 0;
    fps = 0;
    bitrate = 0;
    gop = 0;

    /*
    videoCodec = VC_UNKNOWN;
    memset(&V4L2Capability, 0, sizeof(struct v4l2_capability));
    memset(&format, 0, sizeof(EncoderStreamFormat_t));
    memset(&sizeFormat, 0, sizeof(EncoderStreamSizeFormat_t));
     */
  }
};
struct VideoCaptureFilters{
  int32_t brightness;
  int32_t contrast;
  int32_t hue;
  int32_t saturation;
  int32_t sharpness;
  int32_t gamma;
  int32_t backlightCompensation;
  //int32_t autoWhiteBalance;
  //int32_t whiteBalanceTemperature;
  //int32_t powerlineCompensation;
  //int32_t autoExposure;
  //int32_t exposureRelative;
  //int32_t focusAbsolute;

  VideoCaptureFilters() {
    brightness = 0;
    contrast = 0;
    hue = 0;
    saturation = 0;
    sharpness = 0;
    gamma = 0;
    backlightCompensation = 0;
  }
};

struct V4L2Buffer {
  void *start;
  size_t length;
};


#endif /* _VIDEODEFINE_H */
