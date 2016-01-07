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
#ifndef _NETWORKACTIONHANDLER_H
#define _NETWORKACTIONHANDLER_H

#include "actionhandler/baseactionhandler.h"
#include "system/systemmanager.h"

class SystemInfo;
class GPIOProtocol;
class BaseV4L2VideoCapture;

namespace app_qicstreamer {

class NetworkActionHandler:
  public BaseActionHandler {
    private:
      typedef RestHTTPCode (NetworkActionHandler::*Action)(Variant &req, Variant &res);
      typedef map<string, Action> ActionMap;
      typedef map<bool, ActionMap*> CollectionMap;
      map<string, CollectionMap*> _actionMaps;

    public:
      NetworkActionHandler(string apiversion, BaseClientApplication *pApplication);
      virtual ~NetworkActionHandler();
      virtual bool Initialize();
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
      //Management API
      RestHTTPCode GetNetworkIFs(Variant &params, Variant &message);
      RestHTTPCode GetExternalIP(Variant &params, Variant &message);
      RestHTTPCode GetIPSetting(Variant &params, Variant &message);
      RestHTTPCode SetIPSetting(Variant &params, Variant &message);
      RestHTTPCode Ping(Variant &params, Variant &message);
      RestHTTPCode GetStatistics(Variant &params, Variant &message);
      //WiFi API
      RestHTTPCode GetWiFiIFs(Variant &params, Variant &message);
      RestHTTPCode GetServiceInfo(Variant &params, Variant &message);
      RestHTTPCode GetSiteSurvey(Variant &params, Variant &message);
      RestHTTPCode SetWiFiSecurity(Variant &params, Variant &message);
      RestHTTPCode GetWiFiSecurity(Variant &params, Variant &message);
      RestHTTPCode DoWPS(Variant &params, Variant &message);

      RestHTTPCode ExecuteCommand(Variant &params, Variant &message);
};

}

#endif
#endif
