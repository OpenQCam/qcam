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
 *  Purpose : Base AVC Video Capture Class
 * ==================================================================
 */

#ifndef _BASEAVCVIDEOCAPTURE_H
#define _BASEAVCVIDEOCAPTURE_H

#include "video/basev4l2videocapture.h"

#define SUPPORT_ALS 0x01
#define SUPPORT_IR 0x02
#define SUPPORT_OSD 0x04
#define SUPPORT_PT 0x08
#define SUPPORT_SIMULCAST 0x10

#define MAX_TAG 5

typedef struct {
  uint8_t nvramIndex;
  MotorPosition_t position;
}PTPositionInfo;

class BaseAVCVideoCapture
: public BaseV4L2VideoCapture
{
  protected:
    BaseAVCVideoCapture(HardwareType hwType, int32_t deviceFD);
    vector<uint32_t> _fpsToFrameInterval;
    vector<uint16_t> _fpsToFrameIntervalForGOP;

  public:
    virtual ~BaseAVCVideoCapture();
    virtual bool GetDeviceVideoConstraints() = 0;

    virtual bool VerifyResolution(uint16_t width, uint16_t height) = 0;
    virtual bool VerifyBitrate(uint32_t bitrate) = 0;
    virtual bool VerifyFPS(uint32_t fps) = 0;
    virtual bool VerifyGOP(uint32_t gop) = 0;

    virtual bool SetBitRate(uint32_t bitrate) = 0;
    virtual uint32_t GetBitRate() = 0;

    virtual bool SetGOP(uint32_t gop) = 0;
    virtual uint32_t GetGOP() = 0;

    virtual bool SetFrameRate(uint32_t frameRate) = 0;

    virtual bool GetKeyFrame() = 0;

    virtual uint32_t GetNumOfProfiles() = 0;
    virtual bool GetProfile(int idx, int *level, int *profile, int *constraint_flags) = 0;
    virtual bool SetProfileLevel(int level, int profile, int constraint_flags) = 0;
    virtual bool GetProfileLevel(int *level, int *profile, int *constraint_flags) = 0;

    virtual bool SetDefaultResolution(string width, string height) = 0;
    virtual Variant GetDefaultResolution() = 0;
    virtual bool SetDefaultFrameRate(string fps) = 0;
    virtual string GetDefaultFrameRate() = 0;
    virtual bool SetDefaultBitRate(string bitrate) = 0;
    virtual string GetDefaultBitRate() = 0;
    virtual bool SetDefaultGOP(string gop) = 0;
    virtual string GetDefaultGOP() = 0;

    virtual Variant GetConstraints() = 0;

    // OSD
    virtual bool IsOSDSupported()  = 0;
    virtual bool ClearAllOSD() = 0;
    virtual bool IsValidOSDPosition(int8_t position) = 0;
    virtual bool IsValidOSDDateFormat(int8_t dateFormatIndex) = 0;
    virtual bool IsValidOSDName(uint8_t nameLength) = 0;
    virtual Variant GetOSDInformation() = 0;
    virtual bool SetOSD(bool isNameEnabled, bool isTimeEnabled
                , string name, int8_t namePosition
                , int8_t dateFormat, int8_t datePosition) = 0;

    // PT
    virtual bool IsPTSupported() = 0;
    virtual bool CheckPanTiltBoundary(int16_t pan, int16_t tilt, int8_t& isLeftBoundary, int8_t& isRightBoundary, int8_t& isTopBoundary, int8_t& isBottomBoundary) = 0;
    virtual bool GetAvailableTagSpace(uint8_t& index) = 0;
    virtual bool IsPTTagExist(string tagName) = 0;
    virtual bool GetCurrentPTPosition(Variant& PTPosition) = 0;
    virtual bool RotatePT(int16_t horizontalStep, int16_t verticalStep) = 0;
    virtual bool RotatePTToPosition(int16_t pan, int16_t tilt) = 0;
    virtual Variant GetPTTagList() = 0;
    virtual bool RotatePTToTag(string tagName) = 0;
    virtual bool TagCurrentPTPosition(string tagName) = 0;
    virtual bool DeleteTag(string tagName) = 0;
    virtual bool EditTag(string oldTagName, string newTagName) = 0;
    virtual bool RenameTag(string oldTagName, string newTagName) = 0;
    virtual bool ClearPTTagList() = 0;
    virtual bool NavigatePT() = 0;
    virtual bool NavigatePTTo(queue<string>& tagList) = 0;
    virtual bool StopPTRotate() = 0;

    //IR
    virtual bool IsIRSupported() = 0;
    virtual bool SetIRLEDStatus(unsigned char mode) = 0;
    virtual bool GetIRLEDStatus(bool& status, string& IRLEDMode) = 0;
    //ALS
    virtual bool IsALSSupported() = 0;
    virtual bool GetALS(unsigned short& als) = 0;
    //AE
    virtual bool GetAEInfo(unsigned short& autoExposure) = 0;
    //Simulcast
    virtual bool IsSimulcastSupported() = 0;

    virtual bool ReadMMIO(unsigned int address, unsigned int *value) = 0;
};
#endif /* _BASEAVCVIDEOCAPTURE_H */

