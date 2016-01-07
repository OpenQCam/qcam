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


#ifndef _BASEUDSMESSAGEPROTOCOL_H
#define _BASEUDSMESSAGEPROTOCOL_H

#include "protocols/unixdomainsocket/unixdomainsocketprotocol.h"
#include "protocols/unixdomainsocket/udsmessage.h"

class BaseClientApplication;
class BaseUXDomainSocketAppProtocolHandler;

class DLLEXP BaseUDSMessageProtocol
: public BaseProtocol
{
  private:
    UDS_MESSAGE_HEADER _messageHeader;
    BaseUXDomainSocketAppProtocolHandler *_pProtocolHandler;
  protected:
    IOBuffer _outputBuffer;
    IUDSMessageObserver *_pObserver;

  public:
    BaseUDSMessageProtocol(uint64_t type);
    virtual ~BaseUDSMessageProtocol();
    virtual bool Initialize(Variant &parameters);
    virtual void SetApplication(BaseClientApplication *pApplication);
    virtual bool AllowFarProtocol(uint64_t type);
    virtual bool AllowNearProtocol(uint64_t type);
    virtual IOBuffer *GetOutputBuffer();
    virtual bool SignalInputData(int32_t recvAmount);
    virtual bool SignalInputData(IOBuffer &buffer);
    virtual bool SendDataToProtocol(uint8_t *pBuf, uint32_t length);
    virtual bool EnqueueForOutbound();

    bool SendData(UDS_MESSAGE_TYPE type, const uint8_t *pBuf, const uint32_t length, double ts);
    bool SendMessage(Variant &message);

    void RegisterObserver(IUDSMessageObserver *pObserver);
    void RemoveObserver();

};


#endif  /* _BASEUDSMESSAGEPROTOCOL_H */
