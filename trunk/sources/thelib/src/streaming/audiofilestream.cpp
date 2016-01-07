#include "streaming/audiofilestream.h"
//thelib
#include "streaming/streamstypes.h"
#include "streaming/streamsmanager.h"
#include "streaming/baseoutstream.h"

AudioFileStream::AudioFileStream(StreamsManager *pStreamsManager,
                                 string name)
: BaseInStream(NULL, pStreamsManager, ST_IN_FILE_PCM, name),
  _pAudioFile(NULL) {
}

AudioFileStream::~AudioFileStream() {
  if (_pAudioFile) {
    fclose(_pAudioFile);
    _pAudioFile=NULL;
  }
}


StreamCapabilities* AudioFileStream::GetCapabilities() {
  NYI;
  return NULL;
}

bool AudioFileStream::SignalPlay(double &absoluteTimestamp, double &length) {
  return true;
}

bool AudioFileStream::SignalPause() {
  return true;
}
bool AudioFileStream::SignalResume() {
  return true;
}
bool AudioFileStream::SignalSeek(double &absoluteTimestamp) {
  return true;
}
bool AudioFileStream::SignalStop() {
  Close();
  return true;
}
bool AudioFileStream::FeedData(uint8_t *pData, uint32_t dataLength,
                               uint32_t processedLength, uint32_t totalLength,
                               double absoluteTimestamp, bool isAudio) {

  //DEBUG ("FeedData");
  if (_pAudioFile) {
    LinkedListNode<BaseOutStream *> *pTemp = _pOutStreams;
    if (pTemp) {
      uint8_t* txbuffer=new uint8_t[dataLength];
      uint32_t size=fread((char*)txbuffer, dataLength, 1, _pAudioFile);

      if (size>0) {
        pTemp->info->FeedData(txbuffer, size, 0, size, 0, true);
      }
      else {
        DEBUG ("file read over");
        fclose(_pAudioFile);
        _pAudioFile=NULL;
        pTemp->info->SignalStop();
      }
      delete txbuffer;
    }
    else {
      WARN ("no play pcm stream connected");
    }
  }
  return true;
}

void AudioFileStream::ReadyForSend() {
}

bool AudioFileStream::IsCompatibleWithType(uint64_t type) {
  return true;
}

void AudioFileStream::SignalOutStreamAttached(BaseOutStream *pStream) {
  NYI;
}
void AudioFileStream::SignalOutStreamDetached(BaseOutStream *pStream) {
  NYI;
}

void AudioFileStream::SignalStreamCompleted() {
  NYI;
}

bool AudioFileStream::Open(string filename) {

  Close();
  _pAudioFile=fopen(STR(filename), "rb");
  if (_pAudioFile) {
    fseek(_pAudioFile, 40, SEEK_SET);
  }
  else
    return false;
  return true;
}

void AudioFileStream::Close() {
  if (_pAudioFile) {
    fclose(_pAudioFile);
    _pAudioFile=NULL;
  }
}

bool AudioFileStream::IsReadOver() {
  return true;
}
