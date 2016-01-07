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
#ifndef _SERVICEACTIONHANDLER_H
#define _SERVICEACTIONHANDLER_H

#include "actionhandler/baseactionhandler.h"

typedef struct _ws_relay_t {
  bool isResp;
  uint16_t errorCode;
  uint16_t eventType;
  string uri;
  string name;
} ws_relay_t;

class BaseStream;
class BaseInStream;
namespace app_qicstreamer {

  class ServiceActionHandler
  : public BaseActionHandler{
    private:
      typedef RestHTTPCode (ServiceActionHandler::*Action)(Variant &req, Variant &res);
      typedef map<string, Action> ActionMap;
      typedef map<bool, ActionMap*> CollectionMap;
      map<string, CollectionMap*> _actionMaps;
      //map<string, ws_relay_t> _relayMap;
      //map<uint32_t, string> _cmdMap;

      bool PushToServer(ws_relay_t& params);
      bool IsHandShaking(string name);
    public:
      ServiceActionHandler(string apiversion, BaseClientApplication *pApplication);
      virtual ~ServiceActionHandler();
      virtual bool Initialize();
      virtual bool IsValidAction(map<string, CollectionMap*> *map,
                                 string collection,
                                 string id,
                                 bool isReadyOnly);
      virtual RestHTTPCode DoAction(BaseProtocol *pFrom,
                                    bool isRO,
                                    vector<string> &resource,
                                    Variant &param,
                                    Variant &message);
      virtual void OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                 uint32_t dataLength);

    private:
      //bool InitiateForwardingStream(BaseInStream *pStream, Variant &target);
      //bool IsAVCStreamLive();
      //bool GetLiveStreamInfo(uint32_t uniqueId, Variant &info);

      //Cloud Get
      RestHTTPCode GetCloudConnection(Variant &parameters, Variant &message);
      RestHTTPCode GetCloudSetting(Variant &parameters, Variant &message);
      //RestHTTPCode SignalServerUpdated(Variant &parameter, Variant &message);
      RestHTTPCode WebsocketInfo(Variant &parameter, Variant &message);
      //Cloud Post
      RestHTTPCode PostCloudConnection(Variant &parameters, Variant &message);
      RestHTTPCode PostCloudSetting(Variant &parameters, Variant &message);
      RestHTTPCode PostCloudToken(Variant &parameters, Variant &message);
      RestHTTPCode PostCloudDelToken(Variant &parameters, Variant &message);
      RestHTTPCode PostCloudUnbind(Variant &parameters, Variant &message);
      RestHTTPCode PostCloudUpdateFS(Variant &parameters, Variant &message);
      RestHTTPCode PostCloudUpdateCA(Variant &parameters, Variant &message);
      //Stream Get
      RestHTTPCode GetLivePCMStream(Variant &parameters, Variant &message);
      RestHTTPCode GetLiveMJPGStream(Variant &parameters, Variant &message);
      RestHTTPCode GetLiveAVCStream(Variant &parameters, Variant &message);
      RestHTTPCode FindLinkedOutStreamsById(Variant &parameters, Variant &message);
      RestHTTPCode GetStreamById(Variant &parameters, Variant &message);
      RestHTTPCode ListAllStreams(Variant &parameters, Variant &message);
      //Stream Post
      RestHTTPCode PushAVCStream(Variant &parameters, Variant &message);
      //RestHTTPCode DeleteLiveAVCStream(Variant &parameters, Variant &message);
      RestHTTPCode PushMJPGStream(Variant &parameters, Variant &message);
      RestHTTPCode PushPCMStream(Variant &parameters, Variant &message);
  };

}

#endif
#endif
