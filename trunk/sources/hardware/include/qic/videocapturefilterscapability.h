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
 *  Purpose : Video Capture Filters CCapability Class for all video capture device
 * ==================================================================
 */


#ifndef _VIDEOCAPTUREFILTERSCAPABILITY_H
#define _VIDEOCAPTUREFILTERSCAPABILITY_H

#include "video/videodefine.h"

class VideoCaptureFiltersCapability
{
  private:
    Limitation _brightness;
    Limitation _contrast;
    Limitation _hue;
    Limitation _saturation;
    Limitation _sharpness;
    Limitation _gamma;
    Limitation _backlightCompensation;

  public:
    VideoCaptureFiltersCapability(Limitation brightness, Limitation contrast, Limitation hue, Limitation saturation, Limitation sharpness, Limitation gamma, Limitation backlightCompensation);
    ~VideoCaptureFiltersCapability();

    bool VerifyBrightness(int32_t brightness);
    bool VerifyContrast(int32_t contrast);
    bool VerifyHue(int32_t hue);
    bool VerifySaturation(int32_t saturation);
    bool VerifySharpness(int32_t sharpness);
    bool VerifyGamma(int32_t gamma);
    bool VerifyBacklightCompensation(int32_t backlightCompensation);

    Limitation GetBrightnessConstraint();
    Limitation GetContrastConstraint();
    Limitation GetHueConstraint();
    Limitation GetSaturationConstraint();
    Limitation GetSharpnessConstraint();
    Limitation GetGammaConstraint();
    Limitation GetBacklightCompensationConstraint();

    void PrintConstraintValues();
};
#endif /* _VIDEOCAPTUREFILTERSCAPABILITY_H*/

