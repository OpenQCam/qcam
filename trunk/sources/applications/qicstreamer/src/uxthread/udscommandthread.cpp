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


#include "uxthread/udscommandthread.h"

#define SOCKET_NAME "/tmp/sock.command"

using namespace app_qicstreamer;

UDSCommandThread::UDSCommandThread()
  : BaseUDSThread(THREAD_T_COMMAND, SOCKET_NAME)
{
}

UDSCommandThread::~UDSCommandThread()
{
}
void UDSCommandThread::OnConnected()
{
  _returnMessage.Reset();
  _returnMessage["type"] = UDSCommandThread::MM_THREAD_CREATED;
  _returnMessage["id"] = GetId();
  SendMessageToMainThread(_returnMessage);
}

bool UDSCommandThread::OnMessage(BaseUDSMessageProtocol *pProtocol, Variant &message)
{
  if(!message.HasKey("type")) {
    WARN("unknown command message type");
    return true;
  }
  _returnMessage.Reset();
  switch((uint32_t)message["type"]) {
    case CM_COMMAND:
      if(message.HasKey("cmd")) {
        string outputResult;
        DoSystemCommand(message["cmd"], outputResult);
        _returnMessage["result"] = outputResult;
        SendMessageToMainThread(_returnMessage);
      }else{
        WARN("No command string");
      }
      break;
    default:
      WARN("unknown command message type");
      break;
  }
  //INFO("%s", STR(message.ToString()));
  return true;
}

bool UDSCommandThread::OnData(UDS_MESSAGE_TYPE type, uint8_t *pData, uint32_t length, uint64_t ts)
{
  NYIR;
}

void UDSCommandThread::ExecuteCommand(string &cmd, IUDSMessageObserver *pObserver)
{
  _commandMessage.Reset();
  _commandMessage["type"] = UDSCommandThread::CM_COMMAND;
  _commandMessage["cmd"] = cmd;
  RegisterThreadObserver(pObserver);
  SendMessageToThread(_commandMessage);
}

bool UDSCommandThread::DoSystemCommand(string cmd, string& retMsg) {
  int32_t ret=0;
  retMsg.clear();

  INFO("execute command %s", STR(cmd));
  FILE *fp = popen(cmd.c_str(), "r");
  if(fp){
    char buf[1024];
    while(fgets(buf, sizeof(buf)-1, fp) != NULL){
      retMsg.append(buf);
    }
    ret=pclose(fp);
  } else {
    FATAL("Unable to execute command : %s", STR(cmd));
    return false;
  }
  if (ret!=0)
    return false;
 return true;
}

