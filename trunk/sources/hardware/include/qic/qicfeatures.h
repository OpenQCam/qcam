/*
// ==================================================================
// This confidential and proprietary software may be used only as
// authorized by a licensing agreement from Quanta Computer Inc.
//
//            (C) COPYRIGHT Quanta Computer Inc.
//                   ALL RIGHTS RESERVED
//
// ==================================================================
// ------------------------------------------------------------------
// Date    : 2013/07/18
// Version :
// Author  : Jennifer Chao (jennifer.chao@quantatw.com)
// ------------------------------------------------------------------
// Purpose : qic camera feature limitation definition
// ==================================================================
*/

#ifndef _QICFEATURE_H
#define _QICFEATURE_H
namespace app_qicstreamer {

#define QIC_WIDTH_MAX                          1280
#define QIC_WIDTH_MIN                          160

#define QIC_HEIGHT_MAX                         720
#define QIC_HEIGHT_MIN                         90

#define QIC_FRAMERATE_MAX                      30
#define QIC_FRAMERATE_MIN                      1

#define QIC_BITRATE_MAX                        5000000
#define QIC_BITRATE_MIN                        20000

#define QIC_GOP_MAX                            120
#define QIC_GOP_MIN                            5

#define QIC_BRIGHTNESS_MAX                       255
#define QIC_BRIGHTNESS_MIN                     -255

#define QIC_CONTRAST_MAX                       68
#define QIC_CONTRAST_MIN                       0

#define QIC_HUE_MAX                            21
#define QIC_HUE_MIN                            -15

#define QIC_SATURATION_MAX                     63
#define QIC_SATURATION_MIN                     0

#define QIC_SHARPNESS_MAX                      11
#define QIC_SHARPNESS_MIN                      0

#define QIC_GAMMA_MAX                          26
#define QIC_GAMMA_MIN                          1

#define QIC_AUTO_WHITE_BALANCE_MAX             1
#define QIC_AUTO_WHITE_BALANCE_MIN             0

#define QIC_WHITE_BALANCE_TEMPERATURE_MAX      6500
#define QIC_WHITE_BALANCE_TEMPERATURE_MIN      2800

#define QIC_BACKLIGHT_COMPENSATION_MAX         9
#define QIC_BACKLIGHT_COMPENSATION_MIN         0

#define QIC_POWERLINE_COMPENSATION_MAX         2
#define QIC_POWERLINE_COMPENSATION_MIN         0

#define QIC_AUTO_EXPOSURE_MAX                  3
#define QIC_AUTO_EXPOSURE_MIN                  0

#define QIC_EXPOSURE_RELATIVE_MAX              1250
#define QIC_EXPOSURE_RELATIVE_MIN              2

#define QIC_FOCUS_ABSOLUTE_MAX                 68
#define QIC_FOCUS_ABSOLUTE_MIN                 1

}
#endif

