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


#ifndef _UNIXDOMAINSOCKETPROTOCOL_H
#define _UNIXDOMAINSOCKETPROTOCOL_H

#include "protocols/baseprotocol.h"

class IOHandler;

class DLLEXP UnixDomainSocketProtocol
: public BaseProtocol {
private:
  IOHandler *_pCarrier;
  IOBuffer _inputBuffer;
  uint64_t _decodedBytesCount;

public:
  UnixDomainSocketProtocol();
  virtual ~UnixDomainSocketProtocol();
  virtual bool Initialize(Variant &parameters);
  //virtual void SetApplication(BaseClientApplication *pApplication);
  virtual IOHandler *GetIOHandler();
  virtual void SetIOHandler(IOHandler *pIOHandler);
  virtual bool AllowFarProtocol(uint64_t type);
  virtual bool AllowNearProtocol(uint64_t type);
  virtual IOBuffer *GetInputBuffer();
//  virtual IOBuffer *GetOutputBuffer();
  virtual bool SignalInputData(int32_t recvAmount);
  virtual bool SignalInputData(IOBuffer &buffer);
  virtual bool EnqueueForOutbound();
  virtual uint64_t GetDecodedBytesCount();

  // Handler to protocol interface
  //virtual bool SendDataToProtocol(uint8_t *pBuf, uint32_t length);

  // Stream
  //void RegisterInStream(BaseInStream *pInStream);
  //void RegisterOutStream(BaseOutStream *pOutStream);

  // Get unix socket name
  string GetSocketName();
};


#endif  /* _UNIXDOMAINSOCKETPROTOCOL_H */
