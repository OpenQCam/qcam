/*
 * Copyright (c) 2013, Oculus Technologies Inc. All rights reserved.
 * Three Ocean (to@bcloud.us)
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

#ifndef _MP4DOCUMENTWRITER_H
#define _MP4DOCUMENTWRITER_H

#include "common.h"
#include "mediaformats/qmp4/writemovdocument.h"

enum MP4_WRITER_STATUS {
  IDLE = 0,
  RECORDING,
  NEXT_FILE,
  ERR_FAILED_TO_WRITE_VIDEO_DATA,
  ERR_FAILED_TO_WRITE_AUDIO_DATA,
  ERR_FAILED_TO_CLOSE_FILE,
  ERR_FAILED_TO_OPEN_NEW_FILE
};

class IMP4DocumentWriterCallback {
  public:
    virtual void OnFinish() = 0;
};

class MP4DocumentWriter
{
  private:
    uint32_t _duration;
    uint32_t _elapsedTime;
    uint32_t _fileCounter;
    uint32_t _iteration;
    string _filePrefix;
    WriteMovDocument *_pMovDocument;
    IMP4DocumentWriterCallback *_pCB;
    QMP4_TRAK_INFO *_pVideoTrack;
    QMP4_TRAK_INFO *_pAudioTrack;
    QMP4_FILE_TYPE _fileType;
    MP4_WRITER_STATUS _status;
    struct _streamInfo {
      uint8_t *sps;
      uint32_t spsLength;
      uint8_t *pps;
      uint32_t ppsLength;
    } streamInfo;

  public:
    MP4DocumentWriter(IMP4DocumentWriterCallback *cb);
    ~MP4DocumentWriter();
    // if no file prefix, save file in current directory by default file name
    // if no duration specified, keep recording till invoking Close()
    bool Initialize(QMP4_TRAK_INFO *pVideoTrak, QMP4_TRAK_INFO *pAudioTrak,
        QMP4_FILE_TYPE type, string filePrefix, uint32_t duration, uint32_t iteration);
    // close current file and switch to next file,
    // which will be proceeded while key frame arrived
    bool Next();
    // manually close file, which will be proceeded while key frame arrived
    bool Stop();
    bool AddVideoData(uint8_t *pData, uint32_t length, uint32_t delta, bool isKeyFrame);
    bool AddAudioData(uint8_t *pData, uint32_t length, uint32_t sampleNum);
    bool UpdateSPSPPS(uint8_t *pSPSData, uint32_t spsLength, uint8_t *pPPSData, uint32_t ppsLength);
    MP4_WRITER_STATUS GetStatus();

  private:
    string GetFileName();
    bool OpenNewFile();
    bool CloseFile();
    bool SwitchNextFile();
    bool ProcessStatus();

};

#endif /* _MP4DOCUMENTWRITER_H */

