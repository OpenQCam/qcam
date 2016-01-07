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

#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
#include "protocols/unixdomainsocket/streaming/baserecordstream.h"

BaseRecordStream::BaseRecordStream(BaseProtocol *pProtocol,
    StreamsManager *pStreamsManager, uint64_t type, string name)
: BaseOutStream(pProtocol, pStreamsManager, type, name)
{
}

BaseRecordStream::~BaseRecordStream() {
}

bool BaseRecordStream::SignalPlay(double &absoluteTimestamp, double &length) {
  NYIR;
}

bool BaseRecordStream::SignalPause() {
  NYIR;
}

bool BaseRecordStream::SignalResume() {
  NYIR;
}

bool BaseRecordStream::SignalSeek(double &absoluteTimestamp) {
  NYIR;
}

bool BaseRecordStream::SignalStop() {
  NYIR;
}


#endif
