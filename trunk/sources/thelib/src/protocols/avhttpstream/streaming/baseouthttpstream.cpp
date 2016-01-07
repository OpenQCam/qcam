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

#ifdef HAS_PROTOCOL_AVHTTPSTREAM
#include "protocols/avhttpstream/streaming/baseouthttpstream.h"

BaseOutHTTPStream::BaseOutHTTPStream(BaseProtocol *pProtocol,
    StreamsManager *pStreamsManager, uint64_t type, string name)
: BaseOutStream(pProtocol, pStreamsManager, type, name)
{
}

BaseOutHTTPStream::~BaseOutHTTPStream() {
}

bool BaseOutHTTPStream::SignalPlay(double &absoluteTimestamp, double &length) {
  NYIR;
}

bool BaseOutHTTPStream::SignalPause() {
  NYIR;
}

bool BaseOutHTTPStream::SignalResume() {
  NYIR;
}

bool BaseOutHTTPStream::SignalSeek(double &absoluteTimestamp) {
  NYIR;
}

bool BaseOutHTTPStream::SignalStop() {
  NYIR;
}


#endif
