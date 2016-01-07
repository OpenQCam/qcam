// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    :
// // Version :
// // Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//

#ifndef _VIDEOACTIONHANDLER_H
#define _VIDEOACTIONHANDLER_H

#include "actionhandler/baseactionhandler.h"

class QICMJPGVideoCapture;
class BaseAVCVideoCapture;
//jennifer
//class BaseInStream;
class BaseVideoStream;


namespace app_qicstreamer {

  class Helper{
    public:
      static Variant PackageInObj(signed int max,signed int min,signed int dflt){
        Variant obj;
        obj["max"] = max;
        obj["min"] = min;
        obj["default"] = dflt;
        return obj;
      }
  };

  class VideoActionHandler
  : public BaseActionHandler {
    private:
      typedef RestHTTPCode (VideoActionHandler::*Action)(Variant &req, Variant &res);
      typedef map<string, Action> ActionMap;
      typedef map<bool, ActionMap*> CollectionMap;
      map<string, CollectionMap*> _actionMaps;
      QICMJPGVideoCapture *_pMJPGCaptureInstance;
      BaseAVCVideoCapture *_pAVCCaptureInstance;

      //jennifer
      //BaseInStream *_pMJPGInStream;
      //BaseInStream *_pAVCInStream;
      BaseVideoStream *_pMJPGInStream;
      BaseVideoStream *_pAVMuxInStream;
      BaseVideoStream *_pRelayMuxInStream;

    public:
      VideoActionHandler(string apiversion, BaseClientApplication *pApplication);
      virtual ~VideoActionHandler();
      virtual bool Initialize();
      virtual bool IsValidAction(string action);
      virtual RestHTTPCode DoAction(BaseProtocol *pFrom, bool isRO,
                                    vector<string> &resource,
                                    Variant &parameters, Variant &message);
    private:
      bool IsValidMJPGAction(string action);
      bool IsValidAVCAction(string action);

      // MJPG Get
      RestHTTPCode GetMJPGCaptureStatus(Variant &parameters, Variant &message);
      RestHTTPCode GetMJPGStatus(Variant &parameters, Variant &message);
      RestHTTPCode GetMJPGResolution(Variant &parameters, Variant &message);
      RestHTTPCode GetMJPGFrameRate(Variant &parameters, Variant &message);
      RestHTTPCode GetAllMJPGSetting(Variant &parameters, Variant &message);
      RestHTTPCode GetAllMJPGDefaultSetting(Variant &parameters, Variant &message);
      RestHTTPCode GetMJPGSettingConstraints(Variant &parameters, Variant &message);

      // MJPG Post
      RestHTTPCode StartAndStopMJPGVideoCapture(Variant &parameters, Variant &message);
      RestHTTPCode ResumeAndPauseMJPGVideoCapture(Variant &parameters, Variant &message);
      RestHTTPCode SetMJPGResolution(Variant &parameters, Variant &message);
      RestHTTPCode SetMJPGFrameRate(Variant &parameters, Variant &message);
      RestHTTPCode SetAllMJPGSetting(Variant &parameters, Variant &message);
      RestHTTPCode SetAllMJPGDefaultSetting(Variant &parameters, Variant &message);

      // AVC Get
      RestHTTPCode GetAVCCaptureStatus(Variant &parameters, Variant &message);
      RestHTTPCode GetAVCStatus(Variant &parameters, Variant &message);
      RestHTTPCode GetAVCResolution(Variant &parameters, Variant &message);
      RestHTTPCode GetAVCFrameRate(Variant &parameters, Variant &message);
      RestHTTPCode GetBitRate(Variant &parameters, Variant &message);
      RestHTTPCode GetGOP(Variant &parameters, Variant &message);
      RestHTTPCode GetAllAVCSetting(Variant &parameters, Variant &message);
      RestHTTPCode GetAllAVCDefaultSetting(Variant &parameters, Variant &message);
      RestHTTPCode GetAVCSettingConstraints(Variant &parameters, Variant &message);
      RestHTTPCode GetAVCCapabilities(Variant &parameters, Variant &message);
      RestHTTPCode GetAVCOSDInformation(Variant &parameters, Variant &message);
      // AVC Post
      RestHTTPCode StartAndStopAVCVideoCapture(Variant &parameters, Variant &message);
      RestHTTPCode ResumeAndPauseAVCVideoCapture(Variant &parameters, Variant &message);
      RestHTTPCode SetAVCResolution(Variant &parameters, Variant &message);
      RestHTTPCode SetAVCFrameRate(Variant &parameters, Variant &message);
      RestHTTPCode SetBitRate(Variant &parameters, Variant &message);
      RestHTTPCode SetGOP(Variant &parameters, Variant &message);
      RestHTTPCode SetAllAVCSetting(Variant &parameters, Variant &message);
      RestHTTPCode SetAllAVCDefaultSetting(Variant &parameters, Variant &message);
      RestHTTPCode SetOSD(Variant &parameters, Variant &message);
      RestHTTPCode ClearAllOSD(Variant &parameters, Variant &message);
      // remove
      RestHTTPCode GetKeyFrame(Variant &parameters, Variant &message);
      RestHTTPCode GetRegisteryValue(Variant &parameters, Variant &message);

      // Common Get
      RestHTTPCode GetBrightness(Variant &parameters, Variant &message);
      RestHTTPCode GetContrast(Variant &parameters, Variant &message);
      RestHTTPCode GetHue(Variant &parameters, Variant &message);
      RestHTTPCode GetSaturation(Variant &parameters, Variant &message);
      RestHTTPCode GetSharpness(Variant &parameters, Variant &message);
      RestHTTPCode GetGamma(Variant &parameters, Variant &message);
      RestHTTPCode GetBacklightCompensation(Variant &parameters, Variant &message);
      RestHTTPCode GetFlipMode(Variant &parameters, Variant &message);
      RestHTTPCode GetAllCommonSetting(Variant &parameters, Variant &message);
      RestHTTPCode GetCommonConstraints(Variant &parameters, Variant &message);
      // Common Post
      RestHTTPCode SetBrightness(Variant &parameters, Variant &message);
      RestHTTPCode SetContrast(Variant &parameters, Variant &message);
      RestHTTPCode SetHue(Variant &parameters, Variant &message);
      RestHTTPCode SetSaturation(Variant &parameters, Variant &message);
      RestHTTPCode SetSharpness(Variant &parameters, Variant &message);
      RestHTTPCode SetGamma(Variant &parameters, Variant &message);
      RestHTTPCode SetBacklightCompensation(Variant &parameters, Variant &message);
      RestHTTPCode SetFlipMode(Variant &parameters, Variant &message);
      RestHTTPCode SetAllCommonSetting(Variant &parameters, Variant &message);
      RestHTTPCode SetCommonToDefaultSetting(Variant &parameters, Variant &message);
      // Video firmware
      RestHTTPCode SetCommonVideoFWUpgrade(Variant &parameters, Variant &message);
      // y_value
      RestHTTPCode GetCommonYValue(Variant &parameters, Variant &message);
  };

}

#endif
