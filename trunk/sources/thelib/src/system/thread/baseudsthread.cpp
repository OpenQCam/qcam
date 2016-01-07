/*
 * Copyright (c) 2013, Three Ocean (to@bcloud.us). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "system/thread/baseudsthread.h"
#include "system/thread/threadmanager.h"

BaseUDSThread::BaseUDSThread(THREAD_TYPE type, string socketName)
  : Thread(type, true)
  , _socketName(socketName)
  , _pUDSClient(0)
  , _pUDSMsgProtocol(0)
{
}

BaseUDSThread::~BaseUDSThread()
{
  INFO("command thread was deleted");
  if(_pUDSClient != NULL)
    delete _pUDSClient;

  _pUDSMsgProtocol = NULL;
}

void* BaseUDSThread::Run() {
  _pUDSClient = new UnixDomainSocketClient(_socketName);
  if(_pUDSClient == NULL) {
    FATAL("Unable to create uds client");
    return NULL;
  }
  if(_pUDSClient->Connect()){
    _pUDSClient->RegisterObserver((IUDSMessageObserver *)(this));
    OnConnected();
    for(;;){
      if(!_pUDSClient->Run()){
        _pUDSClient->Close();
        break;
      }
    }
  }else{
    FATAL("Unable to connect\n");
  }
  return NULL;
}

bool BaseUDSThread::SetUDSMessageProtocol(BaseUDSMessageProtocol *pProtocol)
{
  if(_pUDSMsgProtocol != NULL) {
    FATAL("thread already had protocol");
    return false;
  }
  _pUDSMsgProtocol = pProtocol;
  return true;
}

BaseUDSMessageProtocol *BaseUDSThread::GetUDSMessageProtocol()
{
  return _pUDSMsgProtocol;
}

bool BaseUDSThread::Free()
{
  INFO("free type %d", GetType());
  return ThreadManager::EnqueueThread(GetType(), this);
}

bool BaseUDSThread::RegisterThreadObserver(IUDSMessageObserver *pObserver)
{
  if(_pUDSMsgProtocol == NULL) {
    FATAL("Unable to find uds protocol");
    return false;
  }
  _pUDSMsgProtocol->RegisterObserver(pObserver);
  return true;
}
void BaseUDSThread::RemoveThreadObserver()
{
  if(_pUDSMsgProtocol != NULL) {
    _pUDSMsgProtocol->RemoveObserver();
  }
}

bool BaseUDSThread::SendMessageToThread(Variant &message)
{
  if(_pUDSMsgProtocol == NULL){
    FATAL("Unable to find uds protocol");
    return false;
  }
  return _pUDSMsgProtocol->SendMessage(message);
}

bool BaseUDSThread::SendDataToThread(UDS_MESSAGE_TYPE type, uint8_t* pData, uint32_t length, uint64_t ts)
{
  if(_pUDSMsgProtocol == NULL){
    FATAL("Unable to find uds protocol");
    return false;
  }
  return _pUDSMsgProtocol->SendData(type, pData, length, ts);

}

bool BaseUDSThread::SendMessageToMainThread(Variant &message)
{
  return _pUDSClient->SendMessage(message);
}

bool BaseUDSThread::SendDataToMainThread(UDS_MESSAGE_TYPE type, uint8_t* pData, uint32_t length, uint64_t ts)
{
  return _pUDSClient->SendData(type, pData, length, ts);
}


