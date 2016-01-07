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


#ifdef NET_SELECT
#include "netio/select/unixdomainsocketcarrier.h"
#include "netio/select/iohandlermanager.h"
#include "netio/select/uxdomainmanager.h"
#include "protocols/baseprotocol.h"

#define ENABLE_WRITE_DATA \
if (!_writeDataEnabled) { \
    _writeDataEnabled = true; \
    IOHandlerManager::EnableWriteData(this); \
} \
_enableWriteDataCalled=true;

#define DISABLE_WRITE_DATA \
if (_writeDataEnabled) { \
  _enableWriteDataCalled=false; \
  _pProtocol->ReadyForSend(); \
  if(!_enableWriteDataCalled) { \
    if((GETAVAILABLEBYTESCOUNT(*(_pProtocol->GetOutputBuffer())) == 0)) {\
      _writeDataEnabled = false; \
      IOHandlerManager::DisableWriteData(this); \
    } \
  } \
}

UnixDomainSocketCarrier::UnixDomainSocketCarrier(int32_t fd, string socketPath)
: IOHandler(fd, fd, IOHT_UX_CARRIER) {
  IOHandlerManager::EnableReadData(this);
  _writeDataEnabled = false;
  _enableWriteDataCalled = false;
  _sendBufferSize = 1024;
  _recvBufferSize = 1024;
  _rx = 0;
  _tx = 0;
  _ioAmount = 0;

  uint32_t pos = socketPath.find_last_of('/');
  if(pos == string::npos){
    _socketName = socketPath;
  }else{
    _socketName = socketPath.substr(pos+1);
  }
}

UnixDomainSocketCarrier::~UnixDomainSocketCarrier() {
  CLOSE_SOCKET(_inboundFd);
  UnixDomainSocketManager::UnRegisterUXThreadProtocol(_socketName);
}

bool UnixDomainSocketCarrier::OnEvent(select_event &event) {
  //3. Do the I/O
  switch (event.type) {
    case SET_READ:
    {
      IOBuffer *pInputBuffer = _pProtocol->GetInputBuffer();
      o_assert(pInputBuffer != NULL);
      if (!pInputBuffer->ReadFromUxSocketFd(_inboundFd,
          _recvBufferSize, _ioAmount)) {
        FATAL("Unable to read data from unix domain socket");
        return false;
      }
      _rx += _ioAmount;
      //ADD_IN_BYTES_MANAGED(_type, _ioAmount);
      return _pProtocol->SignalInputData(_ioAmount);
    }
    case SET_WRITE:
    {
      IOBuffer *pOutputBuffer = _pProtocol->GetOutputBuffer();
      while(GETAVAILABLEBYTESCOUNT(*pOutputBuffer) > 0){
        if (!pOutputBuffer->WriteToUxSocketFd(_outboundFd,
            GETAVAILABLEBYTESCOUNT(*pOutputBuffer), _ioAmount)) {
          FATAL("Unable to send data to unix domain socket");
          IOHandlerManager::EnqueueForDelete(this);
          return false;
        }
        _tx += _ioAmount;
      }
      DISABLE_WRITE_DATA;
      //INFO("unix socket write event %d",GETAVAILABLEBYTESCOUNT(*pOutputBuffer) );
      /*
      IOBuffer *pOutputBuffer = NULL;
      while ((pOutputBuffer = _pProtocol->GetOutputBuffer()) != NULL) {
        INFO("unix socket write event %d",GETAVAILABLEBYTESCOUNT(*pOutputBuffer) );
        if (!pOutputBuffer->WriteToUxSocketFd(_outboundFd,
            GETAVAILABLEBYTESCOUNT(*pOutputBuffer), _ioAmount)) {
          FATAL("Unable to send data to unix domain socket");
          IOHandlerManager::EnqueueForDelete(this);
          return false;
        }
        _tx += _ioAmount;
        if (GETAVAILABLEBYTESCOUNT(*pOutputBuffer) > 0) {
          ENABLE_WRITE_DATA;
          break;
        }
      }
      if (pOutputBuffer == NULL) {
        DISABLE_WRITE_DATA;
      }
      */
      return true;
    }
    default:
    {
      ASSERT("Invalid state: %hhu", event.type);
      return false;
    }
  }
}

bool UnixDomainSocketCarrier::SignalOutputData() {
  ENABLE_WRITE_DATA;
  return true;
}

UnixDomainSocketCarrier::operator string() {
  if (_pProtocol != NULL)
    return STR(*_pProtocol);
  return format("TCP(%d)", _inboundFd);
}

void UnixDomainSocketCarrier::GetStats(Variant &info, uint32_t namespaceId) {
  info["type"] = "IOHT_UX_CARRIER";
  info["rx"] = _rx;
  info["tx"] = _tx;
}

string UnixDomainSocketCarrier::GetSocketName() {
  return _socketName;
}


#endif /* NET_SELECT */

