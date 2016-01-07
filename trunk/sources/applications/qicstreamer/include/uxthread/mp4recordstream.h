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

#ifndef _MP4RECORDSTREAM_H
#define _MP4RECORDSTREAM_H

#include "streaming/baseoutstream.h"
#include "protocols/unixdomainsocket/udsmessage.h"
#include "netio/select/iotimer.h"

class UDSRecordProtocol;

namespace app_qicstreamer {

class MP4RecordStream
: public BaseOutStream
, public IUDSMessageObserver
, public IOTimer
{
  private:
    uint64_t _startRecordingTS;
    uint64_t _currentTS;
    uint32_t _threadId;
    UDSRecordProtocol *_pUDSProtocol;
  public:
    MP4RecordStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager, string name, uint32_t threadId);
    virtual ~MP4RecordStream();

    //baseoutstream
    virtual void SignalAttachedToInStream();
    virtual void SignalDetachedFromInStream();
    virtual void SignalStreamCompleted();
    virtual bool IsCompatibleWithType(uint64_t type);
    virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
        uint32_t processedLength, uint32_t totalLength,
        double absoluteTimestamp, bool isAudio);

    virtual bool SignalPlay(double &absoluteTimestamp, double &length);
    virtual bool SignalPause();
    virtual bool SignalResume();
    virtual bool SignalSeek(double &absoluteTimestamp);
    virtual bool SignalStop();
    // observer interface
    virtual bool OnMessage(BaseUDSMessageProtocol *pProtocol, Variant &message);
    virtual bool OnData(UDS_MESSAGE_TYPE type, uint8_t *pData, uint32_t length, uint64_t ts);
    // iotimer
    virtual bool OnEvent(select_event &event);
};

}
#endif /* _MP4RECORDSTREAM_H */

#endif

