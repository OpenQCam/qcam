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

#ifdef HAS_PROTOCOL_CLI
#ifndef _DEVICEACTIONHANDLER_H
#define _DEVICEACTIONHANDLER_H

#include "actionhandler/baseactionhandler.h"
#include "system/systemmanager.h"

class SystemInfo;
class GPIOProtocol;
class BaseV4L2VideoCapture;
class BaseAVCVideoCapture;

namespace app_qicstreamer {

class DeviceActionHandler:
  public BaseActionHandler {
    private:
      typedef RestHTTPCode (DeviceActionHandler::*Action)(Variant &req, Variant &res);
      typedef map<string, Action> ActionMap;
      typedef map<bool, ActionMap*> CollectionMap;
      map<string, CollectionMap *> _actionMaps;
      vector<Variant> _profileSets;
      BaseV4L2VideoCapture *_pMJPGCaptureInstance;
      BaseAVCVideoCapture *_pAVCCaptureInstance;
      SystemInfo *_pSystemInfo;

    public:
      DeviceActionHandler(string apiversion, BaseClientApplication *pApplication);
      virtual ~DeviceActionHandler();
      virtual bool Initialize();
      virtual bool IsValidAction(string action);
      virtual bool IsValidAction(map<string, CollectionMap*> *map,
                                 string collection,
                                 string id,
                                 bool isReadyOnly);
      virtual RestHTTPCode DoAction(BaseProtocol *pFrom,
                                    bool isRO,
                                    vector<string> &resource,
                                    Variant &parameters,
                                    Variant &message);
      virtual void OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                 uint32_t dataLength);
    private:
      // Actions
      //Hardware API
      RestHTTPCode GetHWDeviceInfo(Variant &params, Variant &message);
      RestHTTPCode GetHWFirmwareInfo(Variant &params, Variant &message);
      RestHTTPCode GetHWCapabilities(Variant &params, Variant &message);
      RestHTTPCode GetHWStatus(Variant &params, Variant &message);
      //System API
      RestHTTPCode GetTimeStamp(Variant &params, Variant &message);
      RestHTTPCode GetSystemInfo(Variant &params, Variant &message);
      RestHTTPCode PostSystemInfo(Variant &params, Variant &message);
      RestHTTPCode GetSystemNVRam(Variant &params, Variant &message);
      RestHTTPCode PostSystemNVRam(Variant &params, Variant &message);
      RestHTTPCode GetSystemDateTime(Variant &params, Variant &message);
      RestHTTPCode PostSystemDateTime(Variant &params, Variant &message);
      RestHTTPCode GetSystemSettings(Variant &params, Variant &message);
      RestHTTPCode PostSystemSettings(Variant &params, Variant &message);
      RestHTTPCode PostSystemFWDownload(Variant &params, Variant &message);
      RestHTTPCode PostSystemFWUpgrade(Variant &params, Variant &message);
//      RestHTTPCode PostSystemUnbind(Variant &params, Variant &message);
      RestHTTPCode PostSystemFactoryReset(Variant &params, Variant &message);
      RestHTTPCode PostSystemCommand(Variant &params, Variant &message);
      RestHTTPCode PostSystemReboot(Variant &params, Variant &message);

      RestHTTPCode TestJsonp(Variant &params, Variant &message);

      //System
      //RestHTTPCode UpdateFirmware(Variant &params, Variant &message);
      //RestHTTPCode SigFWUpdate(Variant &params, Variant &message);
      //RestHTTPCode DownloadFirmware(Variant &params, Variant &message);
      //RestHTTPCode SigFWDownload(Variant &params, Variant &message);
      //RestHTTPCode EnableSystemInfo(Variant &params, Variant &message);
      //RestHTTPCode DisableSystemInfo(Variant &params, Variant &message);
      //RestHTTPCode UnbindCamera(Variant &params, Variant &message);
      //RestHTTPCode FactoryReset(Variant &params, Variant &message);
      //RestHTTPCode ExecuteCommand(Variant &params, Variant &message);
};
}

#endif
#endif
