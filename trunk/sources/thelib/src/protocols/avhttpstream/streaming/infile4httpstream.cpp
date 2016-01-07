// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    :
// // Version :
// // Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//


#include "protocols/avhttpstream/streaming/infile4httpstream.h"
#include "protocols/avhttpstream/inboundavhttpstreamprotocol.h"
#include "streaming/baseoutstream.h"
#include "streaming/streamstypes.h"
#include "protocols/baseprotocol.h"

#ifndef HAS_MMAP
map<string, pair<uint32_t, File *> > InFile4HTTPStream::_fileCache;
#endif /* HAS_MMAP */

InFile4HTTPStream::InFileStreamTimer::InFileStreamTimer(InFile4HTTPStream *pInFileStream) {
  _pInFileStream = pInFileStream;
}

InFile4HTTPStream::InFileStreamTimer::~InFileStreamTimer() {

}

void InFile4HTTPStream::InFileStreamTimer::ResetStream() {
  _pInFileStream = NULL;
}

bool InFile4HTTPStream::InFileStreamTimer::TimePeriodElapsed() {
  if (_pInFileStream != NULL)
    _pInFileStream->ReadyForSend();
  return true;
}

#define FILE_STREAMING_STATE_PAUSED 0
#define FILE_STREAMING_STATE_PLAYING 1
#define FILE_STREAMING_STATE_FINISHED 2

InFile4HTTPStream::InFile4HTTPStream(BaseProtocol *pProtocol,
    StreamsManager *pStreamsManager, uint64_t type, string name)
: BaseInStream(pProtocol, pStreamsManager, type, name) {
  if (!TAG_KIND_OF(type, ST_IN_FILE)) {
    ASSERT("Incorrect stream type. Wanted a stream type in class %s and got %s",
        STR(tagToString(ST_IN_FILE)), STR(tagToString(type)));
  }
  _pTimer = NULL;
  _pFile = NULL;

  //timing info
  _feedPeriod = 0;
  _totalSentTime = 0;
  _startFeedingTime = 0;

  _bufferWindow = 0;
  //current state info
  _streamingState = FILE_STREAMING_STATE_PAUSED;

  _streamCapabilities.Clear();
}

InFile4HTTPStream::~InFile4HTTPStream() {
  if (_pTimer != NULL) {
    _pTimer->ResetStream();
    _pTimer->EnqueueForDelete();
    _pTimer = NULL;
  }
  INFO("Delete InFileStream %s", STR(GetName()));
  ReleaseFile(_pFile);
}

InFile4HTTPStream *InFile4HTTPStream::GetInstance(InboundAVHTTPStreamProtocol *pProtocol,
    StreamsManager *pStreamsManager, Variant &metaData) {
  string filePath;

  if(metaData.HasKey(IN_FILE_STREAM_FILE_PATH)){
    filePath =(string)metaData[IN_FILE_STREAM_FILE_PATH];
    if(IsFileOpened(filePath)){
      FATAL("File was opened. fullPath: `%s`", STR(filePath));
      return NULL;
    }
    if(!fileExists(filePath)){
      FATAL("File not found. fullPath: `%s`", STR(filePath));
      return NULL;
    }
  }
  if(!metaData.HasKey(IN_FILE_STREAM_NAME)){
    metaData[IN_FILE_STREAM_NAME] = md5(generateRandomString(8), true);
  }
  return (new InFile4HTTPStream(pProtocol, pStreamsManager, (uint64_t)metaData[IN_FILE_STREAM_TYPE], metaData[IN_FILE_STREAM_NAME]));
}

bool InFile4HTTPStream::FeedData(uint8_t *pData, uint32_t dataLength,
    uint32_t processedLength, uint32_t totalLength,
    double absoluteTimestamp, bool isAudio) {
  ASSERT("Operation not supported");
  return false;
}

bool InFile4HTTPStream::IsCompatibleWithType(uint64_t type) {
  return (TAG_KIND_OF(type, ST_OUT_HTTP_MP4));
}

void InFile4HTTPStream::SignalOutStreamAttached(BaseOutStream *pOutStream) {
  FINEST("outbound stream %u attached from inbound stream %u",
      pOutStream->GetUniqueId(), GetUniqueId());
}

void InFile4HTTPStream::SignalOutStreamDetached(BaseOutStream *pOutStream) {
  FINEST("outbound stream %u detached from inbound stream %u",
      pOutStream->GetUniqueId(), GetUniqueId());
}




StreamCapabilities * InFile4HTTPStream::GetCapabilities() {
  return &_streamCapabilities;
}

bool InFile4HTTPStream::ResolveCompleteMetadata(Variant &metaData) {

  metaData[IN_FILE_STREAM_NAME] = (string) GetName();
  metaData[IN_FILE_STREAM_FILE_SIZE] = (uint64_t) _pFile->Size();
  metaData[IN_FILE_STREAM_DURATION] = (uint32_t) _feedPeriod;
  metaData[IN_FILE_STREAM_STATE] = (uint8_t) _streamingState;
  metaData[IN_FILE_STREAM_CURSOR] = (uint64_t) _pFile->Cursor();
  metaData[IN_FILE_STREAM_WINDOW] = (uint32_t) _bufferWindow;
  return true;
}

bool InFile4HTTPStream::Initialize(Variant &metaData, bool hasTimer) {


  string filePath;
  if(metaData.HasKey(IN_FILE_STREAM_FILE_PATH)){
    filePath =(string)metaData[IN_FILE_STREAM_FILE_PATH];
  }else{
    FATAL("No file name");
    return false;
  }

  _pFile = GetFile(filePath, 128*1024);

  if(_pFile == NULL){
    FATAL("Unable to open file %s", STR(filePath));
    return false;
  }

  if(metaData.HasKey(IN_FILE_STREAM_DURATION)){
    _feedPeriod = metaData[IN_FILE_STREAM_DURATION];
  }else{
    _feedPeriod = 1;
  }

  if(metaData.HasKey(IN_FILE_STREAM_WINDOW)){
    _bufferWindow = metaData[IN_FILE_STREAM_WINDOW];
  }else{
    _bufferWindow = 512*1024; //1MB; //8K * 30 fps
  }


  //Create the timer to generate stream event
  if (hasTimer) {
    _pTimer = new InFileStreamTimer(this);
    _pTimer->EnqueueForTimeEvent(_feedPeriod);
  }

  _buffer.EnsureSize(_bufferWindow);
  ResolveCompleteMetadata(metaData);

  return true;
}

bool InFile4HTTPStream::SignalPlay(double &absoluteTimestamp, double &length) {

  _streamingState = FILE_STREAMING_STATE_PLAYING;
  ReadyForSend();
  return true;
}

bool InFile4HTTPStream::SignalPause() {
  //1. Is this already paused
  if (_streamingState != FILE_STREAMING_STATE_PLAYING)
    return true;

  //2. Put the stream in paused mode
  _streamingState = FILE_STREAMING_STATE_PAUSED;

  //3. Done
  return true;
}

bool InFile4HTTPStream::SignalResume() {
  //1. Is this already active
  if (_streamingState == FILE_STREAMING_STATE_PLAYING)
    return true;

  //2. Put the stream in active mode
  _streamingState = FILE_STREAMING_STATE_PLAYING;

  //3. Start the feed reaction
  ReadyForSend();

  //5. Done
  return true;
}

bool InFile4HTTPStream::SignalSeek(double &offset) {

  uint64_t byteOffset = (uint64_t)offset;
  _pFile->SeekTo(byteOffset);
  return false;
}

bool InFile4HTTPStream::SignalStop() {
  //1. Is this already paused
  if (_streamingState != FILE_STREAMING_STATE_PLAYING)
    return true;

  //2. Put the stream in paused mode
  _streamingState = FILE_STREAMING_STATE_PAUSED;

  //3. Done
  return true;
}

void InFile4HTTPStream::ReadyForSend() {
  if (!Feed()) {
    FATAL("Feed failed");
    if (_pOutStreams != NULL)
      _pOutStreams->info->EnqueueForDelete();
  }
}

bool InFile4HTTPStream::Feed() {
  //1. Are we in paused state?
  if (_streamingState != FILE_STREAMING_STATE_PLAYING)
    return true;

  //Check outbound protocol buffer size
  IOBuffer *pOutBuffer = reinterpret_cast<InboundAVHTTPStreamProtocol*>(_pProtocol)->GetProtocolOutputBuffer();
  if (GETAVAILABLEBYTESCOUNT(*pOutBuffer)>= 2*_bufferWindow) {
    INFO ("outbound buffer full");
    return true;
  }

  _totalSentTime += _feedPeriod;

  //3. Test to see if we have sent the last frame
  uint64_t restSize = _pFile->Size() - _pFile->Cursor();
  INFO("rest %d, file size %d, current %d", (uint32_t) restSize, (uint32_t) _pFile->Size(), (uint32_t) _pFile->Cursor());
  if(restSize == 0){
    return true;
  }

  if(restSize < _bufferWindow){
    //if(!_pFile->ReadBuffer(GETIBPOINTER(_buffer), restSize)){
    if(!_buffer.ReadFromFs(*_pFile, restSize)){
        FATAL("Unable to read buffer from stream file");
        return false;
    }
    FINEST("Done streaming file");
    if(_pOutStreams != NULL) _pOutStreams->info->SignalStreamCompleted();
    _streamingState = FILE_STREAMING_STATE_FINISHED;

  }else{
    //if(!_pFile->ReadBuffer(GETIBPOINTER(_buffer), _bufferWindow)){
    if(!_buffer.ReadFromFs(*_pFile, _bufferWindow)){
      FATAL("Unable to read buffer from stream file");
      return false;
    }
  }

  if(_pOutStreams == NULL){
    FATAL("Unable to feed data");
    return false;
  }

  if (!_pOutStreams->info->FeedData(
      GETIBPOINTER(_buffer), //pData
      GETAVAILABLEBYTESCOUNT(_buffer), //dataLength
      0, //processedLength
      GETAVAILABLEBYTESCOUNT(_buffer), //totalLength
      0,
      false
      )) {
    FATAL("Unable to feed data");
    return false;
  }

  //10. Discard the data
  _buffer.IgnoreAll();

  return true;
}

#ifdef HAS_MMAP

MmapFile* InFile4HTTPStream::GetFile(string filePath, uint32_t windowSize) {
  if (windowSize == 0)
    windowSize = 131072;
  MmapFile *pResult = NULL;
  pResult = new MmapFile();
  if (!pResult->Initialize(filePath, windowSize, false)) {
    delete pResult;
    return NULL;
  }
  return pResult;
}

void InFile4HTTPStream::ReleaseFile(MmapFile *pFile) {
  if (pFile == NULL)
    return;
  delete pFile;
}

#else

bool InFile4HTTPStream::IsFileOpened(string filePath) {
  return false;
 // return MAP_HAS1(_fileCache, filePath);
}

File* InFile4HTTPStream::GetFile(string filePath, uint32_t windowSize) {
  File *pResult = NULL;
  pResult = new File();
  if (!pResult->Initialize(filePath)) {
    delete pResult;
    return NULL;
  }

#if 0
  if (!MAP_HAS1(_fileCache, filePath)) {
    pResult = new File();
    if (!pResult->Initialize(filePath)) {
      delete pResult;
      return NULL;
    }
    _fileCache[filePath] = pair<uint32_t, File *>(1, pResult);
  } else {
    pResult = _fileCache[filePath].second;
    _fileCache[filePath].first++;
  }
#endif
  return pResult;
}

void InFile4HTTPStream::ReleaseFile(File *pFile) {
  if (pFile == NULL)
    return;
#if 0
  if (!MAP_HAS1(_fileCache, pFile->GetPath())) {
    WARN("You tryed to release a non-cached file: %s", STR(pFile->GetPath()));
    return;
  }
  _fileCache[pFile->GetPath()].first--;
  if (_fileCache[pFile->GetPath()].first == 0) {
    _fileCache.erase(pFile->GetPath());
    delete pFile;
  }
#else
  delete pFile;
#endif
}
#endif /* HAS_MMAP */
