
#ifndef _UXDOMAINMANAGER_H
#define _UXDOMAINMANAGER_H

#include "common.h"

enum UXThreadRequestIdType {
  TRT_NONE = 0,
  TRT_CALLBACK,
  TRT_SYNC_CALLBACK,
};

typedef struct _SystemThreadHeader {
  bool isJSON;
  string cmd;
  uint32_t pId;
  uint64_t utId;
} SystemCmdHeader;


class ISystemCommandCallback{
  private:
    static uint32_t _callbackIdGenerator;
    uint32_t _callbackId;
  public:
    virtual ~ISystemCommandCallback();
    ISystemCommandCallback();
    uint32_t GetCallbackId();
    virtual void OnCmdFinished(uint32_t msgId, uint8_t* pData,
                               uint32_t dataLength) = 0;
};

class UnixDomainSocketProtocol;

class UnixDomainSocketManager {
  private:
    static map<uint32_t, ISystemCommandCallback *> _uxSocketCallbackMap;  // <callbackId, callbackPointer>
    static map<string, UnixDomainSocketProtocol *> _uxSocketProtocolMap;
    static map<uint32_t, uint32_t> _messageQueue; // <msgId, callbackId>
    static uint32_t _uniqueMessageId;

    static uint32_t GetMessageId();
  public:
    static bool RegisterUXThreadProtocol(string sockName,
                                         UnixDomainSocketProtocol *protocol);
    static bool UnRegisterUXThreadProtocol(string sockName);
    static uint32_t SendUXThreadCommand(UXThreadRequestIdType type,
                                        uint32_t cbId,
                                        SystemCmdHeader& cmdHeader,
                                        Variant* reqParams,
                                        Variant* pScriptParams);
    static void RegisterSystemCommandCallback(ISystemCommandCallback *cb);
    static void UnRegisterSystemCommandCallback(ISystemCommandCallback *cb);
    static void SendDataToUXCallback(uint32_t msgId, uint8_t *pData,
                                     uint32_t dataLength);
    static bool SendDataToCVRThread(uint32_t msgType, uint64_t ts,
                                    uint32_t length, uint8_t* pBuffer);
    //static bool SendCmdToCVRThread(uint32_t msgType, uint32_t length, uint8_t* pBuffer);

    static uint32_t SendCmdToInfoThread(string cmd, uint32_t type, uint32_t cbId);
    static uint32_t ExecuteSystemCommand(string cmd);
    static uint32_t ExecuteSystemCommand(string cmd, uint32_t cbId);
    static uint32_t ExecuteSystemCommand(uint32_t cbId,
                                         SystemCmdHeader& cmdHeader,
                                         Variant* reqParams,
                                         Variant* scriptParams);
    static uint32_t ExecuteSyncSystemCmd(string cmd);
    static uint32_t ExecuteSyncSystemCmd(uint32_t cbId,
                                         SystemCmdHeader& cmdHeader,
                                         Variant* reqParams,
                                         Variant* scriptParams);
    static uint32_t SendResponseToIpcEvent(string message);
};

#endif
