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
// // Author  : Recardo Cheng (recardo.cheng@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//


#ifndef _EVENTACTIONHANDLER_H
#define _EVENTACTIONHANDLER_H
#include "actionhandler/baseactionhandler.h"

class PushHandler;

enum RestErrorCodeEvent {
  EC_EVENT_NO_EVENT_MUXSTREAM,
  EC_EVENT_NO_VIDEO_STREAM,
  EC_EVENT_NO_AUDIO_STREAM,
  EC_EVENT_EVENT_MUX_ERR,
  EC_EVENT_NOT_ENOUGH_SPACE,
  EC_EVENT_NO_STROAGE_DEVICE,
  EC_EVENT_DIRECTORY_NOT_EXISTS,
  EC_EVENT_NO_PUSHVIDEO_HANDLER,
  EC_EVENT_START_FAIL,
};


namespace app_qicstreamer {
  class EventActionHandler
  : public BaseActionHandler {
    private:
      typedef RestHTTPCode (EventActionHandler::*Action)(Variant &req, Variant &res);
      typedef map<string, Action> ActionMap;
      typedef map<bool, ActionMap*> CollectionMap;
      map<string, CollectionMap*> _actionMaps;
      PushHandler *_pPushHandler;
      uint32_t _pushVideoLength;

    public:
      EventActionHandler(string apiversion, BaseClientApplication *pApplication);
      virtual ~EventActionHandler();
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
    private:
        RestHTTPCode GetAudioParams(Variant &params, Variant &message);
        RestHTTPCode GetAudioRMS(Variant &params, Variant &message);
        RestHTTPCode PostAudioParams(Variant &params, Variant &message);
        RestHTTPCode PostAudioRMS(Variant &params, Variant &message);
        RestHTTPCode PostAudioTrigger(Variant &params, Variant &message);
        RestHTTPCode GetPIRParams(Variant &params, Variant &message);
        RestHTTPCode PostPIRParams(Variant &params, Variant &message);
        RestHTTPCode PostPIRTrigger(Variant &params, Variant &message);
        RestHTTPCode GetFileDuration(Variant &params, Variant &message);
        RestHTTPCode PostFileDuration(Variant &params, Variant &message);
        RestHTTPCode GetFileRetention(Variant &params, Variant &message);
        RestHTTPCode PostFileRetention(Variant &params, Variant &message);
  };//class
}; //namespace
#endif
