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
#ifndef _PRODUCTIONACTIONHANDLER_H
#define _PRODUCTIONACTIONHANDLER_H

#include "actionhandler/baseactionhandler.h"
#include "system/systemmanager.h"
#include "netio/select/iotimer.h"


namespace app_qicstreamer {

  class ProductionActionHandler :
  public BaseActionHandler{
    private:
      typedef RestHTTPCode (ProductionActionHandler::*Action)(Variant &req, Variant &res);
      typedef map<string, Action> ActionMap;
      typedef map<bool, ActionMap*> CollectionMap;
      map<string, CollectionMap*> _actionMaps;

    public:
      ProductionActionHandler(string apiversion, BaseClientApplication *pApplication);
      virtual ~ProductionActionHandler();
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
      //virtual void OnCmdFinished(uint32_t msgId, uint8_t *pData, uint32_t dataLength);
    private:
      // Actions
      RestHTTPCode TestCommand(Variant &parameters, Variant &message);
#if 0
      RestHTTPCode TestLED(Variant &parameters, Variant &message);
      RestHTTPCode TestButton(Variant &parameters, Variant &message);
      RestHTTPCode TestSwitch(Variant &parameters, Variant &message);
      RestHTTPCode TestSD(Variant &parameters, Variant &message);
      RestHTTPCode TestFlash(Variant &parameters, Variant &message);
#endif
  };

}

#endif
#endif
