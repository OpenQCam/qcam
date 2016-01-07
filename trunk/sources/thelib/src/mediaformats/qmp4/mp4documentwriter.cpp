/*
 * Copyright (c) 2013, Three Ocean (to@bcloud.us). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mediaformats/qmp4/mp4documentwriter.h"

MP4DocumentWriter::MP4DocumentWriter(IMP4DocumentWriterCallback *cb)
  : _duration(0)
  , _elapsedTime(0)
  , _fileCounter(0)
  , _iteration(0)
  , _pMovDocument(NULL)
  , _pCB(cb)
  , _status(IDLE)
{
  memset(&streamInfo, 0, sizeof(streamInfo));
  _pMovDocument = new WriteMovDocument();
}

MP4DocumentWriter::~MP4DocumentWriter()
{
  delete [] streamInfo.sps;
  delete [] streamInfo.pps;
  if(_pMovDocument != NULL) {
    CloseFile();
    delete _pMovDocument;
    _pMovDocument = NULL;
  }
}

bool MP4DocumentWriter::Initialize(QMP4_TRAK_INFO *vidTrakInfo,
    QMP4_TRAK_INFO *audTrakInfo, QMP4_FILE_TYPE type, string filePrefix,
    uint32_t duration, uint32_t iteration)
{
  _duration = duration*1000;
  //_elapsedTime = 1000;
  _elapsedTime = 0;
  _filePrefix = filePrefix;
  _fileCounter = 0;
  _iteration = iteration;
  _pVideoTrack = vidTrakInfo;
  _pAudioTrack = audTrakInfo;
  _fileType = type;
  return OpenNewFile();
}



bool MP4DocumentWriter::Next()
{
  _status = NEXT_FILE;
  return true;
}

bool MP4DocumentWriter::Stop()
{
  INFO("mp4 stopping");
  // when Stop() was invoked, mp4 should be writen out in no time
  //_status = STOP_RECORDING;
  return CloseFile();
}

bool MP4DocumentWriter::AddVideoData(uint8_t *pData, uint32_t length, uint32_t delta, bool isKeyFrame)
{
  // status is not RECORDING, ignore all data
  if(_status != RECORDING) {
    WARN("Unable to add video because status is not recording");
    return true;
  }
  _elapsedTime += delta;
  if(isKeyFrame) {
    // calculate elasped time at key frame to avoid frequently checking
    if(_elapsedTime > _duration && _duration > 0) {
      _status = NEXT_FILE;
    }
    if(!ProcessStatus()){
      FATAL("Error!");
      return false;
    }
    if(_status != RECORDING) {
      return true;
    }
  }
  if(_pMovDocument->AddVidData(pData, length, delta, isKeyFrame)){
    _status = ERR_FAILED_TO_WRITE_VIDEO_DATA;
    FATAL("Unable to add video data");
    return false;
  }
  return true;
}
bool MP4DocumentWriter::AddAudioData(uint8_t *pData, uint32_t length, uint32_t sampleNum)
{
  if(_pMovDocument->AddAudData(pData, length, sampleNum)){
    _status = ERR_FAILED_TO_WRITE_AUDIO_DATA;
    FATAL("Unable to add video data");
    return false;
  }
  return true;
}
bool MP4DocumentWriter::UpdateSPSPPS(uint8_t *pSPSData, uint32_t spsLength, uint8_t *pPPSData, uint32_t ppsLength)
{
  streamInfo.sps = new uint8_t[spsLength];
  memcpy(streamInfo.sps, pSPSData, spsLength);
  streamInfo.spsLength = spsLength;
  streamInfo.pps = new uint8_t[ppsLength];
  memcpy(streamInfo.pps, pPPSData, ppsLength);
  streamInfo.ppsLength = ppsLength;
  return (_pMovDocument->UpdateSPSR(pSPSData, spsLength, pPPSData, ppsLength) == 0);
}

MP4_WRITER_STATUS MP4DocumentWriter::GetStatus()
{
  return _status;
}

//private
string MP4DocumentWriter::GetFileName()
{
  time_t currentTime;
  struct tm *timeInfo;
  char buf[80];
  string fileName;
  time(&currentTime);
  timeInfo = localtime(&currentTime);
  strftime(buf, sizeof(buf), "%F_%H%M%S", timeInfo);
  if(_filePrefix.length() > 0) {
    fileName = format("%s_%d.mp4", STR(_filePrefix), _fileCounter);
  } else {
    fileName = format("%s_%d.mp4", buf, _fileCounter);
  }
  INFO("%s", STR(fileName));
  return fileName;
}

bool MP4DocumentWriter::OpenNewFile()
{
  if(_pMovDocument->Open(_pVideoTrack, _pAudioTrack, _fileType, STR(GetFileName()))){
    _status = ERR_FAILED_TO_OPEN_NEW_FILE;
    FATAL("Unable to open file");
    return false;
  }
  if(streamInfo.spsLength != 0 && streamInfo.ppsLength != 0) {
    _pMovDocument->UpdateSPSR(streamInfo.sps, streamInfo.spsLength, streamInfo.pps, streamInfo.ppsLength);
  }
  //_elapsedTime = 1000;
  _elapsedTime = 0;
  _status = RECORDING;
  return true;
}

bool MP4DocumentWriter::CloseFile()
{
  if(_pMovDocument->Close()) {
    _status = ERR_FAILED_TO_CLOSE_FILE;
    FATAL("Close file failed");
    return false;
  }
  _status = IDLE;
  return true;
}

bool MP4DocumentWriter::SwitchNextFile()
{
  _fileCounter++;
  if(!CloseFile()){
    return false;
  }
  if(_fileCounter >= _iteration && _iteration > 0) {
    _pCB->OnFinish();
    return true;
  }
  if(!OpenNewFile()){
    FATAL("Unable to create new file");
    return false;
  }
  return true;
}

bool MP4DocumentWriter::ProcessStatus()
{
  switch(_status) {
    case IDLE:
      return true;
      break;
    case RECORDING:
      return true;
      break;
    case NEXT_FILE:
      return SwitchNextFile();
      break;
    //case STOP_RECORDING:
    //  return CloseFile();
    //  break;
    case ERR_FAILED_TO_WRITE_AUDIO_DATA:
    case ERR_FAILED_TO_WRITE_VIDEO_DATA:
    case ERR_FAILED_TO_CLOSE_FILE:
    case ERR_FAILED_TO_OPEN_NEW_FILE:
      FATAL("Error(%d)", _status);
      break;
    default:
      FATAL("Unknown status(%d)", _status);
      break;
  }
  return true;
}
