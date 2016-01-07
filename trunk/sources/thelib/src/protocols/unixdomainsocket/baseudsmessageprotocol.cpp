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


#include "protocols/unixdomainsocket/baseudsmessageprotocol.h"

#include "application/baseclientapplication.h"
#include "protocols/unixdomainsocket/baseuxdomainsocketappprotocolhandler.h"

BaseUDSMessageProtocol::BaseUDSMessageProtocol(uint64_t type)
  : BaseProtocol(type)
  , _pObserver(0)
{
}

BaseUDSMessageProtocol::~BaseUDSMessageProtocol() {
}

bool BaseUDSMessageProtocol::Initialize(Variant &parameters) {
  return _outputBuffer.EnsureSize(1024);
}

void BaseUDSMessageProtocol::SetApplication(BaseClientApplication *pApplication) {
  BaseProtocol::SetApplication(pApplication);
  if (pApplication != NULL) {
    _pProtocolHandler = (BaseUXDomainSocketAppProtocolHandler *)
        pApplication->GetProtocolHandler(this);
  } else {
    _pProtocolHandler = NULL;
  }
}


bool BaseUDSMessageProtocol::AllowFarProtocol(uint64_t type) {
  return type == PT_UNIX_DOMAIN_SOCKET;
}

bool BaseUDSMessageProtocol::AllowNearProtocol(uint64_t type) {
  //This is nearest protocol
  return false;
}

IOBuffer * BaseUDSMessageProtocol::GetOutputBuffer() {
  return &_outputBuffer;
  //if(GETAVAILABLEBYTESCOUNT(_outputBuffer) > 0)
  //  return &_outputBuffer;
  //return NULL;
}

bool BaseUDSMessageProtocol::SignalInputData(int32_t recvAmount) {
  NYIR;
}

bool BaseUDSMessageProtocol::SignalInputData(IOBuffer &buffer) {

  //INFO("%s", STR(buffer.ToString()));
  // if message observers don't exist, send to appprotocolhandler
  if(_pObserver == NULL) {
    if(_pProtocolHandler != NULL) {
      return _pProtocolHandler->ProcessMessage((BaseProtocol*)this, buffer);
    }
    buffer.IgnoreAll();
    WARN("No observer and appprotocolhandler");
    return false;
  }
  uint8_t *pBuf = NULL;
  uint32_t msgLength = 0;
  UDS_MESSAGE_HEADER *pHeader = NULL;
  Variant message;
  while(GETAVAILABLEBYTESCOUNT(buffer) > 0){
    pBuf = GETIBPOINTER(buffer);
    pHeader = (UDS_MESSAGE_HEADER *)pBuf;
    _messageHeader.type = pHeader->type;
    _messageHeader.length = ENTOHL(pHeader->length);
    ENTOHD(pHeader->ts, _messageHeader.ts);
    msgLength = sizeof(UDS_MESSAGE_HEADER) + _messageHeader.length;
    // incomplete buffer, wait for next buffer
    if(msgLength > GETAVAILABLEBYTESCOUNT(buffer)) {
      return true;
    }
    switch(_messageHeader.type) {
      case UDS_MESSAGE:
        Variant::DeserializeFromBin(pBuf+sizeof(UDS_MESSAGE_HEADER), _messageHeader.length, message);
        _pObserver->OnMessage(this, message);
        break;
      default:
        _pObserver->OnData(_messageHeader.type, pBuf+sizeof(UDS_MESSAGE_HEADER),
            _messageHeader.length, _messageHeader.ts);
        break;
    }
    buffer.Ignore(msgLength);
  }
  return true;
}

bool BaseUDSMessageProtocol::EnqueueForOutbound() {
  return GetFarProtocol()->EnqueueForOutbound();
}

bool BaseUDSMessageProtocol::SendDataToProtocol(uint8_t *pBuf, uint32_t length) {
  NYIR;
}

bool BaseUDSMessageProtocol::SendData(UDS_MESSAGE_TYPE type, const uint8_t *pBuf,
    const uint32_t length, double ts)
{
  memset(&_messageHeader, 0, sizeof(UDS_MESSAGE_HEADER));
  _messageHeader.type = type;
  _messageHeader.length = EHTONL(length);
  EHTOND(ts, _messageHeader.ts);
  // header
  _outputBuffer.ReadFromBuffer((uint8_t*)&_messageHeader, sizeof(UDS_MESSAGE_HEADER));
  // data
  _outputBuffer.ReadFromBuffer(pBuf, length);
  return EnqueueForOutbound();
}

bool BaseUDSMessageProtocol::SendMessage(Variant &message)
{
  string smsg;
  double ts = 0;
  message.SerializeToBin(smsg);
  message.Reset();
  //INFO("send message %s", STR(message.ToString()));
  return SendData(UDS_MESSAGE, (uint8_t*)smsg.c_str(), smsg.length(), ts);
}

void BaseUDSMessageProtocol::RegisterObserver(IUDSMessageObserver *pObserver)
{
  INFO("register observer");
  _pObserver = pObserver;
}

void BaseUDSMessageProtocol::RemoveObserver()
{
  _pObserver = NULL;
}

