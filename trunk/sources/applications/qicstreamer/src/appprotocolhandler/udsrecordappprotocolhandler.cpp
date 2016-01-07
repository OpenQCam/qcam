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

#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET


#include "appprotocolhandler/udsrecordappprotocolhandler.h"
#include "uxthread/mp4recordstream.h"
#include "uxthread/udsrecordthread.h"
#include "system/thread/threadmanager.h"
#include "protocols/unixdomainsocket/unixdomainsocketprotocol.h"
#include "protocols/unixdomainsocket/udsrecordprotocol.h"
#include "application/baseclientapplication.h"
#include "utils/misc/crypto.h"


using namespace app_qicstreamer;

UDSRecordAppProtocolHandler::UDSRecordAppProtocolHandler(Variant &configuration)
  : BaseUXDomainSocketAppProtocolHandler(configuration)
{
}

UDSRecordAppProtocolHandler::~UDSRecordAppProtocolHandler()
{
}

bool UDSRecordAppProtocolHandler::ProcessMessage(BaseProtocol *pProtocol, IOBuffer &buffer)
{
  return true;
}

bool UDSRecordAppProtocolHandler::OnMessage(BaseUDSMessageProtocol *pProtocol, Variant &message)
{
  if(!message.HasKey("type") && !message.HasKey("id")) {
    FATAL("Unable to get message type and id");
    return false;
  }
  if(((uint8_t)message["type"]) == UDSRecordThread::MM_THREAD_CREATED) {
    uint32_t threadId = (uint32_t) message["id"];
    UDSRecordThread *pThread = (UDSRecordThread*)ThreadManager::FindThreadById(threadId);
    if(pThread == NULL) {
      FATAL("Unable to find thread by id(%d)", (uint32_t)message["id"]);
      return false;
    }
    // make main thread functions of thread can be reached by uds protocol
    pThread->SetUDSMessageProtocol(pProtocol);
    INFO("create stream and register observer");
    // make thread associate with stream
    MP4RecordStream *pStream = new MP4RecordStream(pProtocol,
        GetApplication()->GetStreamsManager(),
        md5(format("%d", pProtocol->GetId()), true), threadId);
    pThread->RegisterThreadObserver(pStream);
    pThread->SetRecordStream(pStream);

    // Note: we need to enqueue thread here in order to make thread available
    ThreadManager::EnqueueThread(pThread->GetType(), pThread);

  } else {
    FATAL("Unable to deal with this message type(%d)", (uint8_t)message["type"]);
  }
  return true;
}

bool UDSRecordAppProtocolHandler::OnData(UDS_MESSAGE_TYPE type, uint8_t *pData, uint32_t length, uint64_t ts)
{
  NYIR;
}

#endif
