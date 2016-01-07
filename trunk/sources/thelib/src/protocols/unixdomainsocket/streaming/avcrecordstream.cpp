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

#include "protocols/unixdomainsocket/streaming/avcrecordstream.h"
#include "protocols/unixdomainsocket/unixdomainsocketprotocol.h"


AVCRecordStream::AVCRecordStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager,
  string name)
: BaseRecordStream (pProtocol, pStreamsManager, ST_OUT_RECORD_AVC, name),
  _pUXProtocol(0)
{
  _pUXProtocol = reinterpret_cast<UnixDomainSocketProtocol *>(_pProtocol);
}


AVCRecordStream:: ~AVCRecordStream() {
  _pUXProtocol = NULL;
}

void AVCRecordStream::SignalAttachedToInStream() {
}
void AVCRecordStream::SignalDetachedFromInStream() {
}

void AVCRecordStream::SignalStreamCompleted() {
}

bool AVCRecordStream::IsCompatibleWithType(uint64_t type) {
  return true;
}

bool AVCRecordStream::FeedData(uint8_t *pData, uint32_t dataLength, uint32_t processedLength,
      uint32_t totalLength, double absoluteTimestamp, bool isAudio) {

  return _pUXProtocol->SendDataToProtocol(pData, dataLength);

}

#endif
