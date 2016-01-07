#include "mux/basetsmuxoutstream.h"
//thelib
#include "streaming/streamstypes.h"
#include "streaming/baseinstream.h"
//#include "mediaformats/mp4/atomdumy.h"
extern "C"
{
#include "mediaformats/ts/compat.h"
#include "mediaformats/ts/es_fns.h"
#include "mediaformats/ts/accessunit_fns.h"
#include "mediaformats/ts/avs_fns.h"
#include "mediaformats/ts/audio_fns.h"
#include "mediaformats/ts/ts_fns.h"
#include "mediaformats/ts/tswrite_fns.h"
#include "mediaformats/ts/misc_fns.h"
#include "mediaformats/ts/printing_fns.h"
#include "mediaformats/ts/version.h"
}
//hardware
#include "basehardwareabstractdevice.h"
#include "hardwaremanager.h"
#include "storage/basestoragedevice.h"
#include "system/systemmanager.h"
#include "sys/time.h"


static TS_writer_p output;
static uint32_t video_pts=0;


BaseTSMuxOutStream::BaseTSMuxOutStream(StreamsManager *pStreamsManager, string name)
  : BaseOutStream (NULL, pStreamsManager, ST_OUT_FILE_TS, name),
    ISystemCommandCallback(),
    _muxState(MUX_STOP_RECORD),
    _pVideoStream(NULL),
    _pAudioStream(NULL)
{
  _duration=0;
  _videoTimeStamp=0;
  _audioTimeStamp=0;
  _status=0;
  _frameCounter=0;
  _keyframeCounter=0;
  _fileCounter=0;
  _firstFrameTime=_lastFrameTime=0;
  //_mp4file=NULL;
   tsFileIsOpen=false;
   memset (cacheOutputName,'\0',64);
   //strcpy(cacheOutputName,"/myout0.ts");
   strcpy(cacheOutputName,"/qic0.ts");
   _storeDirty=false;
   _frameBufferSize=0;
   video_time=0.0;
}

BaseTSMuxOutStream::~BaseTSMuxOutStream() {
  _pVideoStream = NULL;
  _pAudioStream = NULL;
  /*if (_mp4file) {
    _mp4file->Close();
    delete _mp4file;
  }*/
}

bool BaseTSMuxOutStream::Initialize() {
  return true;
};

bool BaseTSMuxOutStream::SignalPlay(double &absoluteTimestamp, double &length) {
  NYIR;
}
bool BaseTSMuxOutStream::SignalStop() {
  NYIR;
}

bool BaseTSMuxOutStream::SignalPause() {
  NYIR;
}

bool BaseTSMuxOutStream::SignalResume() {
  NYIR;
}

bool BaseTSMuxOutStream::SignalSeek(double &absoluteTimestamp) {
  NYIR;
}

void BaseTSMuxOutStream::SignalAttachedToInStream() {
}

void BaseTSMuxOutStream::SignalDetachedFromInStream() {
}

void BaseTSMuxOutStream::SignalStreamCompleted() {
}

bool BaseTSMuxOutStream::IsCompatibleWithType(uint64_t type) {
  return true;;
}

void BaseTSMuxOutStream::RegisterStream(BaseInStream *inStream) {
  bool reverseLink=false;
  if (!inStream) {
    WARN ("Register a NULL stream");
    return;
  }

  if (inStream->GetType() == ST_IN_AV_MUX) {
    _pVideoStream = inStream;
    _status |= MP4MUX_REG_VIDEO;
    reverseLink=true;
  }
  /*else if (inStream->GetType() == ST_IN_AUDIO_MP3) {
    _pAudioStream = inStream;
    _status |= MP4MUX_REG_AUDIO;
  }
  else if (inStream->GetType() == ST_IN_AUDIO_PCM) {
    _pAudioStream = inStream;
    _status |= MP4MUX_REG_AUDIO;
  }
  else if (inStream->GetType() == ST_IN_AV_MUX) {
    _pVideoStream = inStream;
    _status |= (MP4MUX_REG_AUDIO | MP4MUX_REG_VIDEO);
  }*/
  else {
    FATAL ("Unsupported instream type:%llx", inStream->GetType()) ;
    return;
  }
  FATAL ("Link ok") ;
  inStream->Link(reinterpret_cast<BaseOutStream *>(this), reverseLink);
}

bool BaseTSMuxOutStream::IsRecording() {
  if (_muxState!=MUX_STOP_RECORD)
    return true;
  return false;
}

bool BaseTSMuxOutStream::IsError() {
  bool ret;
  ret=(_muxState!=MUX_STOP_RECORD)&&(_muxState!=MUX_START_RECORD);
  return ret;
};

uint8_t BaseTSMuxOutStream::GetStatus() {
  return _status;
}

void BaseTSMuxOutStream::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                        uint32_t dataLength) {
  NYI;
}

void BaseTSMuxOutStream::UnRegisterAllStream() {

  /*Same behavior of StopRecord function in  record actionhandler */
  reinterpret_cast<BaseHardwareAbstractDevice *>(HardwareManager::GetHardwareInstance(HT_VIDEO_AVC))
    ->UnRegisterObserver((IDeviceObserver *)this);
  if (_pVideoStream)
    UnRegisterStream(_pVideoStream);
  if (_pAudioStream)
    UnRegisterStream(_pAudioStream);
}

void BaseTSMuxOutStream::UnRegisterStream(BaseInStream *inStream) {
 if (!inStream) {
    WARN ("UnRegister a NULL stream");
    return;
  }

  inStream->UnLink(reinterpret_cast<BaseOutStream *>(this), true);
  if (inStream->GetType() == ST_IN_CAM_AVC) {
    _pVideoStream = NULL;
    _status &= ~MP4MUX_REG_VIDEO;
  }
  /*else if (inStream->GetType() == ST_IN_AUDIO_MP3) {
    _pAudioStream = NULL;
    _status &= ~MP4MUX_REG_AUDIO;
  }
  else if (inStream->GetType() == ST_IN_AUDIO_AAC) {
    _pAudioStream = NULL;
    _status &= ~MP4MUX_REG_AUDIO;
  }*/
  else
    return;
}

bool BaseTSMuxOutStream::UpdateInfo(double currentTime) {

  /*if (!_mp4file) {
    return false;
  }*/

  /*if (_videoInfo.size() >= 2) {
    _mp4file->SeekEnd();
    for (uint32_t i=0; i<_videoInfo.size()-1; i++) {
      _mp4file->UpdateHeader(_videoInfo[i], _videoInfo[i+1].timeStamp-_videoInfo[i].timeStamp);
    }
    _updated=true;
    _videoTimeStamp=currentTime;
  }
  else
    return true;

  _mp4file->UpdateSize() ;

  if (!_videoInfo.empty()) {
    AtomUpdateInfo lastInfo = _videoInfo.back();
    _videoInfo.clear();
    _videoInfo.push_back(lastInfo);
  }*/
  return true;
}

static int is_I_or_IDR_frame(access_unit_p  frame)
{
  return (frame->primary_start != NULL &&
          frame->primary_start->nal_ref_idc != 0 &&
          (frame->primary_start->nal_unit_type == NAL_IDR ||
           all_slices_I(frame)));
}

void itoa (int n,char s[])
{
int i,j,sign;
char tmp[256]={'\0'};
if((sign=n)<0)//记录符号
n=-n;//使n成为正数
i=0;
do{
      tmp[i++]=n%10+'0';//取下一个数字
}
while ((n/=10)>0);//删除该数字
if(sign<0)
tmp[i++]='-';
tmp[i]='\0';
int k=0;
for(j=i-1;j>=0;j--) {//生成的数字是逆序的，所以要逆序输出
      //FATAL("%c",tmp[j]);
      s[k++]=tmp[j];
}
}

double round(double d)
{
  return floor(d + 0.5);
}

//static uint32_t accframe=0;
//static uint32_t storeAccframe=0;
bool BaseTSMuxOutStream::FeedData (uint8_t *pData, uint32_t dataLength,
                                    uint32_t processedLength, uint32_t totalLength,
                                    double absoluteTimestamp, bool isKeyFrame) {

  uint8_t *_storeBuffer = (uint8_t *)malloc (sizeof(uint8_t)*(dataLength));
  memcpy(_storeBuffer, pData, dataLength);
  //uint8_t *_storeBuffer=pData;
  uint32_t _storeBufferSize=dataLength;
    _storeDirty=1;

  if(_storeDirty) {
    //valid frame
    _storeDirty=false;
    double video_time_abs=((video_time-5.0)>=0)?(video_time-5.0):(5.0-video_time);
    if((video_time_abs<0.2 || video_time >= 5.2) && isKeyFrame ) {
      tswrite_close(output, 0);
      video_time=0.0;
      //FATAL("video_pts: %d\n",video_pts);
      //delete the old one
      if(_fileCounter>=10) {
        //string cmd = "rm /tmp/myout";
#ifdef __x86__
        //char cmd[64]="rm /usr/share/nginx/html/myout";
        char cmd[64]="rm /usr/share/nginx/html/qic";
#else
        char cmd[64]="rm /etc_ro/web/myout";
#endif
        char rmNum[32]="";
        char posOutputName[4]=".ts";
        itoa(_fileCounter-10,rmNum);
        strcat(cmd, rmNum);
        strcat(cmd, posOutputName);
        string cmdStr=cmd;
        string returnInfo;
        SystemManager::DoSystemCommand(cmdStr, returnInfo);
      }
      //create ts file
#ifdef __x86__
      //char preOutputName[64]="/usr/share/nginx/html/myout";
      char preOutputName[64]="/usr/share/nginx/html/qic";
#else
      char preOutputName[64]="/etc_ro/web/myout";
#endif
      //char nameForLink[64]="/myout";
      char nameForLink[64]="/qic";
      char posOutputName[4]=".ts";
      char fileNum[32]="";
      itoa(_fileCounter,fileNum);
      strcat(preOutputName, fileNum);
      strcat(preOutputName, posOutputName);
      strcat(nameForLink, fileNum);
      strcat(nameForLink, posOutputName);

      int err;
      err = tswrite_open(TS_W_FILE, preOutputName,NULL,0,0,&output);
      if (err) {
    FATAL("[base TS outStream] tswrite_open error");
        tsFileIsOpen=false;
        return false;
      }
      int ii;
      uint32_t prog_pids[2];
      byte     prog_type[2];
      for (ii=0; ii<8; ii++) {
         err = write_TS_null_packet(output);
         if (err) return false;
      }
      prog_pids[0] = DEFAULT_VIDEO_PID;
      prog_pids[1] = DEFAULT_AUDIO_PID;
      prog_type[0] = AVC_VIDEO_STREAM_TYPE;
      //FATAL("write_TS_program_data2");
      err = write_TS_program_data2(output,
           1, // transport stream id
           1, // program number
         DEFAULT_PMT_PID,
         DEFAULT_VIDEO_PID,  // PCR pid
         2,prog_pids,prog_type);

      /*if((_fileCounter-1)/2==1) {//make m3u8
FATAL("my out name cache: %s\n",cacheOutputName);
        char cmd[128]="sh keepalive &";*/
        /*char space[2]=" ";
        char seqNum[32]="";
        itoa(_fileCounter/2,seqNum);
        strcat(cmd, seqNum);
        strcat(cmd, space);
        char preDuration[4]="";
        itoa(round(cacheDuration),preDuration);
        strcat(cmd, preDuration);
        strcat(cmd, space);
        strcat(cmd, cacheOutputName);
        strcat(cmd, space);
        char thisDuration[4]="";
        itoa(round(video_time),thisDuration);
        strcat(cmd, thisDuration);
        strcat(cmd, space);
  strcat(cmd, nameForLink);
        FATAL("my cmd is: %s\n", cmd);*/
        //strcat(cmd, posOutputName);
        /*string cmdStr=cmd;
        string returnInfo;
        SystemManager::DoSystemCommand(cmdStr, returnInfo);
      }*/

      _fileCounter++;
      _frameCounter=0;
      cacheDuration=video_time;
      video_time=0;
      //cacheOutputName=preOutputName;
      memset (cacheOutputName,'\0',64);
      strcpy(cacheOutputName,nameForLink);

      //FATAL("tswrite_open success");
    }//end of segmenter
    if (!tsFileIsOpen) {
      tsFileIsOpen=true;
      _firstFrameTime=absoluteTimestamp;

      //create ts file
#ifdef __x86__
      //char myOutputName[64]="/usr/share/nginx/html/myout0.ts";
      char myOutputName[64]="/usr/share/nginx/html/qic0.ts";
#else
      char myOutputName[64]="/etc_ro/web/myout0.ts";
#endif
      int err;
      if(_fileCounter==0) {
        err = tswrite_open(TS_W_FILE, myOutputName,NULL,0,0,&output);
          if (err) {
        FATAL("[base TS outStream] tswrite_open error");
            tsFileIsOpen=false;
            return false;
          }
      }
      _fileCounter++;
      //FATAL("tswrite_open success");

      int ii;
      uint32_t prog_pids[2];
      byte     prog_type[2];
      for (ii=0; ii<8; ii++) {
         err = write_TS_null_packet(output);
         if (err) return false;
      }
      prog_pids[0] = DEFAULT_VIDEO_PID;
      prog_pids[1] = DEFAULT_AUDIO_PID;
      prog_type[0] = AVC_VIDEO_STREAM_TYPE;
      //FATAL("write_TS_program_data2");
      err = write_TS_program_data2(output,
           1, // transport stream id
           1, // program number
         DEFAULT_PMT_PID,
         DEFAULT_VIDEO_PID,  // PCR pid
         2,prog_pids,prog_type);
    }

    _lastFrameTime = absoluteTimestamp;
    _duration = (uint64_t)(_lastFrameTime-_firstFrameTime);


    //init es
    ES_p   video_es = NULL;
    //FATAL("build_elementary_stream_mm");
    build_elementary_stream_mm(_storeBuffer, _storeBufferSize, &video_es);
    access_unit_context_p h264_video_context = NULL;
    int err = build_access_unit_context(video_es,&h264_video_context);
    if (err) {
      free(video_es);
      FATAL("build_access_unit_context error");
    }



    //start merge

    //uint64_t video_frame_rate=25;
    //uint64_t video_pts_increment = 90000 / video_frame_rate;

    bool got_video = true;

    //while (got_video)
    {
      //write frame info
      access_unit_p  access_unit;
      //FATAL("get_next_h264_frame");
      err = get_next_h264_frame(h264_video_context,0,0,&access_unit);
      if (err == EOF)
      {
        //FATAL("EOF: no more video data\n");
        got_video=false;
      }else if(err){
        FATAL("[base TS outStream] get_next_h264_frame error\n");
        free(_storeBuffer);
        free(video_es);
        free_access_unit_context(&h264_video_context);
        return false;
      }
      if(got_video) {
        _frameCounter++;
        video_time = _frameCounter / 25.0;
        video_pts += 3600;
        //FATAL("is_I_or_IDR_frame");
        if (!is_I_or_IDR_frame(access_unit)){
           err = write_access_unit_as_TS_with_pts_dts(access_unit,h264_video_context,
                      output,DEFAULT_VIDEO_PID,
                  TRUE, video_pts,
                  TRUE, video_pts);
        }else {
          err = write_access_unit_as_TS_with_PCR(access_unit,h264_video_context,
                         output,DEFAULT_VIDEO_PID,
                       video_pts,0);
        }
        if(err) {
          FATAL("[base TS outStream] is_I_or_IDR_frame error");
          free_access_unit(&access_unit);
          free(_storeBuffer);
          free(video_es);
          free_access_unit_context(&h264_video_context);
          return false;
        }
        free_access_unit(&access_unit);

        // Did the logical video stream end after the last access unit?
        if (h264_video_context->end_of_stream)
        {
          //FATAL("Found End-of-stream NAL unit\n");
          got_video = FALSE;
        }
        //FATAL("is_I_or_IDR_frame done");
      }//end of got video
    }//end of got video while
    free(_storeBuffer);
    free_access_unit_context(&h264_video_context);
    free(video_es);
    //FATAL("write video done");
    //INFO("%.2f\n", video_time);

    if(_fileCounter==0) {
      cacheDuration=video_time;
    }
  }//end of dirty

  return true;
}

void BaseTSMuxOutStream::OnDeviceVideoData(DeviceData& data) {
FATAL("base TS mux out stream  on devicevideodata\n");
  /*uint8_t naluType = 0;
  uint8_t spsLength = 0;
  LinkedListNode<BaseOutStream *> *pTemp;

  naluType = data.buffer[4] & 0x1F;
  switch (naluType){
    case 5:
      if(_requestKeyFrame){
        ResendSPSPPS();
        _requestKeyFrame = false;
      }
      pTemp = _pOutStreams;
      while (pTemp != NULL) {
        switch(pTemp->info->GetType()){
          case ST_OUT_NET_RTP:
            if(!pTemp->info->FeedData(data.buffer+4, data.length-4, 0, data.length-4, _currentTS, false)){
              FATAL("Unable to feed OS: %p", pTemp->info);
            }
            break;
          case ST_OUT_NET_RTMP_4_RTMP:
            _dataBuffer.ReadFromBuffer(RTMPIFrameHeader.header, RTMPIFrameHeader.length);
            _dataBuffer.ReadFromBuffer(data.buffer, data.length);
            pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false);
            _dataBuffer.IgnoreAll();
            break;
          default:
            if(!pTemp->info->FeedData(data.buffer, data.length, 0, data.length, _currentTS, false)){
              FATAL("Unable to feed OS: %p", pTemp->info);
            }
            break;
        } //switch
        pTemp = pTemp->pPrev;
      } // while
      break;
    case 1:
      pTemp = _pOutStreams;
      while (pTemp != NULL) {
        switch(pTemp->info->GetType()){
          case ST_OUT_NET_RTP:
            // Ignore P frame in order to prevent incomplete sequence
            if(_requestKeyFrame){
               pTemp = pTemp->pPrev;
               continue;
            }
            if(!pTemp->info->FeedData(data.buffer+4, data.length-4, 0, data.length-4, _currentTS, false)){
              FATAL("Unable to feed OS: %p", pTemp->info);
            }
            break;
          case ST_OUT_NET_RTMP_4_RTMP:
            _dataBuffer.ReadFromBuffer(RTMPPFrameHeader.header, RTMPPFrameHeader.length);
            _dataBuffer.ReadFromBuffer(data.buffer, data.length);
            pTemp->info->FeedData(GETIBPOINTER(_dataBuffer), GETAVAILABLEBYTESCOUNT(_dataBuffer), 0, GETAVAILABLEBYTESCOUNT(_dataBuffer), _currentTS, false);
            _dataBuffer.IgnoreAll();
            break;
          default:
            if(!pTemp->info->FeedData(data.buffer, data.length, 0, data.length, _currentTS, false)){
              FATAL("Unable to feed OS: %p", pTemp->info);
            }
            break;
        } // end switch
        pTemp = pTemp->pPrev;
      } // while
      break;

    case 7: // SPS
      if(!_isInitVideo){
        INFO("init avmux sps/pps");
        // Update timestamp
        spsLength = data.buffer[3];
        INFO("%x %x ", spsLength, data.buffer[spsLength+7]);
        _pStreamCapabilities->InitVideoH264(data.buffer+4, spsLength, data.buffer+spsLength+8, data.buffer[spsLength+7]);
        _isInitVideo = true;
        ResendSPSPPS();
      }
      break;
    default:
      break;
  } // nalu type switch
*/
}

/* FIXME(Recardo)-use a mp4 document builder class to do this */
bool BaseTSMuxOutStream::BuildTSDocument(string filename,
                                           uint64_t creationTime,
                                           uint32_t maxFrames) {
  /*bool ret=true;
  Variant emptyVar;
  BaseAtom *pAtomFTYP, *pAtomMVHD, *pAtomTRAK, *pAtomMOOV;
  BaseAtom *pAtomTKHD, *pAtomMDHD, *pAtomHDLR, *pAtomMINF;
  BaseAtom *pAtomVMHD, *pAtomDINF, *pAtomAVC1, *pAtomAVCC;
  BaseAtom *pAtomSTBL, *pAtomSTSD, *pAtomSTSC, *pAtomSTTS, *pAtomSTSS, *pAtomSTSZ;
  BaseAtom *pAtomDREF, *pAtomURL, *pAtomMDIA;
  BaseAtom *pAtomMDAT, *pAtomDUMY;
  BaseAtom *pAtomSTCO;
  uint32_t timeScale=1000;
  uint32_t trackId=1;

  _mp4file = new WriteMP4Document(emptyVar);
  ret=_mp4file->Open(filename);
  if (!ret) {
    DEBUG ("open mp4 file fail");
    return ret;
  }

  vector<uint32_t> brands;
  brands.push_back(MAKE32_TAG4('m','p','4','1'));
  brands.push_back(MAKE32_TAG4('a','v','c','1'));
  brands.push_back(MAKE32_TAG4('q','t',' ',' '));
  pAtomFTYP=new AtomFTYP(_mp4file, MAKE32_TAG4('i','s','o','m'), 0x0, brands);
  pAtomFTYP=pAtomFTYP;
  pAtomMOOV=new AtomMOOV(_mp4file, A_MOOV, 0, 0);

  //Parent Moov
  pAtomMVHD=new AtomMVHD(_mp4file, creationTime, 1000, trackId+1);
  pAtomMVHD->SetParentAtom(pAtomMOOV);
  pAtomTRAK=new AtomTRAK(_mp4file, A_TRAK, 0, 0);
  pAtomTRAK->SetParentAtom(pAtomMOOV);

  //Parent TRAK
  pAtomTKHD=new AtomTKHD(_mp4file, creationTime, trackId,
      _pVideoStream->GetCapabilities()->avc._width,
      _pVideoStream->GetCapabilities()->avc._height);
  pAtomTKHD->SetParentAtom(pAtomTRAK);
  pAtomMDIA=new AtomMDIA(_mp4file, A_MDIA, 0, 0);
  pAtomMDIA->SetParentAtom(pAtomTRAK);

  //Parent MDIA
  pAtomMDHD=new AtomMDHD(_mp4file, creationTime, timeScale);
  pAtomMDHD->SetParentAtom(pAtomMDIA);
  pAtomHDLR=new AtomHDLR(_mp4file, 0x0, MAKE32_TAG4('v','i','d','e'), "VideoHandler ");
  pAtomHDLR->SetParentAtom(pAtomMDIA);
  pAtomMINF=new AtomMINF(_mp4file, A_MINF, 0, 0);
  pAtomMINF->SetParentAtom(pAtomMDIA);

  //Parent MINF, pAtom2 is MINF
  pAtomVMHD=new AtomVMHD(_mp4file, A_VMHD, 0, 0);
  pAtomVMHD->SetParentAtom(pAtomMINF);
  pAtomDINF=new AtomDINF(_mp4file, A_DINF, 0, 0);
  pAtomDINF->SetParentAtom(pAtomMINF);

  //Parent DINF, pAtom is DINF
  pAtomDREF=new AtomDREF(_mp4file, A_DREF, 0, 0);
  pAtomDREF->SetParentAtom(pAtomDINF);
  //Parent DREF, pAtom3 is DREF
  pAtomURL=new AtomURL(_mp4file, A_URL, 0, 0);
  pAtomURL->SetParentAtom(pAtomDREF);

  //Parent MINF, pAtom2 is MINF
  pAtomSTBL=new AtomSTBL(_mp4file, A_STBL, 0, 0);
  pAtomSTBL->SetParentAtom(pAtomMINF);

  //Parent STBL, pAtom is STBL
  pAtomSTSD=new AtomSTSD(_mp4file, A_STSD, 0, 0);
  pAtomSTSD->SetParentAtom(pAtomSTBL);
*/
  //Parent STSD, pAtom2 is STSD
  //pAtomAVC1=new AtomAVC1(_mp4file,
  //    _pVideoStream->GetCapabilities()->avc._width,
  //    _pVideoStream->GetCapabilities()->avc._height,
  //    1/*frame count*/, 24/*depth*/);
  /*pAtomAVC1->SetParentAtom(pAtomSTSD);

  //Parent AVC1, pAtom is AVC1
  pAtomAVCC=new AtomAVCC(_mp4file, (uint8_t)0x42, (uint8_t)0xe0, (uint8_t)0x1f);
  pAtomAVCC->SetParentAtom(pAtomAVC1);
  reinterpret_cast<AtomAVCC*>(pAtomAVCC)->SetSPSParam( _pVideoStream->GetCapabilities()->avc._spsLength, _pVideoStream->GetCapabilities()->avc._pSPS);
  reinterpret_cast<AtomAVCC*>(pAtomAVCC)->SetPPSParam( _pVideoStream->GetCapabilities()->avc._ppsLength, _pVideoStream->GetCapabilities()->avc._pPPS);

  //Parent STBL
  pAtomSTSC=new AtomSTSC(_mp4file, 1, 1);
  pAtomSTSC->SetParentAtom(pAtomSTBL);
  pAtomSTCO=new AtomSTCO(_mp4file, A_STCO, 0, 0);
  pAtomSTCO->SetParentAtom(pAtomSTBL);
  //push a entry
  reinterpret_cast<AtomSTCO*>(pAtomSTCO)->AddEntries(0x00);


  //reserve file size
  pAtomSTTS=new AtomSTTS(_mp4file, A_STTS, 0, 0);
  pAtomSTTS->SetParentAtom(pAtomSTBL);
  pAtomDUMY=new AtomDUMY(_mp4file, 8, A_STTS);
  pAtomDUMY->SetParentAtom(pAtomSTBL);

  pAtomSTSS=new AtomSTSS(_mp4file, A_STSS, 0, 0);
  pAtomSTSS->SetParentAtom(pAtomSTBL);
  pAtomDUMY=new AtomDUMY(_mp4file, 4, A_STSS);
  pAtomDUMY->SetParentAtom(pAtomSTBL);

  pAtomSTSZ=new AtomSTSZ(_mp4file, A_STSZ, 0, 0);
  pAtomSTSZ->SetParentAtom(pAtomSTBL);
  pAtomDUMY=new AtomDUMY(_mp4file, 4, A_STSZ);
  pAtomDUMY->SetParentAtom(pAtomSTBL);

  pAtomMDAT=new AtomMDAT(_mp4file, A_MDAT, 0, 0);
  pAtomMDAT=pAtomMDAT;
  ret=_mp4file->SerializeToFile(0, maxFrames);
  if (!ret) {
    WARN ("SerializeToFile fail");
    return ret;
  }
  ret=reinterpret_cast<AtomSTCO*>(pAtomSTCO)->UpdateChunkOffset(_mp4file->Size());
  if (!ret) {
    WARN ("Updatee STCO fail");
    return ret;
  }
*/
  return true;
}

bool BaseTSMuxOutStream::StartRecord(string fileName, uint32_t maxFrames) {
  bool ret=true;
  uint64_t timeStamp=(uint64_t)time(NULL);

  if (IsRecording()) {
    WARN("mp4 mux stream already start");
    return false;
  }
  //_curfileName=_tmpPath+SystemManager::GetCUID()+"_"+numToString<uint64_t>(timeStamp, 0);
  _muxState=MUX_START_RECORD;
  //_videoInfo.clear();
  ret=BuildTSDocument(fileName, timeStamp, maxFrames);
  _duration = _videoTimeStamp = _audioTimeStamp = _frameCounter = 0;
  _updated=false;
  _maxFrames=maxFrames;

  if (!ret) {
    WARN ("build mp4 document fail, StopRecord");
    _muxState=MUX_OPEN_FILE_ERR;
    StopRecord();
  }
  return ret;
}

bool BaseTSMuxOutStream::StopRecord() {
  NYI;
  return false;
}
