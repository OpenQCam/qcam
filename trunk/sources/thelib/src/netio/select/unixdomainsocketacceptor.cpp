/* ==================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 * (C) COPYRIGHT Quanta Computer Inc. ALL RIGHTS RESERVED
 *
 * ==================================================================
 * ------------------------------------------------------------------
 *  Date    :
 *  Version :
 *  Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 *  Purpose :
 *  =================================================================
 */

#ifdef NET_SELECT
//thelib
#include "netio/select/unixdomainsocketacceptor.h"
#include "netio/select/iohandlermanager.h"
#include "protocols/protocolfactorymanager.h"
#include "protocols/unixdomainsocket/unixdomainsocketprotocol.h"
#include "netio/select/unixdomainsocketcarrier.h"
#include "application/baseclientapplication.h"

UnixDomainSocketAcceptor::UnixDomainSocketAcceptor(string sockPath, Variant parameters,
    vector<uint64_t>/*&*/ protocolChain)
: IOHandler(0, 0, IOHT_UX_ACCEPTOR) {
  _pApplication = NULL;
  memset(&_address, 0, sizeof (struct sockaddr_un));
  _address.sun_family = AF_UNIX;
  strcpy(_address.sun_path, STR(sockPath));
  _protocolChain = protocolChain;
  _parameters = parameters;
  _enabled = false;
  _acceptedCount = 0;
  _droppedCount = 0;
  _sockPath = sockPath;
}

UnixDomainSocketAcceptor::~UnixDomainSocketAcceptor() {
  CLOSE_SOCKET(_inboundFd);
}

bool UnixDomainSocketAcceptor::Bind() {
  _inboundFd = _outboundFd = (int) socket(PF_UNIX, SOCK_STREAM, 0);
  if (_inboundFd < 0) {
    int err = LASTSOCKETERROR;
    FATAL("Unable to create unix socket: %s(%d)", strerror(err), err);
    return false;
  }

  //FIXME(Tim): is non-blocking necessary?
  //if (!setFdOptions(_inboundFd, false)) {
  //  FATAL("Unable to set unix socket options");
  //  return false;
  //}

  if(fileExists(_sockPath)){
    deleteFile(_sockPath);
  }

  if (::bind(_inboundFd, (sockaddr *) & _address, sizeof (struct sockaddr_un)) != 0) {
    int error = LASTSOCKETERROR;
    FATAL("Unable to bind %s Error was: %s (%d)",
        STR(_sockPath),
        strerror(error),
        error);
    return false;
  }

  if (listen(_inboundFd, 5) != 0) {
    FATAL("Unable to put the socket in listening mode");
    return false;
  }

  _enabled = true;
  return true;
}

void UnixDomainSocketAcceptor::SetApplication(BaseClientApplication *pApplication) {
  o_assert(_pApplication == NULL);
  _pApplication = pApplication;
}

bool UnixDomainSocketAcceptor::StartAccept() {
  return IOHandlerManager::EnableAcceptConnections(this);
}

bool UnixDomainSocketAcceptor::SignalOutputData() {
  ASSERT("Operation not supported");
  return false;
}

bool UnixDomainSocketAcceptor::OnEvent(select_event &event) {
  if (!OnConnectionAvailable(event))
    return IsAlive();
  else
    return true;
}

bool UnixDomainSocketAcceptor::OnConnectionAvailable(select_event &event) {
  if (_pApplication == NULL)
    return Accept();
  //return _pApplication->AcceptTCPConnection(this);
  return _pApplication->AcceptUnixDomainSocketConnection(this);
}

bool UnixDomainSocketAcceptor::Accept() {
  struct sockaddr_un remoteAddress;
  memset(&remoteAddress, 0, sizeof (struct sockaddr_un));
  socklen_t len = sizeof (struct sockaddr_un);
  int32_t fd;
  int32_t error;

  //1. Accept the connection
  fd = accept(_inboundFd, (sockaddr *)&remoteAddress, &len);
  error = LASTSOCKETERROR;
  if (fd < 0) {
    FATAL("Unable to accept UX client connection: %s (%d)", strerror(error), error);
    return false;
  }
  if (!_enabled) {
    CLOSE_SOCKET(fd);
    _droppedCount++;
    WARN("Acceptor is not enabled. UX Client dropped: %s", STR(_sockPath));
    return true;
  }
  INFO("Client connected: %s", STR(_sockPath));

  //if (!setFdOptions(fd, false)) {
  //  FATAL("Unable to set unix socket options");
  //  CLOSE_SOCKET(fd);
  //  return false;
  //}

  //2. Create the chain
  BaseProtocol *pProtocol = ProtocolFactoryManager::CreateProtocolChain(
      _protocolChain, _parameters);
  if (pProtocol == NULL) {
    FATAL("Unable to create protocol chain");
    CLOSE_SOCKET(fd);
    return false;
  }

  //3. Create the carrier and bind it
  UnixDomainSocketCarrier *pUnixDomainSocketCarrier = new UnixDomainSocketCarrier(fd, _sockPath);
  pUnixDomainSocketCarrier->SetProtocol(pProtocol->GetFarEndpoint());
  pProtocol->GetFarEndpoint()->SetIOHandler(pUnixDomainSocketCarrier);

  //4. Register protocol for thread access
  UnixDomainSocketManager::RegisterUXThreadProtocol(pUnixDomainSocketCarrier->GetSocketName(),
      (UnixDomainSocketProtocol *)pUnixDomainSocketCarrier->GetProtocol());

  //5. Register the protocol stack with an application
  if (_pApplication != NULL) {
    pProtocol = pProtocol->GetNearEndpoint();
    pProtocol->SetApplication(_pApplication);
  }

  if (pProtocol->GetNearEndpoint()->GetOutputBuffer() != NULL)
    pProtocol->GetNearEndpoint()->EnqueueForOutbound();

  _acceptedCount++;

  return true;
}

bool UnixDomainSocketAcceptor::Drop() {
  struct sockaddr_un remoteAddress;
  memset(&remoteAddress, 0, sizeof (struct sockaddr_un));
  socklen_t len = sizeof (sockaddr);


  //1. Accept the connection
  int32_t fd = accept(_inboundFd, (sockaddr *) &remoteAddress, &len);
  if (fd < 0) {
    uint32_t err = LASTSOCKETERROR;
    WARN("Accept failed. Error code was: %"PRIu32, err);
    return true;
  }

  //2. Drop it now
  CLOSE_SOCKET(fd);
  _droppedCount++;

  INFO("Client explicitly dropped: %s", STR(_sockPath));
  return true;
}

Variant & UnixDomainSocketAcceptor::GetParameters() {
  return _parameters;
}

BaseClientApplication *UnixDomainSocketAcceptor::GetApplication() {
  return _pApplication;
}

vector<uint64_t> &UnixDomainSocketAcceptor::GetProtocolChain() {
  return _protocolChain;
}

UnixDomainSocketAcceptor::operator string() {
  return format("A(%d)", _inboundFd);
}

void UnixDomainSocketAcceptor::GetStats(Variant &info, uint32_t namespaceId) {
  info = _parameters;
  info["id"] = (((uint64_t) namespaceId) << 32) | GetId();
  info["enabled"] = (bool)_enabled;
  info["acceptedConnectionsCount"] = _acceptedCount;
  info["droppedConnectionsCount"] = _droppedCount;
  if (_pApplication != NULL) {
    info["appId"] = (((uint64_t) namespaceId) << 32) | _pApplication->GetId();
    info["appName"] = _pApplication->GetName();
  } else {
    info["appId"] = (((uint64_t) namespaceId) << 32);
    info["appName"] = "";
  }
}

bool UnixDomainSocketAcceptor::Enable() {
  return _enabled;
}

void UnixDomainSocketAcceptor::Enable(bool enabled) {
  _enabled = enabled;
}

bool UnixDomainSocketAcceptor::IsAlive() {
  //TODO: Implement this. It must return true
  //if this acceptor is operational
  NYI;
  return true;
}

#endif /* NET_SELECT */

