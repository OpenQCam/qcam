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


#ifndef _UDSRECORDTHREAD_H
#define _UDSRECORDTHREAD_H

#include "system/thread/baseudsthread.h"
#include "mediaformats/qmp4/mp4documentwriter.h"
#include "streaming/streamcapabilities.h"

namespace app_qicstreamer {

class MP4RecordStream;
class UDSRecordThread
  : public BaseUDSThread
  , public IMP4DocumentWriterCallback
{
  private:
    IOBuffer _buffer;
    MP4RecordStream *_pRecordStream;
    string _fileName;
    uint32_t _duration;
    uint32_t _iteration;
    // mp4 info
    struct STREAM_INFO {
      struct VIDEO {
        uint8_t sps[20];
        uint8_t spsLength;
        uint8_t pps[20];
        uint8_t ppsLength;
        uint16_t width;
        uint16_t height;
        uint16_t fps;
      } video ;
      struct AUDIO {
        uint32_t sampleRate;
        uint8_t numOfChannels;
      } audio;
    } _streamInfo;
    QMP4_TRAK_INFO _videoTrakInfo;
    QMP4_TRAK_INFO _audioTrakInfo;
    // file pointer
    MP4DocumentWriter *_pMovDocument;
    uint64_t _lastTS;
    Mutex _mutex;
  public:
    enum MT_MESSAGE {
      MM_THREAD_CREATED = 0,
      MM_LINK_STREAM,
      MM_ACK_START,
      MM_ACK_NEXT,
      MM_ACK_STOP,
      MM_ACK_FINISH,
      MM_ERROR_START,
      MM_ERROR_NEXT,
      MM_ERROR_STOP,
      MM_ERROR_VIDEO,
      MM_ERROR_AUDIO,
      MM_ERROR_UNKNOWN
    };
    enum CT_MESSAGE {
      CM_START = 0,
      CM_NEXT,
      CM_STOP
    };
    UDSRecordThread();
    virtual ~UDSRecordThread();
    // functions for main thread
    void SetRecordStream(MP4RecordStream* pStream);
    MP4RecordStream* GetRecordStream();
    bool StartRecording(Variant &parameters);
    bool SwitchToNextFile(string fileName);
    bool StopRecording();
    //void SendMessage(Variant &message);
    // callback functions for child thread
    // which are not able to be invoked by main thread
    virtual void OnConnected();
    // child thread observer interface
    virtual bool OnMessage(BaseUDSMessageProtocol *pProtocol, Variant &message);
    virtual bool OnData(UDS_MESSAGE_TYPE type, uint8_t *pData, uint32_t length, uint64_t ts);

    // IMP4DocumentWriterCallback
    virtual void OnFinish();
  private:
    bool ProcessVideoData(uint8_t *pData, uint32_t length, uint64_t ts);
    bool ProcessAudioData(uint8_t *pData, uint32_t length, uint64_t ts);
    bool AckMessage(MT_MESSAGE msg);
    bool OpenNewMovFile();
};

}
#endif /* _UDSRECORDTHREAD_H */

