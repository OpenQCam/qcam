/*
 * Copyright (c) 2013, Oculus Technologies Inc. All rights reserved.
 * Three Ocean (to@bcloud.us)
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

#ifndef _UDSMESSAGE_H
#define _UDSMESSAGE_H

#define UDS_HEADER_LEN    13
#define UDS_TYPE_LEN      1
#define UDS_DATA_LEN      4
#define UDS_TIMESTAMP_LEN 8

enum UDS_MESSAGE_TYPE {
  UDS_MESSAGE = 0,
  UDS_AUDIO,
  UDS_VIDEO
};

// message protocol
/*
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +---------------+---------------+---------------+---------------+
 * |   msg type    |   reserved    |             Length            |
 * +---------------+---------------+---------------+---------------+
 * |            Length             |           TimeStamp           |
 * +---------------+---------------+---------------+---------------+
 * |                            TimeStamp                          |
 * +---------------+---------------+---------------+---------------+
 * |           TimeStamp           |                               |
 * +---------------+---------------+---------------+---------------+
 *
 */
#pragma pack(push, 1)
struct UDS_MESSAGE_HEADER {
  UDS_MESSAGE_TYPE type     : 8;
  uint32_t length           : 32;
  uint64_t ts               : 64;
};
#pragma pack(pop)

// Unix Domain Socket Message Interface
class BaseUDSMessageProtocol;
class IUDSMessageObserver {
  public:
    virtual bool OnMessage(BaseUDSMessageProtocol*pProtocol, Variant &message) = 0;
    virtual bool OnData(UDS_MESSAGE_TYPE type, uint8_t *pData, uint32_t length, uint64_t ts) = 0;
};



#endif /* _UDSMESSAGE_H */

