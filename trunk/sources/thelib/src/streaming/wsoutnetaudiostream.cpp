
#ifdef HAS_PROTOCOL_HTTP
#include "audio/pcmstream.h"
#include "streaming/wsoutnetaudiostream.h"
#include "streaming/streamstypes.h"
#include "protocols/http/websocket/basewssubprotocol.h"
#include "application/baseclientapplication.h"
#include "system/eventdefine.h"

WSOutNetAudioStream::WSOutNetAudioStream (BaseProtocol *pProtocol,
  StreamsManager *pStreamsManager,
  string streamName)
: BaseOutNetStream(pProtocol, pStreamsManager, ST_OUT_WSNET_AUDIO, streamName),
  _dataThreshold(AUDIOTHRE)
{
  _soundBuffer.IgnoreAll();
}

WSOutNetAudioStream::~WSOutNetAudioStream() {
}

bool WSOutNetAudioStream::SignalPlay (double &absTimeStamp, double &length)
{
	return true;
}

bool WSOutNetAudioStream::SignalPause()
{
	return true;
}

bool WSOutNetAudioStream::SignalResume()
{
	return true;
}

bool WSOutNetAudioStream::SignalSeek(double &absoluteTimestamp)
{
	return true;
}

bool WSOutNetAudioStream::SignalStop()
{
  return true;
}

bool WSOutNetAudioStream::IsCompatibleWithType(uint64_t type)
{
  return true;
}

void WSOutNetAudioStream::SignalStreamCompleted()
{
}

void WSOutNetAudioStream::SignalAttachedToInStream() {
  if (_pInStream) {
    _dataThreshold=reinterpret_cast<BaseAudioStream*>(_pInStream)->GetAudioDataTh();
  }

  uint16_t event=EVT_OUT_AREC_CONNECTED;
  if (_pProtocol->GetType()==PT_INBOUND_HTTP_WSCONNECTION) {
    event=EVT_IN_AREC_CONNECTED;
  }

  const_cast<BaseClientApplication*>
      (_pProtocol->GetApplication())->OnNotifyEvent(CLOUD_MSG_EVENT,
                                                    event,
                                                    0);
}

void WSOutNetAudioStream::SignalDetachedFromInStream() {
  uint16_t event=EVT_OUT_AREC_DISCONNECTED;
  if (_pProtocol->GetType()==PT_INBOUND_HTTP_WSCONNECTION) {
    event=EVT_IN_AREC_DISCONNECTED;
  }
  const_cast<BaseClientApplication*>
      (_pProtocol->GetApplication())->OnNotifyEvent(CLOUD_MSG_EVENT,
                                                    event,
                                                    0);
}

bool WSOutNetAudioStream::FeedData(uint8_t *pData, uint32_t dataLength,
		uint32_t processedLength, uint32_t totalLength,
		double absoluteTimestamp, bool isAudio) {

  if (IsEnqueueForDelete())
    return true;


	_soundBuffer.ReadFromBuffer(pData, dataLength);
  uint32_t availcount= GETAVAILABLEBYTESCOUNT(_soundBuffer);

  if (!_pInStream)
    return false;

  if (availcount>_dataThreshold ) {
    IOBuffer header;
	  if (_pProtocol) {
      BaseWSSubProtocol* pWSProtocol=reinterpret_cast<BaseWSSubProtocol*>(_pProtocol);
      BaseAudioStream* pAudioStream = reinterpret_cast<BaseAudioStream*>(_pInStream);

      pAudioStream->GetHeader(header, availcount);
      pWSProtocol->EnqueueForWSOutbound (GETIBPOINTER(header), GETAVAILABLEBYTESCOUNT(header), false, WS_OPCODE_BINARY_FRAME);
      pWSProtocol->EnqueueForWSOutbound (GETIBPOINTER(_soundBuffer), availcount , true, WS_OPCODE_CONTINUATION);

      _soundBuffer.Ignore(availcount);
    }
  }


  return true;
}

bool WSOutNetAudioStream::FeedMSGData(uint8_t *pData, uint32_t dataLength)
{
  if (IsEnqueueForDelete()) {
    DEBUG ("Is EnququeForDelete return");
    return false;
  }

	if (_pProtocol) {
    return reinterpret_cast<BaseWSSubProtocol*>
       (_pProtocol)->EnqueueForWSOutbound (pData, dataLength, true, WS_OPCODE_TEXT_FRAME);
  }
  return false;
}
#endif
