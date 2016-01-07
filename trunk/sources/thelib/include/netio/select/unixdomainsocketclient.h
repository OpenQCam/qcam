/* ============================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 *            (C) COPYRIGHT Quanta Computer Inc.
 *                   ALL RIGHTS RESERVED
 *
 * ============================================================================
 * ----------------------------------------------------------------------------
 * Date    :
 * Version :
 * Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 * ----------------------------------------------------------------------------
 * Purpose :
 * ============================================================================
 */

#ifndef _UNIXDOMAINSOCKETCLIENT_H
#define _UNIXDOMAINSOCKETCLIENT_H

#include "common.h"
#include "protocols/unixdomainsocket/baseudsmessageprotocol.h"

class UnixDomainSocketClient
: public BaseUDSMessageProtocol
{
  private:
    string _socketPath;
    struct sockaddr_un _socket;
    int _socketFd;
    struct timeval tv;
    fd_set readFds;
    fd_set writeFds;
    IOBuffer _inputBuffer;
    int32_t _recvBufferSize;
    int32_t _sendBufferSize;
    int32_t _ioAmount;
    bool _enableWriteData;
    //UDS_MESSAGE_HEADER _messageHeader;
    //IUDSMessageObserver *_pObserver;

  public:
    UnixDomainSocketClient(string sockPath);
    virtual ~UnixDomainSocketClient();
    bool Connect();
    //void Send(const uint8_t *buf, const uint32_t length);
    void Close();
    bool Run();
    // BaseProtocol
    virtual bool EnqueueForOutbound();
};

#endif /* _UNIXDOMAINSOCKETCLIENT_H */
