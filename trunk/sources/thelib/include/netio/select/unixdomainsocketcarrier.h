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
#ifndef _UNIXDOMAINSOCKETCARRIER_H
#define _UNIXDOMAINSOCKETCARRIER_H

#include "netio/select/iohandler.h"

class BaseProtocol;

class DLLEXP UnixDomainSocketCarrier
: public IOHandler {
  private:
    bool _writeDataEnabled;
    bool _enableWriteDataCalled;
    int32_t _sendBufferSize;
    int32_t _recvBufferSize;
    uint64_t _rx;
    uint64_t _tx;
    int32_t _ioAmount;
    string _socketName;
  public:
    UnixDomainSocketCarrier(int32_t fd, string socketName);
    virtual ~UnixDomainSocketCarrier();
    virtual bool OnEvent(select_event &event);
    virtual bool SignalOutputData();
    virtual operator string();
    virtual void GetStats(Variant &info, uint32_t namespaceId = 0);
    // return socket name w/o extension
    string GetSocketName();
  private:
};


#endif  /* _UNIXDOMAINSOCKETCARRIER_H */
#endif /* NET_SELECT */


