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


#include "netio/select/unixdomainsocketclient.h"

UnixDomainSocketClient::UnixDomainSocketClient(string sockPath)
  : BaseUDSMessageProtocol(PT_UNIX_DOMAIN_SOCKET)
{
  _socketPath = sockPath;
  _socketFd = 0;
  _recvBufferSize = 40960;
  _sendBufferSize = 1024;
  _ioAmount = 0;
  _enableWriteData = false;
  memset(&tv, 0, sizeof(struct timeval));
  memset(&_socket, 0, sizeof(struct sockaddr_un));
  _socket.sun_family = AF_UNIX;
  strcpy(_socket.sun_path, STR(_socketPath));
}

UnixDomainSocketClient::~UnixDomainSocketClient() {
  _pObserver=NULL;
}


bool UnixDomainSocketClient::Connect() {
  // 1. create socket
  _socketFd = socket(PF_UNIX, SOCK_STREAM, 0);
  if(_socketFd < 0) {
    int err = LASTSOCKETERROR;
    DEBUG("Unable to create unix domain socket %s: %s(%d)\n",
        STR(_socketPath), strerror(err), err);
    return false;
  }

  // 2. connect socket
  if(connect(_socketFd, (struct sockaddr *) &_socket, sizeof(struct sockaddr_un)) != 0 ){
    int err = LASTSOCKETERROR;
    DEBUG("Unable to connect unix domain socket %s: %s(%d)\n",
        STR(_socketPath), strerror(err), err);
    return false;
  }

  return true;

}

bool UnixDomainSocketClient::Run(){

  FD_ZERO(&readFds);
  FD_ZERO(&writeFds);
  FD_SET(_socketFd, &readFds);

  if(_enableWriteData)
    FD_SET(_socketFd, &writeFds);
  else
    FD_CLR(_socketFd, &writeFds);

  tv.tv_sec = 1;
  tv.tv_usec = 0;

  int32_t count = select(_socketFd+1, &readFds, &writeFds, NULL, &tv);
  if(count < 0){
    int err = LASTSOCKETERROR;
    if(err == EINTR) return true;
    DEBUG("Unable to select: %u\n", (uint32_t) LASTSOCKETERROR);
    return false;
  }

  if(count == 0){
    return true;
  }

  if(FD_ISSET(_socketFd, &readFds)){
    if(!_inputBuffer.ReadFromUxSocketFd(_socketFd, _recvBufferSize, _ioAmount)){
        DEBUG("Unable to read data from unix domain socket\n");
        return false;
    }
    //baseudsmessageprotocol
    SignalInputData(_inputBuffer);
  }

  if(FD_ISSET(_socketFd, &writeFds)){
    while(GETAVAILABLEBYTESCOUNT(_outputBuffer) > 0){
      if(!_outputBuffer.WriteToUxSocketFd(_socketFd, GETAVAILABLEBYTESCOUNT(_outputBuffer), _ioAmount)){
          DEBUG("Unable to send data to unix domain socket\n");
          return false;
      }
    }
    _enableWriteData = false;
  }
  return true;

}

bool UnixDomainSocketClient::EnqueueForOutbound()
{
  _enableWriteData = true;
  return true;
}

void UnixDomainSocketClient::Close() {
  if(_socketFd)
    close(_socketFd);
}
