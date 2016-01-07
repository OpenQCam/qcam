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
#ifndef _UNIXDOMAINSOCKETACCEPTOR_H
#define _UNIXDOMAINSOCKETACCEPTOR_H


#include "netio/select/iohandler.h"
#include "protocols/baseprotocol.h"

class BaseClientApplication;

class DLLEXP UnixDomainSocketAcceptor : public IOHandler {
  private:
    struct sockaddr_un _address;
    vector<uint64_t> _protocolChain;
    BaseClientApplication *_pApplication;
    Variant _parameters;
    bool _enabled;
    uint32_t _acceptedCount;
    uint32_t _droppedCount;
    string _sockPath;
  public:
    UnixDomainSocketAcceptor(string sockPath, Variant parameters,
        vector<uint64_t>/*&*/ protocolChain);
    virtual ~UnixDomainSocketAcceptor();
    bool Bind();
    void SetApplication(BaseClientApplication *pApplication);
    bool StartAccept();
    virtual bool SignalOutputData();
    virtual bool OnEvent(select_event &event);
    virtual bool OnConnectionAvailable(select_event &event);
    bool Accept();
    bool Drop();
    Variant & GetParameters();
    BaseClientApplication *GetApplication();
    vector<uint64_t> &GetProtocolChain();
    virtual operator string();
    virtual void GetStats(Variant &info, uint32_t namespaceId = 0);
    bool Enable();
    void Enable(bool enabled);
  private:
    bool IsAlive();
};


#endif  /* _UNIXDOMAINSOCKETACCEPTOR_H */
#endif /* NET_SELECT */


