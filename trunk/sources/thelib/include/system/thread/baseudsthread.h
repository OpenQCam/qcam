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


#ifndef _BASEUDSTHREAD_H
#define _BASEUDSTHREAD_H

#include "common.h"
#include "netio/select/unixdomainsocketclient.h"
#include "protocols/unixdomainsocket/baseudsmessageprotocol.h"
#include "system/thread/thread.h"

class BaseUDSThread
  : public IUDSMessageObserver
  , public Thread
{
  private:
    string _socketName;
    UnixDomainSocketClient *_pUDSClient;
    BaseUDSMessageProtocol *_pUDSMsgProtocol;
    Mutex _mutex;
  public:
    BaseUDSThread(THREAD_TYPE type, string socketName);
    virtual ~BaseUDSThread();
    bool SetUDSMessageProtocol(BaseUDSMessageProtocol *pProtocol);
    BaseUDSMessageProtocol *GetUDSMessageProtocol();
    bool RegisterThreadObserver(IUDSMessageObserver *pObserver);
    void RemoveThreadObserver();
    virtual bool Free();
    // thread
    virtual void *Run();
    virtual void OnConnected() = 0;
    // thread observer interface
    //virtual bool OnMessage(BaseUDSMessageProtocol *pProtocol, Variant &message);
    //virtual bool OnData(UDS_MESSAGE_TYPE type, uint8_t *pData, uint32_t length, uint64_t ts);
  protected:
    // main to thread functions
    bool SendMessageToThread(Variant &message);
    bool SendDataToThread(UDS_MESSAGE_TYPE type, uint8_t* pData, uint32_t length, uint64_t ts);
    // thread to main functions
    bool SendMessageToMainThread(Variant &message);
    bool SendDataToMainThread(UDS_MESSAGE_TYPE type, uint8_t* pData, uint32_t length, uint64_t ts);

};
#endif /* _BASEUDSTHREAD_H */

