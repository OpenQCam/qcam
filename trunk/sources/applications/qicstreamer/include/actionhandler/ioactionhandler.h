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
#ifndef _IOACTIONHANDLER_H
#define _IOACTIONHANDLER_H

#include "actionhandler/baseactionhandler.h"
#include "system/systemmanager.h"
#include "netio/select/iotimer.h"

class BaseAVCVideoCapture;

namespace app_qicstreamer {

  class IOActionHandler :
  public BaseActionHandler{
    private:
      typedef RestHTTPCode (IOActionHandler::*Action)(Variant &req, Variant &res);
      typedef map<string, Action> ActionMap;
      typedef map<bool, ActionMap*> CollectionMap;
      map<string, CollectionMap*> _actionMaps;
      BaseAVCVideoCapture *_pAVCCaptureInstance;

    public:
      IOActionHandler(string apiversion, BaseClientApplication *pApplication);
      virtual ~IOActionHandler();
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
      /*PT*/
      RestHTTPCode RotatePT(Variant &parameters, Variant &message);
      RestHTTPCode StopPT(Variant &parameters, Variant &message);

      RestHTTPCode GetPTSupportStatus(Variant &parameters, Variant &message);
      RestHTTPCode GetPTPosition(Variant &parameters, Variant &message);
      RestHTTPCode PostPTRotate(Variant &parameters, Variant &message);
      RestHTTPCode PostPTTag(Variant &parameters, Variant &message);
      RestHTTPCode GetPTTagList(Variant &parameters, Variant &message);
      RestHTTPCode PostPTTagEdit(Variant &parameters, Variant &message);
      RestHTTPCode PostPTTagRename(Variant &parameters, Variant &message);
      RestHTTPCode PostPTTagDelete(Variant &parameters, Variant &message);
      RestHTTPCode PostPTTagListClear(Variant &parameters, Variant &message);
      RestHTTPCode PostPTNavigate(Variant &parameters, Variant &message);
      RestHTTPCode PostPTNavigateTo(Variant &parameters, Variant &message);
      RestHTTPCode PostPTReset(Variant &parameters, Variant &message);
      RestHTTPCode PostPTStop(Variant &parameters, Variant &message);
      /*LED*/
      RestHTTPCode PostLEDStatus(Variant &parameters, Variant &message);
      RestHTTPCode GetLEDStatus(Variant &parameters, Variant &message);
      RestHTTPCode PostLEDEvent(Variant &parameters, Variant &message);
      /*PIR*/
      RestHTTPCode PostPIRStatus(Variant &parameters, Variant &message);
      RestHTTPCode GetPIRStatus(Variant &parameters, Variant &message);
      /*IR LED*/
      RestHTTPCode PostIRLEDStatus(Variant &parameters, Variant &message);
      RestHTTPCode GetIRLEDStatus(Variant &parameters, Variant &message);
      /*ALS*/
      RestHTTPCode GetALS(Variant &parameters, Variant &message);
  };

}

#endif
#endif
