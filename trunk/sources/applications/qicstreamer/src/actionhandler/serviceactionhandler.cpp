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
//

#ifdef HAS_PROTOCOL_CLI

#include "actionhandler/serviceactionhandler.h"
//thelib
#include "clidefine.h"
#include "application/baseclientapplication.h"
#include "streaming/streamstypes.h"
#include "streaming/basestream.h"
#include "streaming/baseinstream.h"
#include "streaming/outjpgstream.h"
#include "protocols/avhttpstream/avstreamdefine.h"
#include "system/eventdefine.h"
#include "system/nvramdefine.h"
#include "protocols/http/outboundhttpwsprotocol.h"
#include "protocols/http/httpauthhelper.h"
//hardware
#include "system/systemmanager.h"
//application
#include "appprotocolhandler/wsclientappprotocolhandler.h"
#include "qicstreamerapplication.h"

using namespace app_qicstreamer;


ServiceActionHandler::ServiceActionHandler(string version, BaseClientApplication *pApplication)
: BaseActionHandler(AT_IO, version, pApplication)
{
}

ServiceActionHandler::~ServiceActionHandler() {
  (*_actionMaps[QIC_CLI_SERVICE_STREAM])[true]->clear();
  (*_actionMaps[QIC_CLI_SERVICE_STREAM])[false]->clear();
  (*_actionMaps[QIC_CLI_SERVICE_CLOUD])[true]->clear();
  (*_actionMaps[QIC_CLI_SERVICE_CLOUD])[false]->clear();
  _actionMaps[QIC_CLI_SERVICE_STREAM]->clear();
  _actionMaps[QIC_CLI_SERVICE_CLOUD]->clear();
  _actionMaps.clear();
}

bool ServiceActionHandler::Initialize() {
  ActionMap *pStreamGetActionMap = new ActionMap;
  ActionMap *pStreamPostActionMap = new ActionMap;
  ActionMap *pCloudGetActionMap = new ActionMap;
  ActionMap *pCloudPostActionMap = new ActionMap;
  CollectionMap *pStreamMap= new CollectionMap;
  CollectionMap *pCloudMap= new CollectionMap;

  //construct map
  (*pStreamMap)[true] = pStreamGetActionMap;
  (*pStreamMap)[false] = pStreamPostActionMap;
  (*pCloudMap)[true] = pCloudGetActionMap;
  (*pCloudMap)[false] = pCloudPostActionMap;
  _actionMaps[QIC_CLI_SERVICE_STREAM] = pStreamMap;
  _actionMaps[QIC_CLI_SERVICE_CLOUD] = pCloudMap;

  //Stream Get API
  (*pStreamGetActionMap)[QIC_CLI_SERVICE_STREAM_AVC] = reinterpret_cast<Action>(&ServiceActionHandler::GetLiveAVCStream);
  (*pStreamGetActionMap)[QIC_CLI_SERVICE_STREAM_MJPG] = reinterpret_cast<Action>(&ServiceActionHandler::GetLiveMJPGStream);
  (*pStreamGetActionMap)[QIC_CLI_SERVICE_STREAM_PCM] = reinterpret_cast<Action>(&ServiceActionHandler::GetLivePCMStream);
  (*pStreamGetActionMap)[QIC_CLI_SERVICE_STREAM_ALL] = reinterpret_cast<Action>(&ServiceActionHandler::ListAllStreams);
  //Stream Post API
  (*pStreamPostActionMap)[QIC_CLI_SERVICE_STREAM_PUSH_AVC] = reinterpret_cast<Action>(&ServiceActionHandler::PushAVCStream);
  (*pStreamPostActionMap)[QIC_CLI_SERVICE_STREAM_PUSH_MJPG] = reinterpret_cast<Action>(&ServiceActionHandler::PushMJPGStream);
  (*pStreamPostActionMap)[QIC_CLI_SERVICE_STREAM_PUSH_PCM] = reinterpret_cast<Action>(&ServiceActionHandler::PushPCMStream);

  //Cloud GET API
  (*pCloudGetActionMap)[QIC_CLI_SERVICE_CLOUD_CONNECTION] = reinterpret_cast<Action>(&ServiceActionHandler::GetCloudConnection);
  (*pCloudGetActionMap)[QIC_CLI_SERVICE_CLOUD_SETTING] = reinterpret_cast<Action>(&ServiceActionHandler::GetCloudSetting);
  (*pCloudGetActionMap)[QIC_CLI_SERVICE_CLOUD_WEBSOCKETINFO] = reinterpret_cast<Action>(&ServiceActionHandler::WebsocketInfo);
  //Cloud POST API
  (*pCloudPostActionMap)[QIC_CLI_SERVICE_CLOUD_CONNECTION] = reinterpret_cast<Action>(&ServiceActionHandler::PostCloudConnection);
  (*pCloudPostActionMap)[QIC_CLI_SERVICE_CLOUD_SETTING] = reinterpret_cast<Action>(&ServiceActionHandler::PostCloudSetting);
  (*pCloudPostActionMap)[QIC_CLI_SERVICE_CLOUD_UPDATE_FS] = reinterpret_cast<Action>(&ServiceActionHandler::PostCloudUpdateFS);
  (*pCloudPostActionMap)[QIC_CLI_SERVICE_CLOUD_UPDATE_CA] = reinterpret_cast<Action>(&ServiceActionHandler::PostCloudUpdateCA);
  (*pCloudPostActionMap)[QIC_CLI_SERVICE_CLOUD_AUTH] = reinterpret_cast<Action>(&ServiceActionHandler::PostCloudToken);
  (*pCloudPostActionMap)[QIC_CLI_SERVICE_CLOUD_DEAUTH] = reinterpret_cast<Action>(&ServiceActionHandler::PostCloudDelToken);
  (*pCloudPostActionMap)[QIC_CLI_SERVICE_CLOUD_UNBIND] = reinterpret_cast<Action>(&ServiceActionHandler::PostCloudUnbind);

  return true;
}

void ServiceActionHandler::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                         uint32_t dataLength) {
//#define AIRPORT_TICKET "ticket"
//
//  string result((char*)pData, dataLength);
//  Variant data;
//  uint32_t start=0;
//  string ticket;
//  Variant::DeserializeFromJSON(result, data, start);
//
//  string cmd=_cmdMap[msgId];
//
//  if (MAP_HAS1(_relayMap, cmd)) {
//    _relayMap[cmd].isResp=true;
//    uint16_t errorCode=_relayMap[cmd].errorCode;
//    if (data==V_MAP && data.HasKey(AIRPORT_TICKET)) {
//      ticket = (string)data[AIRPORT_TICKET];
//      WSClientAppProtocolHandler* pHandler=reinterpret_cast<WSClientAppProtocolHandler*>
//        (_pApplication->GetProtocolHandler(PT_OUTBOUND_HTTP_WSCONNECTION));
//
//      ws_param_t wsParam={true, false, false,
//        _relayMap[cmd].errorCode, _relayMap[cmd].eventType, 20000,
//        _relayMap[cmd].uri, _relayMap[cmd].name};
//
//      if (pHandler->IsServiceTimerExists(wsParam.name)) {
//        DEBUG ("relay is handshaking..");
//        return;
//      }
//      if (!pHandler->InitiateConnection(wsParam, ticket)) {
//        WARN ("Unable to establish connection with relay server");
//        string errorDesc=ERROR_DESCRIPTION(errorCode)+" Unable to connection relay server. Address:" + wsParam.uri;
//        //1. Send error for cloud
//        reinterpret_cast<QICStreamerApplication*>(_pApplication)->SendError(ERROR_CODE(errorCode),
//                                                                            errorDesc);
//        //1. Send event for client
//        double ts;
//        GETTIMESTAMP(ts);
//        reinterpret_cast<QICStreamerApplication*>(_pApplication)->SendEvent(_relayMap[cmd].eventType,
//                                                                            (uint64_t)ts, "");
//      }
//    }
//    else {
//      WARN ("invalid ticket parameter");
//      string errorDesc=ERROR_DESCRIPTION(errorCode)+" Invalid ticket parameter.";
//      reinterpret_cast<QICStreamerApplication*>(_pApplication)->SendError(ERROR_CODE(errorCode),
//                                                                          errorDesc);
//      double ts;
//      GETTIMESTAMP(ts);
//      reinterpret_cast<QICStreamerApplication*>(_pApplication)->SendEvent(_relayMap[cmd].eventType,
//                                                                          (uint64_t)ts, "");
//    }
//  }
}

bool ServiceActionHandler::IsValidAction(map<string, CollectionMap*> *map,
                                         string collection,
                                         string id,
                                         bool isReadyOnly) {
  return true;
}


RestHTTPCode ServiceActionHandler::DoAction(BaseProtocol *pFrom,
                                            bool isRO,
                                            vector<string> &resource,
                                            Variant &payload,
                                            Variant &message) {
  string collection = resource[3];
  string id = resource[4];

  if(!MAP_HAS1(_actionMaps, collection)) {
    return SendFailure(HC_400_BAD_REQUEST, EC_106_COLLECTION_NOT_FOUND,
        "Unable to find collection: " + collection, message);
  }
  if(!MAP_HAS1(*_actionMaps[collection], isRO)){
    return SendFailure(HC_400_BAD_REQUEST, EC_103_METHOD_NOT_ALLOWED,
        "Invalid method: " + isRO, message);
  }
  if(!MAP_HAS1(*(*_actionMaps[collection])[isRO], id)){
    return SendFailure(HC_400_BAD_REQUEST, EC_107_RESOURCE_ID_NOT_FOUND,
        "Unable to find resource id: " + id, message);
  }

  Action pAction = (*(*_actionMaps[collection])[isRO])[id];
  return ((this->*pAction)(payload[REST_PARAM_STR], message));
}

// Private Functions
RestHTTPCode ServiceActionHandler::ListAllStreams(Variant &parameters, Variant &message) {
  map<uint32_t, BaseStream*> allStreams = _pApplication->GetStreamsManager()->GetAllStreams();
  FOR_MAP(allStreams, uint32_t, BaseStream*, i){
    Variant streamInfo;
    BaseStream *pStream = MAP_VAL(i);
    streamInfo["id"] = pStream->GetUniqueId();
    streamInfo["type"] = STR(tagToString(pStream->GetType()));
    streamInfo["name"] = pStream->GetName();
    message.PushToArray(streamInfo);
  }
  return HC_200_OK;
}

RestHTTPCode ServiceActionHandler::WebsocketInfo(Variant &parameter, Variant &message) {
  DEBUG ("websocket info");
  WSClientAppProtocolHandler *pProtocolHandler=
      reinterpret_cast<WSClientAppProtocolHandler*>(_pApplication->GetProtocolHandler(PT_OUTBOUND_HTTP_WSCONNECTION));
  if (!pProtocolHandler) {
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_704_WEBSOCKET_UNSUPPORTED,
      "Not supported websocket outbound protocol", message);
  }
  string state=pProtocolHandler->ListAllServiceTimerState();
  message["result"] = state;

  return HC_200_OK;
}

RestHTTPCode ServiceActionHandler::PostCloudUpdateFS(Variant &param, Variant &message) {
#define SERVER_ADDR "serverAddr"

  if (!param.HasKey(SERVER_ADDR)) {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
      "Missing parameters - serverAddr", message);
  }

  string serverAddr=param[SERVER_ADDR];
  SystemManager::SetNVRam(NVRAM_NETWORK_CLOUD_FILESERVER, serverAddr, false);
  return HC_200_OK;
}

RestHTTPCode ServiceActionHandler::PostCloudUpdateCA(Variant &param, Variant &message) {
#define CAM_AGENT "camAgent"

  if (!param.HasKey(CAM_AGENT)) {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
      "Missing parameters - camAgent", message);
  }
  if (param[CAM_AGENT]!=V_STRING) {
    return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
      "Invalid parameter type - camAgent", message);
  }

  string serverAgent=param[CAM_AGENT];
  //FIXME(Recardo):Find out server address connected to cloud
  //((QICStreamerApplication*)_pApplication)->UpdateCAgent(serverAgent);
  return HC_200_OK;
}

RestHTTPCode ServiceActionHandler::PostCloudDelToken(Variant &param, Variant &message) {
  if (!param.HasKey("clientId")) {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
      "Missing parameters - clientId", message);
  }
  string clientId=(string)param["clientId"];
  HTTPAuthHelper::ClearAuthToken(clientId);
  return HC_200_OK;
}

RestHTTPCode ServiceActionHandler::PostCloudToken(Variant &param, Variant &message) {

  if (!param.HasKey("clientId")) {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
      "Missing parameters - clientId", message);
  }
  if (!param.HasKey("key")) {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
      "Missing parameters - key", message);
  }
  if (HTTPAuthHelper::Size()>10) {
    return SendFailure(HC_400_BAD_REQUEST, EC_705_TOKEN_EXCEED,
      "Maximum clientId", message);
  }

  string clientId=(string)param["clientId"];
  string key=(string)param["key"];

  HTTPAuthHelper::SetAuthKey(clientId, key);
  return HC_200_OK;
}

RestHTTPCode ServiceActionHandler::PostCloudSetting(Variant &parameters, Variant &message) {
  if (parameters.HasKey("mediator")){
    SystemManager::SetNVRam(NVRAM_NETWORK_CLOUD_SERVERAGENT, parameters["mediator"]);
    //update nvram and signal an event to cloud
  }
  else {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
      "Missing parameter: mediator", message);
  }
  return HC_200_OK;
}

RestHTTPCode ServiceActionHandler::GetCloudSetting(Variant &parameters, Variant &message) {
  message["mediator"]=SystemManager::GetNVRam(NVRAM_NETWORK_CLOUD_SERVERAGENT);
  return HC_200_OK;
}

RestHTTPCode ServiceActionHandler::PostCloudConnection(Variant &parameters, Variant &message) {
  WSClientAppProtocolHandler *pProtocolHandler=
      reinterpret_cast<WSClientAppProtocolHandler*>(_pApplication->GetProtocolHandler(PT_OUTBOUND_HTTP_WSCONNECTION));
  if (!pProtocolHandler) {
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_704_WEBSOCKET_UNSUPPORTED,
      "No support for websocket protocol", message);
  }

  if (!parameters.HasKey("enabled")) {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
      "Missing parameter: enabled", message);
  }
  string value=parameters["enabled"]?"1":"0";
  pProtocolHandler->EnableCloudConnection();
  SystemManager::SetNVRam(NVRAM_NETWORK_CLOUD_ENABLED, value);
  return HC_200_OK;
}

RestHTTPCode ServiceActionHandler::GetCloudConnection(Variant &parameters, Variant &message) {
  WSClientAppProtocolHandler *pProtocolHandler=
      reinterpret_cast<WSClientAppProtocolHandler*>(_pApplication->GetProtocolHandler(PT_OUTBOUND_HTTP_WSCONNECTION));
  if (!pProtocolHandler) {
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_704_WEBSOCKET_UNSUPPORTED,
      "Not supported websocket outbound protocol", message);
  }
  string value;
  SystemManager::GetNVRam(NVRAM_NETWORK_CLOUD_ENABLED, value);
  message["enabled"]=value=="0"?false:true;
  return HC_200_OK;
}

RestHTTPCode ServiceActionHandler::GetLivePCMStream(Variant &parameters, Variant &message) {
  //check if there is any weboskcet connection already exists
  map<uint32_t, BaseStream *> streamMap =
    _pApplication->GetStreamsManager()->FindByType(ST_OUT_WSNET_AUDIO, false);
  if (streamMap.size() >= 1) {
    string targetUri;
    BaseStream *pStream= streamMap.begin()->second;
    BaseProtocol *pProtocol=pStream->GetProtocol();

    if (pProtocol->GetType()==PT_OUTBOUND_HTTP_WSCONNECTION) {
      targetUri = reinterpret_cast<OutboundHTTPWSProtocol*>(pProtocol)->GetUrl();
    }
    message["targetUri"] = targetUri;
    message["id"] = pStream->GetUniqueId();
    message["type"] = tagToString(pStream->GetType());
    message["name"] = SystemManager::GetCUID();
    return HC_200_OK;
  }

  return SendFailure(HC_404_NOT_FOUND, EC_701_STREAM_NOT_FOUND,
    "Unable to any live pcm stream", message);

}

RestHTTPCode ServiceActionHandler::GetLiveMJPGStream(Variant &parameters, Variant &message) {
  //check if there is any weboskcet connection already exists
  map<uint32_t, BaseStream *> streamMap =
    _pApplication->GetStreamsManager()->FindByType(ST_OUT_WSNET_MJPG, false);
  if (streamMap.size()>=1) {
    string targetUri;
    BaseStream *pStream= streamMap.begin()->second;
    BaseProtocol *pProtocol=pStream->GetProtocol();

    if (pProtocol->GetType()==PT_OUTBOUND_HTTP_WSCONNECTION) {
      targetUri = reinterpret_cast<OutboundHTTPWSProtocol*>(pProtocol)->GetUrl();
    }
    message["targetUri"] = targetUri;
    message["id"] = pStream->GetUniqueId();
    message["type"] = tagToString(pStream->GetType());
    return HC_200_OK;
  }

  return SendFailure(HC_404_NOT_FOUND, EC_701_STREAM_NOT_FOUND,
    "Unable to any live mjpg stream", message);
}

bool ServiceActionHandler::IsHandShaking(string name) {
  WSClientAppProtocolHandler* pHandler=reinterpret_cast<WSClientAppProtocolHandler*>
    (_pApplication->GetProtocolHandler(PT_OUTBOUND_HTTP_WSCONNECTION));

  if (pHandler->IsServiceTimerExists(name)) {
    return true;
  }
  return false;
}


bool ServiceActionHandler::PushToServer(ws_relay_t& params) {

  WSClientAppProtocolHandler* pHandler=reinterpret_cast<WSClientAppProtocolHandler*>
    (_pApplication->GetProtocolHandler(PT_OUTBOUND_HTTP_WSCONNECTION));

  ws_param_t wsParam={true, false, false,params.errorCode,
    params.eventType, 20000, params.uri, params.name};

  //Get timestamp and generate hashvalue
  uint64_t ts;
  GETTIMESTAMP(ts);
  string ticket=numToString<uint64_t>(ts+60000, 0);
  return pHandler->InitiateConnection(wsParam, ticket);
}

RestHTTPCode ServiceActionHandler::PushMJPGStream(Variant &parameters, Variant &message) {
#define TARGET_URI   "targetUri"
#define STREAM_READY "streamReady"

  //1.check target uri is available
  if(!parameters.HasKey(TARGET_URI) || (parameters[TARGET_URI]!=V_STRING)) {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
      "Missing parameter: targetUri", message);
  }

  //2.check if there is any weboskcet connection already exists
  map<uint32_t, BaseStream *> streamMap =
    _pApplication->GetStreamsManager()->FindByType(ST_OUT_WSNET_MJPG, false);

  if (streamMap.size()>=1) {
    message[STREAM_READY]=true;
    return HC_200_OK;
  }

  if (IsHandShaking("pushmjpg")) {
    message[STREAM_READY]=false;
    return HC_200_OK;
  }

  string targetUri=(string)parameters[TARGET_URI];
  ws_relay_t config={false, EC_PUSH_MJPG, EVT_OUT_MJPG_DISCONNECTED ,targetUri, "pushmjpg"};

  if (PushToServer(config)) {
    message[STREAM_READY]=false;
  }
  return HC_200_OK;
}

RestHTTPCode ServiceActionHandler::PushPCMStream(Variant &parameters, Variant &message) {
#define TARGET_URI "targetUri"
//#define TICKET_URI "ticketUri"

  //1.check target uri is available
  if(!parameters.HasKey(TARGET_URI) || (parameters[TARGET_URI]!=V_STRING)) {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
      "Missing parameter: targetUri", message);
  }
  //if(!parameters.HasKey(TICKET_URI) || (parameters[TICKET_URI]!=V_STRING)) {
  //  return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
  //    "Missing parameter: ticketUri", message);
  //}

  //2.check if there is any weboskcet connection already exists
  map<uint32_t, BaseStream *> streamMap =
    _pApplication->GetStreamsManager()->FindByType(ST_OUT_WSNET_AUDIO, false);

  if (streamMap.size()>=1) {
    message[STREAM_READY]=true;
    return HC_200_OK;
  }
  if (IsHandShaking("pushwave")) {
    message[STREAM_READY]=false;
    return HC_200_OK;
  }

  string targetUri=(string)parameters[TARGET_URI];
  ws_relay_t config={false, EC_PUSH_AUDIO, EVT_OUT_AREC_DISCONNECTED, targetUri, "pushwave"};
  if (PushToServer(config)) {
    message[STREAM_READY]=false;
  }
  return HC_200_OK;
}



RestHTTPCode ServiceActionHandler::PushAVCStream(Variant &parameters, Variant &message) {

  //1.check target uri is available
  if(!parameters.HasKey(TARGET_URI) || (parameters[TARGET_URI]!=V_STRING)) {
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
      "Missing parameter: targetUri", message);
  }

  //2.check if there is any weboskcet connection already exists
  map<uint32_t, BaseStream *> streamMap =
    _pApplication->GetStreamsManager()->FindByType(ST_OUT_WSNET_AVC, false);
  if (streamMap.size()>=1) {
    message[STREAM_READY]=true;
    return HC_200_OK;
  }

  if (IsHandShaking("pushavc")) {
    message[STREAM_READY]=false;
    return HC_200_OK;
  }

  string targetUri=(string)parameters[TARGET_URI];
  ws_relay_t config={false, EC_PUSH_AVC, EVT_OUT_AVC_DISCONNECTED ,targetUri,   "pushavc"};

  if (PushToServer(config)) {
    message[STREAM_READY]=false;
  }
  return HC_200_OK;
}

//bool ServiceActionHandler::GetLiveStreamInfo(uint32_t uniqueId, Variant &info) {
//  BaseStream *pStream = _pApplication->GetStreamsManager()->FindByUniqueId(uniqueId);
//  if(pStream == NULL){
//    return false;
//  }
//  BaseProtocol *pProtocol = pStream->GetProtocol();
//  if(pProtocol != NULL){
//    IOHandler *pHandler = pProtocol->GetIOHandler();
//    //DEBUG("%s", STR(IOHandler::IOHTToString(pHandler->GetType())));
//    if(pHandler != NULL){
//      if(pHandler->GetType() == IOHT_TCP_CARRIER){
//        info["airportIP"] = ((TCPCarrier*)pHandler)->GetFarEndpointAddressIp();
//        info["airportPort"] = ((TCPCarrier*)pHandler)->GetFarEndpointPort();
//      }else if(pHandler->GetType() ==  IOHT_UDP_CARRIER){
//        info["airportIP"] = ((UDPCarrier*)pHandler)->GetFarEndpointAddress();
//        info["airportPort"] = ((UDPCarrier*)pHandler)->GetFarEndpointPort();
//      }
//    }
//  }
//  info["id"] = pStream->GetUniqueId();
//  info["type"] = STR(tagToString(pStream->GetType()));
//  info["name"] = pStream->GetName();
//  return true;
//}

RestHTTPCode ServiceActionHandler::GetLiveAVCStream(Variant &parameters, Variant &message) {

#if 1
  //check if there is any weboskcet connection already exists
  map<uint32_t, BaseStream *> streamMap =
    _pApplication->GetStreamsManager()->FindByType(ST_OUT_WSNET_AVC, false);
  if (streamMap.size()>=1) {
    string targetUri;
    BaseStream *pStream= streamMap.begin()->second;
    BaseProtocol *pProtocol=pStream->GetProtocol();

    if (pProtocol->GetType()==PT_OUTBOUND_HTTP_WSCONNECTION) {
      targetUri = reinterpret_cast<OutboundHTTPWSProtocol*>(pProtocol)->GetUrl();
    }
    message["targetUri"] = targetUri;
    message["id"] = pStream->GetUniqueId();
    message["type"] = tagToString(pStream->GetType());
    return HC_200_OK;
  }


  return SendFailure(HC_404_NOT_FOUND, EC_701_STREAM_NOT_FOUND,
    "Unable to any live avc stream", message);
#else
  if(!IsAVCStreamLive()){
    return SendFailure(HC_404_NOT_FOUND, EC_701_STREAM_NOT_FOUND,
      "Unable to find live avc stream", message);
  }
  QICStreamerApplication *pQICApplication = (QICStreamerApplication *)_pApplication;
  if(!GetLiveStreamInfo(pQICApplication->GetLiveAVCStreamId(), message)){
    FATAL("Doing handshake with airport. Unable to push avc stream again");
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_701_STREAM_NOT_FOUND,
      "Doing handshake with airport. Unable to push avc stream again", message);
  }
  return HC_200_OK;
#endif
}

//bool ServiceActionHandler::IsAVCStreamLive(){
//  QICStreamerApplication *pQICApplication = (QICStreamerApplication *)_pApplication;
//  return pQICApplication->IsAVCStreamLive();
//}
//
//bool ServiceActionHandler::InitiateForwardingStream(BaseInStream *pStream, Variant &target) {
//  return true;
//#ifndef HAS_PROTOCOL_RTMP
//  FATAL("RTMP protocol not supported");
//  return false;
//#else
//  //1. Filter the stream
//  if (target.HasKey("localStreamName")) {
//    if (((string) target["localStreamName"]) != pStream->GetName()) {
//      if (pStream->GetName().find(((string) target["localStreamName"]) + "?") != 0) {
//        FINEST("Stream name not matching. Wanted: %s(?); Got: %s",
//            STR(target["localStreamName"]),
//            STR(pStream->GetName()));
//        return true;
//      }
//    }
//  }
//
//  //2. Compute the target stream name
//  Variant parameters = target;
//  if (!parameters.HasKey("targetStreamName"))
//    parameters["targetStreamName"] = pStream->GetName();
//
//  //3. Some nice info
//  INFO("Initiate forward stream %u of type %s with name `%s` owned by application `%s` to server %s with name `%s`",
//      pStream->GetUniqueId(),
//      STR(tagToString(pStream->GetType())),
//      STR(pStream->GetName()),
//      STR(_pApplication->GetName()),
//      STR((string) target["targetUri"]),
//      STR(parameters["targetStreamName"]));
//
//  //4. Push stream
//  return _pApplication->PushLocalStream(parameters);
//
//  //4. Enqueue the push
//  //EnqueuePush(parameters);
//#endif /* HAS_PROTOCOL_RTMP */
//}

RestHTTPCode ServiceActionHandler::PostCloudUnbind(Variant &params, Variant &message) {
  string cuid, privateKey;
  string paramKey;

  if (params.HasKey("privateKey")) {
    paramKey=(string)params["privateKey"];
  }
  else {
    message["status"]="fail";
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "no privateKey specified", message);
  }
  privateKey = SystemManager::GetPrivateKey();
  if (paramKey != privateKey) {
    message["status"]="fail";
    return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
        "invalid privateKey", message);
  }

  string result;

  //Make sure cuid and privateKey are empty
  SystemManager::SetNVRam(NVRAM_UNBINDING, "1", true);
  message["status"] = "success";
  return HC_202_ACCEPTED;
}

#endif
