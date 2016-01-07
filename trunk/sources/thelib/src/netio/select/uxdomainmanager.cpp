#include "netio/select/uxdomainmanager.h"
#include "system/systemdefine.h"
#include "protocols/unixdomainsocket/unixdomainsocketprotocol.h"
#include "protocols/cli/restclidefine.h"


//XXX(Tim): callbackId 0 is reserved for calling system command but doesn't care
uint32_t ISystemCommandCallback::_callbackIdGenerator = 1;

ISystemCommandCallback::ISystemCommandCallback() {
  _callbackId = ++_callbackIdGenerator;
  UnixDomainSocketManager::RegisterSystemCommandCallback(this);
}
ISystemCommandCallback::~ISystemCommandCallback() {
  UnixDomainSocketManager::UnRegisterSystemCommandCallback(this);
}

uint32_t ISystemCommandCallback::GetCallbackId() {
  return _callbackId;
}

uint32_t UnixDomainSocketManager::_uniqueMessageId = 0;
map<uint32_t, ISystemCommandCallback *> UnixDomainSocketManager::_uxSocketCallbackMap;  // <callbackId, callbackPointer>
map<uint32_t, uint32_t> UnixDomainSocketManager::_messageQueue; // <msgId, callbackId>
map<string, UnixDomainSocketProtocol *> UnixDomainSocketManager::_uxSocketProtocolMap;


bool UnixDomainSocketManager::RegisterUXThreadProtocol(string sockName, UnixDomainSocketProtocol *protocol) {

  if(MAP_HAS1(_uxSocketProtocolMap, sockName))
    return false;

  _uxSocketProtocolMap[sockName] = protocol;
  return true;
}

bool UnixDomainSocketManager::UnRegisterUXThreadProtocol(string sockName) {

  if(!MAP_HAS1(_uxSocketProtocolMap, sockName))
    return false;

  MAP_ERASE1(_uxSocketProtocolMap, sockName);
  return true;
}

void UnixDomainSocketManager::RegisterSystemCommandCallback(ISystemCommandCallback *cb) {
  uint32_t id = cb->GetCallbackId();
  if(!MAP_HAS1(_uxSocketCallbackMap, id)){
    _uxSocketCallbackMap[id] = cb;
  }
}

void UnixDomainSocketManager::UnRegisterSystemCommandCallback(ISystemCommandCallback *cb) {
  uint32_t id = cb->GetCallbackId();
  if(MAP_HAS1(_uxSocketCallbackMap, id)){
    MAP_ERASE1(_uxSocketCallbackMap, id);
  }
}

void UnixDomainSocketManager::SendDataToUXCallback(uint32_t msgId,
                                                   uint8_t *pData,
                                                   uint32_t dataLength){
  //get back callback information
  uint32_t cbId = _messageQueue[msgId];
  // Check if callback pointer is still available
  if(MAP_HAS1(_uxSocketCallbackMap, cbId)){
    ISystemCommandCallback *cb = _uxSocketCallbackMap[cbId];
    cb->OnCmdFinished(msgId, pData, dataLength);
  }
  //done. remove message from queue
  MAP_ERASE1(_messageQueue, msgId);
}

uint32_t UnixDomainSocketManager::SendCmdToInfoThread(string cmd,
                                                      uint32_t type,
                                                      uint32_t cbId) {

  if(!MAP_HAS1(_uxSocketProtocolMap, UX_SOCKET_INFO)){
    WARN("%s didn't exist, exec cmd:%s", UX_SOCKET_INFO, STR(cmd));
    return 0;
  }

  UnixDomainSocketProtocol *pProtocol = _uxSocketProtocolMap[UX_SOCKET_INFO];
  uint32_t msgId = UnixDomainSocketManager::GetMessageId();

  //keep callback information
  _messageQueue[msgId] = cbId;
  uint32_t length = cmd.length();
  uint8_t padBytes[PADDED_BYTES(length)];

  pProtocol->SendDataToProtocol((uint8_t*)&type, sizeof(type));
  pProtocol->SendDataToProtocol((uint8_t*)&msgId, sizeof(msgId));
  pProtocol->SendDataToProtocol((uint8_t*)&length, sizeof(length));
  pProtocol->SendDataToProtocol((uint8_t*)STR(cmd), length);
  pProtocol->SendDataToProtocol(padBytes, sizeof(padBytes));
  return msgId;
}


#if 0
bool UnixDomainSocketManager::SendCmdToCVRThread(uint32_t msgType,
                                                 uint32_t length,
                                                 uint8_t* pBuffer) {
  if(!MAP_HAS1(_uxSocketProtocolMap, UX_SOCKET_CVR)){
    WARN("%s didn't exist", UX_SOCKET_CVR);
    return false;
  }
  UnixDomainSocketProtocol *pProtocol = _uxSocketProtocolMap[UX_SOCKET_CVR];
  if (pProtocol) {
    pProtocol->SendDataToProtocol((uint8_t*)&msgType, sizeof(msgType));
    pProtocol->SendDataToProtocol((uint8_t*)&length, sizeof(length));
    if (pBuffer) {
      uint8_t padBytes[PADDED_BYTES(length)];
      pProtocol->SendDataToProtocol(pBuffer, length);
      //Send padded bytes
      pProtocol->SendDataToProtocol(padBytes, sizeof(padBytes));
    }
  }
  else {
    WARN("no %s domain protocol", UX_SOCKET_CVR);
    return false;
  }
  return true;
}
#endif

bool UnixDomainSocketManager::SendDataToCVRThread(uint32_t msgType,
                                                  uint64_t ts,
                                                  uint32_t length,
                                                  uint8_t* pBuffer) {
  if(!MAP_HAS1(_uxSocketProtocolMap, UX_SOCKET_CVR)){
    WARN("%s didn't exist", UX_SOCKET_CVR);
    return false;
  }
  UnixDomainSocketProtocol *pProtocol = _uxSocketProtocolMap[UX_SOCKET_CVR];
  if (pProtocol) {
    pProtocol->SendDataToProtocol((uint8_t*)&msgType, sizeof(msgType));
    pProtocol->SendDataToProtocol((uint8_t*)&ts, sizeof(ts));
    if (pBuffer) {
      uint8_t padBytes[PADDED_BYTES(length+16)];
      pProtocol->SendDataToProtocol((uint8_t*)&length, sizeof(length));
      pProtocol->SendDataToProtocol(pBuffer, length);
      pProtocol->SendDataToProtocol(padBytes, sizeof(padBytes));
    }
  }
  else {
    WARN("no %s domain protocol", UX_SOCKET_CVR);
    return false;
  }
  return true;
}

uint32_t UnixDomainSocketManager::ExecuteSystemCommand(string cmd) {
  SystemCmdHeader cmdHeader = { false, cmd, 0, 0};
  return UnixDomainSocketManager::ExecuteSystemCommand(0, cmdHeader, NULL, NULL);
}

uint32_t UnixDomainSocketManager::ExecuteSystemCommand(string cmd, uint32_t cbId) {
  SystemCmdHeader cmdHeader = { false, cmd, 0, 0};
  return UnixDomainSocketManager::ExecuteSystemCommand(cbId, cmdHeader, NULL, NULL);
}

uint32_t UnixDomainSocketManager::ExecuteSyncSystemCmd(string cmd) {
  SystemCmdHeader cmdHeader = { false, cmd, 0, 0};
  return UnixDomainSocketManager::ExecuteSyncSystemCmd(0, cmdHeader, NULL, NULL);
}

uint32_t UnixDomainSocketManager::ExecuteSystemCommand(uint32_t cbId,
                                                       SystemCmdHeader& cmdHeader,
                                                       Variant* reqParams,
                                                       Variant* pScriptParams) {
  return SendUXThreadCommand(TRT_CALLBACK, cbId, cmdHeader, reqParams, pScriptParams);
}

uint32_t UnixDomainSocketManager::ExecuteSyncSystemCmd(uint32_t cbId,
                                                       SystemCmdHeader& cmdHeader,
                                                       Variant* reqParams,
                                                       Variant* pScriptParams) {
  return SendUXThreadCommand(TRT_SYNC_CALLBACK, cbId, cmdHeader, reqParams, pScriptParams);
}

uint32_t UnixDomainSocketManager::SendUXThreadCommand(UXThreadRequestIdType type,
                                                      uint32_t cbId,
                                                      SystemCmdHeader& cmdHeader,
                                                      Variant* reqParams,
                                                      Variant* pScriptParams) {
  Variant req;
  string reqBin;
  if(!MAP_HAS1(_uxSocketProtocolMap, UX_SOCKET_SYSTEM)){
    WARN("%s didn't exist, exec cmd:%s", UX_SOCKET_SYSTEM, STR(cmdHeader.cmd));
    return 0;
  }
  UnixDomainSocketProtocol *pProtocol = _uxSocketProtocolMap[UX_SOCKET_SYSTEM];
  uint32_t msgId = UnixDomainSocketManager::GetMessageId();
  //keep callback information
  _messageQueue[msgId] = cbId;
  req[REST_CMD_STR] = cmdHeader.cmd;
  req[REST_ISJSON_STR] = cmdHeader.isJSON;
  req[REST_PID_STR] = cmdHeader.pId;
  req[REST_UTID_STR] = cmdHeader.utId;

  if (pScriptParams)
    req[REST_SCRIPTPARAM_STR]=(*pScriptParams);
  if (reqParams)
    req[REST_REQPARAMS_STR]=(*reqParams);

  req.SerializeToJSON(reqBin);
  uint32_t length = reqBin.length();
  uint8_t padBytes[PADDED_BYTES(length)];
  pProtocol->SendDataToProtocol((uint8_t*)&type, sizeof(type));
  pProtocol->SendDataToProtocol((uint8_t*)&msgId, sizeof(msgId));
  pProtocol->SendDataToProtocol((uint8_t*)&length, sizeof(length));
  pProtocol->SendDataToProtocol((uint8_t*)STR(reqBin), length);
  pProtocol->SendDataToProtocol(padBytes, sizeof(padBytes));

  return msgId;
}

uint32_t UnixDomainSocketManager::SendResponseToIpcEvent(string message) {

  if(!MAP_HAS1(_uxSocketProtocolMap, UX_SOCKET_IPC_EVENT)){
    WARN("%s didn't exist", UX_SOCKET_IPC_EVENT);
    return 0;
  }

  UnixDomainSocketProtocol *pProtocol = _uxSocketProtocolMap[UX_SOCKET_IPC_EVENT];
  uint32_t msgId = UnixDomainSocketManager::GetMessageId();

  //keep callback information
  //_messageQueue[msgId] = cbId;
  /* message.length()"+1" is because the extra null-terminated char for C string*/
  pProtocol->SendDataToProtocol((uint8_t*)STR(message), (message.length()+1));
  return msgId;
}

uint32_t UnixDomainSocketManager::GetMessageId() {
  return ++_uniqueMessageId;
}

