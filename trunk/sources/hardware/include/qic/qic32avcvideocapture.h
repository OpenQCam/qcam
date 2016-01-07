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
 *  Purpose : Video Capture Class for QIC1832
 * ==================================================================
 */

#ifdef QIC1832

#ifndef _QIC32AVCVIDEOCAPTURE_H
#define _QIC32AVCVIDEOCAPTURE_H

#include "qic/videocapturecapability.h"
#include "streaming/baseoutstream.h"
#include "video/baseavcvideocapture.h"
#include "qic/qic_xuctrl.h"
#include "qic/qic_control.h"

#pragma pack(push, 1)
struct QIC32Header {
  bool IsKeyFrame : 1;
  uint8_t SPSLength : 8;
  uint8_t PPSLength : 8;
  uint32_t FrameOffset : 32;
  uint32_t FrameLength : 32;
  uint64_t PTS          : 64;
};
#pragma pack(pop)


class QIC32AVCVideoCapture
: public BaseAVCVideoCapture
{
  private:
    typedef struct _OSD{
      bool isNameEnabled;
      bool isTimeEnabled;
      string name;
      int8_t namePosition;
      int8_t dateFormatIndex;
      int8_t datePosition;
      _OSD():isNameEnabled(0), isTimeEnabled(0)
            ,name(""), namePosition(-1)
            ,dateFormatIndex(-1), datePosition(-1)
      {}
    }OSD;

    static BaseHardwareAbstractDevice *_pDeviceInstance;
    static const char * const _kDateFormat[];
    static const uint16_t _kOSDPositionXCoordinate[];
    static const uint16_t _kOSDPositionYCoordinate[];
    static const uint16_t _kOSDPositionToLine[];
    static StreamLayerId_t _deviceStreamIdMap[];
    bool _subStreamEnableStatus[16];


    OSD *_pOSDInfo;
    string _defaultWidth;
    string _defaultHeight;
    string _defaultFrameRate;
    string _defaultBitRate;
    string _defaultGOP;
    bool _isSetFormat;
    bool _isALSSupported;
    bool _isIRSupported;
    bool _isOSDSupported;
    bool _isPTSupported;
    bool _isSimulcastSupported;
    Limitation _pan;
    Limitation _tilt;

    uint32_t _lfcnt;
    bool _PTTagSpaceAvailability[MAX_TAG+1];
    map<string,PTPositionInfo> _PTTagList;
    string _IRLEDMode;

    VideoConstraintMap _deviceVideoCapabilities;
  private:


    bool InitSimulcast();


    uint32_t DecodePPSId(uint8_t *pData);

#if 0
    /*This whole block are used to parse AVC frame,
     */
    bool ParseFrame(uint8_t *pData, uint32_t dataLength, QIC32Header* &header);
    uint32_t FindStartCode(uint8_t *pData, uint32_t dataLength, uint8_t &type);
    bool ParseKeyFrame(uint8_t *pData, uint32_t dataLength, QIC32Header* &header, uint32_t startCodePosition);
    bool ParsePFrame(uint8_t *pData, uint32_t dataLength, QIC32Header* &header, uint32_t nextStartCodeOffset);
#endif

    void SelectDefaultSettingFields(string streamNumber);
  public:
    uint32_t _fcnt;
    StreamLayerId_t IdentifySubStream(uint8_t *pData, uint32_t pDataLength);
    QIC32Header *_pHeader;
    QIC32AVCVideoCapture(int32_t fd);
    virtual ~QIC32AVCVideoCapture();

    // Device singleton Interface
    static BaseHardwareAbstractDevice* GetInstance(string devNode);

    // BaseHardwareAbstractDevice
    virtual bool OnReadEvent();
    virtual bool InitializeDevice();
    virtual bool GetDeviceVideoConstraints();

    // BaseV4L2VideoCapture
    virtual uint8_t CreateVideoStreams(StreamsManager *pSM, string streamName);
    virtual bool CustomizeVideoSettings();

    virtual bool SelectDeviceStream(string streamName);

    virtual bool StartCapture();
    virtual bool StopCapture();

    virtual bool VerifyResolution(uint16_t width, uint16_t height);
    virtual bool VerifyBitrate(uint32_t bitrate);
    virtual bool VerifyFPS(uint32_t fps);
    virtual bool VerifyGOP(uint32_t gop);

    virtual bool SetResolution(uint32_t width, uint32_t height);
    virtual bool SetFrameRate(uint32_t frameRate);
    virtual uint32_t GetFrameRate();
    virtual bool SetPanTilt(int16_t pan, int16_t tilt);
    virtual void GetPanTilt(int16_t &pan, int16_t &tilt);

    // BaseAVCVideoCapture
    virtual bool SetBitRate(uint32_t bitrate);
    virtual uint32_t GetBitRate();
    virtual bool SetGOP(uint32_t gop);
    virtual uint32_t GetGOP();
    virtual bool GetKeyFrame();

    virtual uint32_t GetNumOfProfiles();
    virtual bool GetProfile(int idx, int *level, int *profile, int *constraint_flags);
    virtual bool SetProfileLevel(int level, int profile, int constraint_flags);
    virtual bool GetProfileLevel(int *level, int *profile, int *constraint_flags);

    virtual string GetFirmwareVersion();

    virtual bool SetDefaultResolution(string width, string height);
    virtual Variant GetDefaultResolution();
    virtual bool SetDefaultFrameRate(string fps);
    virtual string GetDefaultFrameRate();
    virtual bool SetDefaultBitRate(string bitrate);
    virtual string GetDefaultBitRate();
    virtual bool SetDefaultGOP(string gop);
    virtual string GetDefaultGOP();

    virtual Variant GetConstraints();
    //OSD
    virtual bool IsOSDSupported();
    bool SetOSDStatus(unsigned char status);

    void ResetOSDInfo();
    virtual Variant GetOSDInformation();
    void UpdateOSDInformation(bool isNameEnabled, bool isTimeEnabled
                              , string name, uint8_t namePosition
                              , uint8_t dateFormatIndex, uint8_t datePosition);

    uint8_t GetAvailablePosition(uint8_t position);
    virtual bool IsValidOSDPosition(int8_t position);
    bool IsPositionCompatible(uint8_t namePosition,uint8_t datePosition);

    virtual bool IsValidOSDDateFormat(int8_t dateFormatIndex);
    virtual bool IsValidOSDName(uint8_t nameLength);

    bool SetOSDString(string content, unsigned char startIndex
                      , int position, OsdCharAttr_t *charAttribute);
    bool SetOSDTime(unsigned char lineNumber, unsigned char enable, int position);
    bool DeterminePosition(bool isNameEnabled, bool isTimeEnabled
                           , int8_t &namePosition, int8_t &datePosition
                           , uint8_t nameLength);
    bool SetOSDContent(uint8_t position, unsigned char startIndex, string content);

    virtual bool ClearAllOSD();
    virtual bool SetOSD(bool isNameEnabled, bool isTimeEnabled
                , string name, int8_t namePosition
                , int8_t dateFormat, int8_t datePosition);

    virtual void CheckStatus(double ts);
    // PT
    bool InitializePTTagList();
    bool DeleteTagInfo(string tagName, uint8_t nvramIndex);
    bool SetNVRamForPTTag(uint8_t index, string tagName, string pan, string tilt);
    bool ResetMotorPosition();
    virtual bool IsPTSupported();
    virtual bool CheckPanTiltBoundary(int16_t pan, int16_t tilt, int8_t& isLeftBoundary, int8_t& isRightBoundary, int8_t& isTopBoundary, int8_t& isBottomBoundary);
    virtual bool GetAvailableTagSpace(uint8_t& index);
    virtual bool IsPTTagExist(string tagName);
    virtual bool GetCurrentPTPosition(Variant& PTPosition);
    virtual bool RotatePT(int16_t horizontalStep, int16_t verticalStep);
    virtual bool RotatePTToPosition(int16_t pan, int16_t tilt);
    virtual Variant GetPTTagList();
    virtual bool RotatePTToTag(string tagName);
    virtual bool TagCurrentPTPosition(string tagName);
    virtual bool DeleteTag(string tagName);
    virtual bool EditTag(string oldTagName, string newTagName);
    virtual bool RenameTag(string oldTagName, string newTagName);
    virtual bool ClearPTTagList();
    virtual bool NavigatePT();
    virtual bool NavigatePTTo(queue<string>& tagList);
    virtual bool StopPTRotate();

    //IR
    virtual bool IsIRSupported();
    virtual bool SetIRLEDStatus(unsigned char mode);
    virtual bool GetIRLEDStatus(bool& status, string& IRLEDMode);
    //ALS
    virtual bool IsALSSupported();
    virtual bool GetALS(unsigned short& als);
    //AE
    virtual bool GetAEInfo(unsigned short& autoExposure);
    //Simulcast
    virtual bool IsSimulcastSupported();
    virtual bool ReadMMIO(unsigned int address, unsigned int *value);
    virtual bool GetSubStreamEnableStatus(bool *status);

    // Helper
    void PrintDeviceVideoConstraints();
    virtual void CheckIfSEIEnabled();
};
#endif /* _QIC32AVCVIDEOCAPTURE_H */

#endif

