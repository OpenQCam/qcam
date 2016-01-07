
#include "streaming/streamstypes.h"
#include "streaming/baseoutstream.h"
#include "streaming/inmsgstream.h"

InMSGStream::InMSGStream (StreamsManager *pStreamsManager, string streamName)
: BaseInStream (NULL, pStreamsManager, ST_IN_MSG_SYSTEM, streamName)
{
}

InMSGStream::~InMSGStream ()
{
}

bool InMSGStream::SignalPlay (double &absTimeStamp, double &length)
{
  return true;
}

bool InMSGStream::SignalPause()
{
	return true;
}

bool InMSGStream::SignalResume()
{
	return true;
}

bool InMSGStream::SignalSeek(double &absoluteTimestamp)
{
	return true;
}

bool InMSGStream::SignalStop()
{
  return true;
}

bool InMSGStream::IsCompatibleWithType(uint64_t type)
{
  return true;
}

void InMSGStream::SignalStreamCompleted()
{
}

void InMSGStream::ReadyForSend()
{
}

StreamCapabilities* InMSGStream::GetCapabilities()
{
  return NULL;
}

void InMSGStream::SignalOutStreamAttached(BaseOutStream *pOutStream)
{
  while (!_queuedMsg.empty()) {
    string msg=_queuedMsg.front();
    LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;

    while (pTemp != NULL) {
      bool ret=pTemp->info->FeedData((uint8_t*)msg.c_str(), msg.length(), 0, msg.length(), 0, false);
      if (!ret) return;
      pTemp = pTemp->pPrev;
    }
    _queuedMsg.pop();
  }
}


void InMSGStream::SignalOutStreamDetached(BaseOutStream *pOutStream)
{
}

#define MAX_MSGQSIZE 300
bool InMSGStream::FeedMSGData(uint8_t *pData, uint32_t dataLength, bool queued) {
  LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;

  if ( (!pTemp&&queued) && (_queuedMsg.size()<MAX_MSGQSIZE) ) {
    _queuedMsg.push(string((char*)pData, dataLength));
    return true;
  }

  while (pTemp != NULL) {
    if (!pTemp->info->FeedData(pData, dataLength, 0, dataLength, 0, false)) {
      FATAL ("Unable to feed OS: %p", pTemp->info);
      if (queued && _queuedMsg.size()<MAX_MSGQSIZE) {
        _queuedMsg.push(string((char*)pData, dataLength));
      }
    }
    pTemp = pTemp->pPrev;
  }

  return true;
}

bool InMSGStream::FeedData(uint8_t *pData, uint32_t dataLength,
			uint32_t processedLength, uint32_t totalLength,
			double absoluteTimestamp, bool isAudio)
{
  NYI;
  return false;
}
