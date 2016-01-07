/* ==================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 * (C) COPYRIGHT Quanta Computer Inc. ALL RIGHTS RESERVED
 *
 * ==================================================================
 * ------------------------------------------------------------------
 *  Date    :
 *  Version :
 *  Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 *  Purpose :
 *  =================================================================
 */

#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET

#include "appprotocolhandler/uxdomainsocketappprotocolhandler.h"
#include "qicstreamerapplication.h"
#include "actionhandler/actionrouter.h"

#include "system/systemdefine.h"
#include "system/eventdefine.h"
#include "system/systemmanager.h"
#include "streaming/streamsmanager.h"
#include "streaming/basestream.h"
#include "protocols/protocolmanager.h"
#include "protocols/cli/restclidefine.h"
#include "protocols/unixdomainsocket/unixdomainsocketprotocol.h"
#include "protocols/http/basehttpwsprotocol.h"
//hardware
#include "hardwaremanager.h"
#include "system/systemmanager.h"
#include "qic/qicmjpgvideocapture.h"
#include "video/baseavcvideocapture.h"
#include "video/basevideostream.h"

using namespace app_qicstreamer;

UXDomainSocketAppProtocolHandler::UXDomainSocketAppProtocolHandler(Variant& configuration)
: BaseUXDomainSocketAppProtocolHandler(configuration) {
  _uxSocketMap[UX_SOCKET_CVR] = reinterpret_cast<SocketFunction>(&UXDomainSocketAppProtocolHandler::CVRSocket);
  _uxSocketMap[UX_SOCKET_SYSTEM] = reinterpret_cast<SocketFunction>(&UXDomainSocketAppProtocolHandler::SystemSocket);
  _uxSocketMap[UX_SOCKET_INFO] = reinterpret_cast<SocketFunction>(&UXDomainSocketAppProtocolHandler::InfoSocket);
  _uxSocketMap[UX_SOCKET_EVENT] = reinterpret_cast<SocketFunction>(&UXDomainSocketAppProtocolHandler::EventSocket);
  _uxSocketMap[UX_SOCKET_IPC_EVENT] = reinterpret_cast<SocketFunction>(&UXDomainSocketAppProtocolHandler::IpcEventSocket);
}


UXDomainSocketAppProtocolHandler::~UXDomainSocketAppProtocolHandler() {
}

bool UXDomainSocketAppProtocolHandler::ProcessMessage(UnixDomainSocketProtocol *pFrom, IOBuffer &buffer) {
  string socketName = pFrom->GetSocketName();
  if(MAP_HAS1(_uxSocketMap, socketName)){
    SocketFunction pFunc = _uxSocketMap[socketName];
    return ((this->*pFunc)(pFrom, buffer));
  } else {
    WARN("socket %s: Not implemented yet", STR(socketName));
  }
  return true;
}

bool UXDomainSocketAppProtocolHandler::CVRSocket(UnixDomainSocketProtocol *pFrom, IOBuffer &buffer) {
  /*FIXME(Recardo): take care of msg length is 4-byte aligned*/
  //DEBUG ("CVR socket:%d", GETAVAILABLEBYTESCOUNT(buffer));
  reinterpret_cast<QICStreamerApplication*>(GetApplication())->OnCVRNotify(buffer);
  return true;
}

/*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-------+-+-------------+-------------------------------+
 |                           length                              |
 +-+-+-+-+-------+-+-------------+-------------------------------+
 |   msg type                    |           sub type            |
 ++-+-+-+-+-------+-+--------------------------------------------+
 |                           resv1                               |
 +-+-+-+-+-------+-+---------------------------------------------+
 |                           resv2                               |
 +-+-+-+-+-------+-+---------------------------------------------+
 |                           binary                              |
 +-+-+-+-+-------+-+---------------------------------------------+
 |                       padded data                             |
 +-+-+-+-+-------+-+---------------------------------------------+
 ...
*/


bool UXDomainSocketAppProtocolHandler::EventSocket(UnixDomainSocketProtocol *pFrom, IOBuffer &buffer) {
  uint32_t buflen = GETAVAILABLEBYTESCOUNT(buffer);

  while (buflen>=16) {
    uint8_t* pBuf=GETIBPOINTER(buffer);

    CHK_ALIGN4(pBuf);
    uint32_t msglen=*((uint32_t*)(pBuf+12));
    uint32_t total = PADDED_TOTAL(msglen+16);
    string data;

    if (buflen < total) {
      break;
    }
    uint16_t type=*((uint16_t*)(pBuf));
    uint32_t subType=*((uint16_t*)(pBuf+2));
    uint64_t eventInfo=0;

    DEBUG ("type:%d, subtype:%d", type, subType);
    //message has data
    if (msglen) {
      string temp((char*)(pBuf+16), msglen);
      data=temp;
    }
    QICStreamerApplication *pApp=
        static_cast<QICStreamerApplication*>(GetApplication());

    switch (type) {
      case (CLOUD_MSG_ERROR):
        pApp->SendError(subType, data);
        HardwareManager::SetStatus(subType, false);
        break;
      case (CLOUD_MSG_EVENT):
        eventInfo=*((uint64_t*)(pBuf+4));
        pApp->SendEvent(subType, eventInfo, data);
        break;
      case (DEVICE_EVENT):
        {
          RestCLIMessage restMessage;
          uint32_t start=0;
          ActionRouter *pActionRouter=pApp->GetActionRouter();
          Variant::DeserializeFromJSON(data, restMessage.request, start);
          pActionRouter->RouteRequestAction(restMessage);
        }
        break;
    }
    buffer.Ignore(total);
    buflen = GETAVAILABLEBYTESCOUNT(buffer);
  }

  return true;
}

bool UXDomainSocketAppProtocolHandler::InfoSocket(UnixDomainSocketProtocol *pFrom, IOBuffer &buffer) {
  uint32_t avail=GETAVAILABLEBYTESCOUNT(buffer);
  QICStreamerApplication* pApp=
      reinterpret_cast<QICStreamerApplication*>(GetApplication());


  //DEBUG ("data:%s", STR(buffer.ToString()));
  while (avail>=12) {
    uint8_t* pBuf=GETIBPOINTER(buffer);
    CHK_ALIGN4(pBuf);
    uint32_t type=*((uint32_t*)(pBuf));
    uint32_t msgId=*((uint32_t*)(pBuf+4));
    uint32_t length=*((uint32_t*)(pBuf+8));
    uint32_t total=PADDED_TOTAL(length+12);

    if (avail<total)
      break;

    UnixDomainSocketManager::SendDataToUXCallback(msgId, (uint8_t*)(GETIBPOINTER(buffer)+12), length);

    if (type==INFO_OP_SYSTEMINFO) {
      pApp->SendInfo(INFO_SYSTEM, (uint8_t*)(GETIBPOINTER(buffer)+12), length);
    }
    else if (type==INFO_OP_CMD && length) {
      uint32_t errorCode=*((uint32_t*)(pBuf+12));

      if (errorCode!=EC_RESERVED) {
        pApp->SendError(ERROR_CODE(errorCode), ERROR_DESCRIPTION(errorCode));
      }
    }

    buffer.Ignore(total);
    avail=GETAVAILABLEBYTESCOUNT(buffer);
  }
  return true;
}

bool UXDomainSocketAppProtocolHandler::SystemSocket(UnixDomainSocketProtocol *pFrom, IOBuffer &buffer) {
  ParseUXDomainSocketData(_qSystemSocketData, buffer);
  while (!_qSystemSocketData.empty()) {
    UXSocketData *pSocketData=_qSystemSocketData.front();
    _qSystemSocketData.pop();
    string responseBin((char*)GETIBPOINTER(pSocketData->dataBuffer), GETAVAILABLEBYTESCOUNT(pSocketData->dataBuffer));
    Variant resp;
    if (Variant::DeserializeFromBin(responseBin, resp)) {

      string payload=(string)resp[REST_PAYLOAD_STR];
      UnixDomainSocketManager::SendDataToUXCallback(pSocketData->id, (uint8_t*)payload.c_str(), payload.length());

      (reinterpret_cast<QICStreamerApplication*>(GetApplication()))->SendCmdResp(payload,
                                                                                 (uint32_t)resp[REST_UTID_STR],
                                                                                 (uint64_t)resp[REST_PID_STR]);
      if (pSocketData->type==TRT_SYNC_CALLBACK) {
        (reinterpret_cast<QICStreamerApplication*>(GetApplication()))->SendSync(resp);
      }
    }
    delete pSocketData;
  }
  return true;
}

/*
 0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-------+-+-------------+-------------------------------+
 |                           type                                |
 +-+-+-+-+-------+-+-------------+-------------------------------+
 |                           msgId                               |
 ++-+-+-+-+-------+-+--------------------------------------------+
 |                           length                              |
 +-+-+-+-+-------+-+---------------------------------------------+
 |                           payload                             |
 +-+-+-+-+-------+-+---------------------------------------------+
 |               |        padded data                            |
 +-+-+-+-+-------+-+---------------------------------------------+
*/

inline void UXDomainSocketAppProtocolHandler::ParseUXDomainSocketData(queue<UXSocketData*> &socketData, IOBuffer &buffer) {
  uint32_t length = GETAVAILABLEBYTESCOUNT(buffer);

  while (length>=12) {
    UXSocketData *pSocketData = (UXSocketData*) new UXSocketData();
    uint8_t *pBuf = GETIBPOINTER(buffer);

    CHK_ALIGN4(pBuf);
    pSocketData->type = (*pBuf);
    pSocketData->id = *((uint32_t*)(pBuf+4));
    pSocketData->dataLength = *((uint32_t*)(pBuf+8));
    uint32_t total=PADDED_TOTAL(pSocketData->dataLength+12);

    if (length<total) {
      delete pSocketData;
      break;
    } else {
      pSocketData->dataBuffer.ReadFromBuffer(pBuf+12, pSocketData->dataLength);
      buffer.Ignore(total);
      socketData.push(pSocketData);
    }
    length = GETAVAILABLEBYTESCOUNT(buffer);
  }
}
bool UXDomainSocketAppProtocolHandler::IpcEventSocket(UnixDomainSocketProtocol *pFrom, IOBuffer &buffer) {
    uint32_t length = GETAVAILABLEBYTESCOUNT(buffer);
    uint8_t *pBuf = GETIBPOINTER(buffer);
    BaseAVCVideoCapture *_pAVCCaptureInstance;

    _pAVCCaptureInstance = reinterpret_cast<BaseAVCVideoCapture *>(HardwareManager::GetHardwareInstance(HT_VIDEO_AVC));

    printf ("buf length (%d): %s\n", length, pBuf);
    buffer.Ignore(length);

    _pAVCCaptureInstance->VerifyResolution(640,360);
    _pAVCCaptureInstance->SetResolution(640,360);

    UnixDomainSocketManager::SendResponseToIpcEvent ("2");

    return true;
}
#endif /* HAS_PROTOCOL_UNIXDOMAIN_SOCKET */
