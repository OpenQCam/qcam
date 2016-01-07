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
 *  Purpose : Video Capture Filters Capability Class for all video capture device
 * ==================================================================
 */


#include "qic/videocapturefilterscapability.h"

VideoCaptureFiltersCapability::VideoCaptureFiltersCapability(Limitation brightness, Limitation contrast, Limitation hue, Limitation saturation, Limitation sharpness, Limitation gamma, Limitation backlightCompensation)
{
  _brightness = brightness;
  _contrast=contrast;
  _hue=hue;
  _saturation=saturation;
  _sharpness=sharpness;
  _gamma=gamma;
  _backlightCompensation=backlightCompensation;
}

VideoCaptureFiltersCapability::~VideoCaptureFiltersCapability() {
}
bool VideoCaptureFiltersCapability::VerifyBrightness(int32_t brightness){
  if(brightness < _brightness.min || brightness > _brightness.max){
    FATAL("Invalid brightness %d. Brightness should between %d and %d", brightness, _brightness.min, _brightness.max);
    return false;
  }
  return true;
}
bool VideoCaptureFiltersCapability::VerifyContrast(int32_t contrast){
  if(contrast < _contrast.min || contrast > _contrast.max){
    FATAL("Invalid contrast %d. Contrast should between %d and %d", contrast, _contrast.min, _contrast.max);
    return false;
  }
  return true;
}
bool VideoCaptureFiltersCapability::VerifyHue(int32_t hue){
  if(hue < _hue.min || hue > _hue.max){
    FATAL("Invalid hue %d. Hue should between %d and %d", hue, _hue.min, _hue.max);
    return false;
  }
  return true;
}
bool VideoCaptureFiltersCapability::VerifySaturation(int32_t saturation){
  if(saturation < _saturation.min || saturation > _saturation.max){
    FATAL("Invalid saturation %d. Saturation should between %d and %d", saturation, _saturation.min, _saturation.max);
    return false;
  }
  return true;
}
bool VideoCaptureFiltersCapability::VerifySharpness(int32_t sharpness){
  if(sharpness < _sharpness.min || sharpness > _sharpness.max){
    FATAL("Invalid sharpness %d. Sharpness should between %d and %d", sharpness, _sharpness.min, _sharpness.max);
    return false;
  }
  return true;
}
bool VideoCaptureFiltersCapability::VerifyGamma(int32_t gamma){
  if(gamma < _gamma.min || gamma > _gamma.max){
    FATAL("Invalid gamma %d. Gamma should between %d and %d", gamma, _gamma.min, _gamma.max);
    return false;
  }
  return true;
}
bool VideoCaptureFiltersCapability::VerifyBacklightCompensation(int32_t backlightCompensation){
  if(backlightCompensation < _backlightCompensation.min || backlightCompensation > _backlightCompensation.max){
    FATAL("Invalid backlight compensation %d. Backlight compensation should between %d and %d", backlightCompensation, _backlightCompensation.min, _backlightCompensation.max);
    return false;
  }
  return true;
}
Limitation VideoCaptureFiltersCapability::GetBrightnessConstraint(){
  return _brightness;
}
Limitation VideoCaptureFiltersCapability::GetContrastConstraint(){
  return _contrast;
}
Limitation VideoCaptureFiltersCapability::GetHueConstraint(){
  return _hue;
}
Limitation VideoCaptureFiltersCapability::GetSaturationConstraint(){
  return _saturation;
}
Limitation VideoCaptureFiltersCapability::GetSharpnessConstraint(){
  return _sharpness;
}
Limitation VideoCaptureFiltersCapability::GetGammaConstraint(){
  return _gamma;
}
Limitation VideoCaptureFiltersCapability::GetBacklightCompensationConstraint(){
  return _backlightCompensation;
}
void VideoCaptureFiltersCapability::PrintConstraintValues(){
  INFO("Brightness, max : %d, min :%d, default : %d ", _brightness.max, _brightness.min, _brightness.dflt);
  INFO("Contrast, max : %d, min :%d, default : %d ", _contrast.max, _contrast.min, _contrast.dflt);
  INFO("Hue, max : %d, min :%d, default : %d ", _hue.max, _hue.min, _hue.dflt);
  INFO("Saturation, max : %d, min :%d, default : %d ", _saturation.max, _saturation.min, _saturation.dflt);
  INFO("Sharpness, max : %d, min :%d, default : %d ", _sharpness.max, _sharpness.min, _sharpness.dflt);
  INFO("Gamma, max : %d, min :%d, default : %d ", _gamma.max, _gamma.min, _gamma.dflt);
  INFO("Backlight compensation, max : %d, min :%d, default : %d ", _backlightCompensation.max, _backlightCompensation.min, _backlightCompensation.dflt);
}
