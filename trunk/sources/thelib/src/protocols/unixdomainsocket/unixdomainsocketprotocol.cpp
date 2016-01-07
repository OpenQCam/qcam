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


#include "protocols/unixdomainsocket/unixdomainsocketprotocol.h"
#include "protocols/unixdomainsocket/baseuxdomainsocketappprotocolhandler.h"
#include "application/baseclientapplication.h"
#include "netio/netio.h"
#include "netio/select/unixdomainsocketcarrier.h"
#include "streaming/baseinstream.h"
#include "streaming/baseoutstream.h"

UnixDomainSocketProtocol::UnixDomainSocketProtocol()
: BaseProtocol(PT_UNIX_DOMAIN_SOCKET),
  _pCarrier(0)
  //_pInStream(0),
  //_pOutStream(0),
  //_pProtocolHandler(0)
{
  _decodedBytesCount = 0;
}

UnixDomainSocketProtocol::~UnixDomainSocketProtocol() {
  if (_pCarrier != NULL) {
    IOHandler *pCarrier = _pCarrier;
    _pCarrier = NULL;
    pCarrier->SetProtocol(NULL);
    delete pCarrier;
  }

  //if(_pInStream != NULL){
  //  delete _pInStream;
  //}
  //if(_pOutStream != NULL){
  //  delete _pOutStream;
  //}
}

bool UnixDomainSocketProtocol::Initialize(Variant &parameters) {
  return true;
}

//void UnixDomainSocketProtocol::SetApplication(BaseClientApplication *pApplication) {
//  BaseProtocol::SetApplication(pApplication);
//  if (pApplication != NULL) {
//    _pProtocolHandler = (BaseUXDomainSocketAppProtocolHandler *)
//        pApplication->GetProtocolHandler(this);
//  } else {
//    _pProtocolHandler = NULL;
//  }
//}


IOHandler *UnixDomainSocketProtocol::GetIOHandler() {
  return _pCarrier;
}



void UnixDomainSocketProtocol::SetIOHandler(IOHandler *pIOHandler) {
  if (pIOHandler != NULL) {
    if ((pIOHandler->GetType() != IOHT_UX_CARRIER)
        && (pIOHandler->GetType() != IOHT_STDIO)) {
      ASSERT("This protocol accepts only TCP carriers");
    }
  }
  _pCarrier = pIOHandler;
}

bool UnixDomainSocketProtocol::AllowFarProtocol(uint64_t type) {
  WARN("This protocol doesn't accept any far protocol");
  return false;
}

bool UnixDomainSocketProtocol::AllowNearProtocol(uint64_t type) {
  return true;
}

IOBuffer * UnixDomainSocketProtocol::GetInputBuffer() {
  return &_inputBuffer;
}

//IOBuffer * UnixDomainSocketProtocol::GetOutputBuffer() {
//  if(GETAVAILABLEBYTESCOUNT(_outputBuffer) > 0){
//    return &_outputBuffer;
//  }else{
//    return NULL;
//  }
//}

bool UnixDomainSocketProtocol::SignalInputData(int32_t recvAmount) {
  _decodedBytesCount += recvAmount;
  //if(_pProtocolHandler != NULL){
  //  _pProtocolHandler->ProcessMessage(this, _inputBuffer);
  //}
  _pNearProtocol->SignalInputData(_inputBuffer);
  return true;
}

bool UnixDomainSocketProtocol::SignalInputData(IOBuffer & /* ignored */) {
  ASSERT("OPERATION NOT SUPPORTED");
  return false;
}

bool UnixDomainSocketProtocol::EnqueueForOutbound() {
  if (_pCarrier == NULL) {
    FATAL("UnixDomainSocketProtocol has no carrier");
    return false;
  }
  return _pCarrier->SignalOutputData();
}

uint64_t UnixDomainSocketProtocol::GetDecodedBytesCount() {
  return _decodedBytesCount;
}


//bool UnixDomainSocketProtocol::SendDataToProtocol(uint8_t *pBuf, uint32_t length) {
//  _outputBuffer.ReadFromBuffer(pBuf, length);
//  return EnqueueForOutbound();
//}

//void UnixDomainSocketProtocol::RegisterOutStream(BaseOutStream *pStream) {
//  //_pOutStream = pStream;
//}
//
//void UnixDomainSocketProtocol::RegisterInStream(BaseInStream *pStream) {
//  //_pInStream = pStream;
//}

string UnixDomainSocketProtocol::GetSocketName() {
  return reinterpret_cast<UnixDomainSocketCarrier *>(_pCarrier)->GetSocketName();
}


