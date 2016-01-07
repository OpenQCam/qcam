
#ifdef HAS_PROTOCOL_HTTP
#include "streaming/streamstypes.h"
#include "streaming/wsinnetaudiostream.h"
#include "protocols/http/inboundhttpprotocol.h"

WSInNetAudioStream::WSInNetAudioStream (BaseProtocol *pProtocol,
  StreamsManager *pStreamsManager,
  string name)
: BaseInNetStream (pProtocol, pStreamsManager, ST_IN_WSNET_AUDIO, name) {
}

WSInNetAudioStream::~WSInNetAudioStream() {
  INFO("Delete WSInNetAudioStream\n");
}

bool WSInNetAudioStream::SignalPlay (double &absTimeStamp, double &length)
{
	return true;
}

bool WSInNetAudioStream::SignalPause()
{
	return true;
}

bool WSInNetAudioStream::SignalResume()
{
	return true;
}

bool WSInNetAudioStream::SignalSeek(double &absoluteTimestamp)
{
	return true;
}

bool WSInNetAudioStream::SignalStop()
{
  return true;
}

bool WSInNetAudioStream::IsCompatibleWithType(uint64_t type)
{
  return true;
}

void WSInNetAudioStream::SignalStreamCompleted()
{
}

void WSInNetAudioStream::ReadyForSend()
{
}

StreamCapabilities* WSInNetAudioStream::GetCapabilities()
{
  return NULL;
}

void WSInNetAudioStream::SignalOutStreamAttached(BaseOutStream *pOutStream)
{
}


void WSInNetAudioStream::SignalOutStreamDetached(BaseOutStream *pOutStream)
{
}

bool WSInNetAudioStream::FeedData(uint8_t *pData, uint32_t dataLength,
			uint32_t processedLength, uint32_t totalLength,
			double absoluteTimestamp, bool isAudio)
{
  return true;
}


#endif
