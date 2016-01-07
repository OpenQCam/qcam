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


#include "appprotocolhandler/udscommandappprotocolhandler.h"
#include "uxthread/udscommandthread.h"
#include "system/thread/threadmanager.h"
#include "protocols/unixdomainsocket/unixdomainsocketprotocol.h"
#include "protocols/unixdomainsocket/udscommandprotocol.h"
#include "application/baseclientapplication.h"


using namespace app_qicstreamer;

UDSCommandAppProtocolHandler::UDSCommandAppProtocolHandler(Variant &configuration)
  : BaseUXDomainSocketAppProtocolHandler(configuration)
{
}

UDSCommandAppProtocolHandler::~UDSCommandAppProtocolHandler()
{
}

bool UDSCommandAppProtocolHandler::ProcessMessage(BaseProtocol *pProtocol, IOBuffer &buffer)
{
  NYIR;
}

bool UDSCommandAppProtocolHandler::OnMessage(BaseUDSMessageProtocol *pProtocol, Variant &message)
{
  if(!message.HasKey("type") && !message.HasKey("id")) {
    FATAL("Unable to get message type or thread id\n%s", STR(message.ToString()));
    return false;
  }
  if(((uint8_t)message["type"]) == UDSCommandThread::MM_THREAD_CREATED) {
    uint32_t threadId = (uint32_t) message["id"];
    UDSCommandThread *pThread = (UDSCommandThread*)ThreadManager::FindThreadById(threadId);
    if(pThread == NULL) {
      FATAL("Unable to find thread by id(%d)", threadId);
      return false;
    }
    pThread->SetUDSMessageProtocol(pProtocol);
    ThreadManager::EnqueueThread(pThread->GetType(), pThread);
    //INFO("set protocol");
    //pThread->SetUDSMessageProtocol(pProtocol);
  }else{
    FATAL("Unable to handle this message type");
    return false;
  }
  return true;
}

bool UDSCommandAppProtocolHandler::OnData(UDS_MESSAGE_TYPE type, uint8_t *pData, uint32_t length, uint64_t ts)
{
  NYIR;
}
#endif
