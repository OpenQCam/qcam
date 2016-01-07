#include "mux/basemp4muxoutstream.h"
//thelib
#include "streaming/streamstypes.h"
#include "streaming/baseinstream.h"
#include "mediaformats/mp4/writemp4document.h"
#include "mediaformats/mp4/atomftyp.h"
#include "mediaformats/mp4/atommoov.h"
#include "mediaformats/mp4/atommvhd.h"
#include "mediaformats/mp4/atomtrak.h"
#include "mediaformats/mp4/atomtkhd.h"
#include "mediaformats/mp4/atommdia.h"
#include "mediaformats/mp4/atommdhd.h"
#include "mediaformats/mp4/atomhdlr.h"
#include "mediaformats/mp4/atomminf.h"
#include "mediaformats/mp4/atomvmhd.h"
#include "mediaformats/mp4/atomdinf.h"
#include "mediaformats/mp4/atomdref.h"
#include "mediaformats/mp4/atomurl.h"
#include "mediaformats/mp4/atomstbl.h"
#include "mediaformats/mp4/atomstsd.h"
#include "mediaformats/mp4/atomavc1.h"
#include "mediaformats/mp4/atomavcc.h"
#include "mediaformats/mp4/atomstsc.h"
#include "mediaformats/mp4/atomstco.h"
#include "mediaformats/mp4/atomstts.h"
#include "mediaformats/mp4/atomstss.h"
#include "mediaformats/mp4/atomstsz.h"
#include "mediaformats/mp4/atommdat.h"
#include "mediaformats/mp4/atomdumy.h"
#include "system/nvramdefine.h"
//hardware
#include "basehardwareabstractdevice.h"
#include "hardwaremanager.h"
#include "storage/basestoragedevice.h"
#include "system/systemmanager.h"

BaseMP4MuxOutStream::BaseMP4MuxOutStream(StreamsManager *pStreamsManager, string name)
  : BaseOutStream (NULL, pStreamsManager, ST_OUT_FILE_MP4, name),
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
  _firstFrameTime=_lastFrameTime=0;
  _mp4file=NULL;
  _pStorageDevice=reinterpret_cast<BaseStorageDevice*>(HardwareManager::GetHardwareInstance(HT_STORAGE));
}

BaseMP4MuxOutStream::~BaseMP4MuxOutStream() {
  _pVideoStream = NULL;
  _pAudioStream = NULL;
  if (_mp4file) {
    _mp4file->Close();
    delete _mp4file;
  }
}

bool BaseMP4MuxOutStream::Initialize() {
  return true;
};

bool BaseMP4MuxOutStream::SignalPause() {
  NYIR;
}

bool BaseMP4MuxOutStream::SignalResume() {
  NYIR;
}

bool BaseMP4MuxOutStream::SignalSeek(double &absoluteTimestamp) {
  NYIR;
}

void BaseMP4MuxOutStream::SignalAttachedToInStream() {
}

void BaseMP4MuxOutStream::SignalDetachedFromInStream() {
}

void BaseMP4MuxOutStream::SignalStreamCompleted() {
}

bool BaseMP4MuxOutStream::IsCompatibleWithType(uint64_t type) {
  return true;;
}

void BaseMP4MuxOutStream::RegisterStream(BaseInStream *inStream) {
  bool reverseLink=false;
  if (!inStream) {
    WARN ("Register a NULL stream");
    return;
  }

  if (inStream->GetType() == ST_IN_CAM_AVC) {
    _pVideoStream = inStream;
    _status |= MP4MUX_REG_VIDEO;
    reverseLink=true;
  }
  else if (inStream->GetType() == ST_IN_AUDIO_MP3) {
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
  }
  else {
    WARN ("Unsupported instream type:%llx", inStream->GetType()) ;
    return;
  }

  inStream->Link(reinterpret_cast<BaseOutStream *>(this), reverseLink);
}

bool BaseMP4MuxOutStream::IsRecording() {
  if (_muxState!=MUX_STOP_RECORD)
    return true;
  return false;
}

bool BaseMP4MuxOutStream::IsError() {
  bool ret;
  ret=(_muxState!=MUX_STOP_RECORD)&&(_muxState!=MUX_START_RECORD);
  return ret;
};

uint8_t BaseMP4MuxOutStream::GetStatus() {
  return _status;
}

void BaseMP4MuxOutStream::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                        uint32_t dataLength) {
  NYI;
}

void BaseMP4MuxOutStream::UnRegisterAllStream() {

  /*Same behavior of StopRecord function in  record actionhandler */
  reinterpret_cast<BaseHardwareAbstractDevice *>(HardwareManager::GetHardwareInstance(HT_VIDEO_AVC))
    ->UnRegisterObserver((IDeviceObserver *)this);
  if (_pVideoStream)
    UnRegisterStream(_pVideoStream);
  if (_pAudioStream)
    UnRegisterStream(_pAudioStream);
}

void BaseMP4MuxOutStream::UnRegisterStream(BaseInStream *inStream) {
 if (!inStream) {
    WARN ("UnRegister a NULL stream");
    return;
  }

  inStream->UnLink(reinterpret_cast<BaseOutStream *>(this), true);
  if (inStream->GetType() == ST_IN_CAM_AVC) {
    _pVideoStream = NULL;
    _status &= ~MP4MUX_REG_VIDEO;
  }
  else if (inStream->GetType() == ST_IN_AUDIO_MP3) {
    _pAudioStream = NULL;
    _status &= ~MP4MUX_REG_AUDIO;
  }
  else if (inStream->GetType() == ST_IN_AUDIO_AAC) {
    _pAudioStream = NULL;
    _status &= ~MP4MUX_REG_AUDIO;
  }
  else
    return;
}

bool BaseMP4MuxOutStream::UpdateInfo(double currentTime) {

  if (!_mp4file) {
    return false;
  }

  if (_videoInfo.size() >= 2) {
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
  }
  return true;
}

bool BaseMP4MuxOutStream::FeedData (uint8_t *pData, uint32_t dataLength,
                                    uint32_t processedLength, uint32_t totalLength,
                                    double absoluteTimestamp, bool isAudio) {
  bool ret=true;

  if (!IsRecording()||IsError())
    return true;

  if (_frameCounter>_maxFrames) {
    DEBUG ("max frames reached");
    StopRecord();
  }

  if (isAudio) {
  }
  else { //Video
    AtomUpdateInfo videoInfo;

    if ((*(pData+4)&0x1f)!=5) {//not keyframe
      videoInfo.isKeyFrame=false;
      if (!_frameCounter) {
        WARN ("Wait first keyframe....");
        return true;
      }
    }
    else
      videoInfo.isKeyFrame=true;

    //valid frame
    _frameCounter++;
    if (_frameCounter==1) {
      _firstFrameTime=absoluteTimestamp;
    }
    videoInfo.frameIndex=_frameCounter;
    videoInfo.size = dataLength;
    videoInfo.timeStamp = absoluteTimestamp;
    _videoInfo.push_back(videoInfo);
    _lastFrameTime = absoluteTimestamp;
    _duration = (uint64_t)(_lastFrameTime-_firstFrameTime);

    if (_mp4file) {
      ret=_mp4file->UpdateMediaData (pData, dataLength);
      if (!ret) {
        _muxState = MUX_UPDATE_MEDIA_ERR;
        return ret;
      }
    }

    double currentTime=time(NULL);
    if ((currentTime-_videoTimeStamp)>1) {
      ret=UpdateInfo(currentTime);
      if (!ret) {
        _muxState = MUX_UPDATE_HEADER_ERR;
      }
    }//update
 } //video

  return ret;
}

/* FIXME(Recardo)-use a mp4 document builder class to do this */
bool BaseMP4MuxOutStream::BuildMP4Document(string filename,
                                           uint64_t creationTime,
                                           uint32_t maxFrames) {
  bool ret=true;
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

  //Parent STSD, pAtom2 is STSD
  pAtomAVC1=new AtomAVC1(_mp4file,
      _pVideoStream->GetCapabilities()->avc._width,
      _pVideoStream->GetCapabilities()->avc._height,
      1/*frame count*/, 24/*depth*/);
  pAtomAVC1->SetParentAtom(pAtomSTSD);

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

  return ret;
}

bool BaseMP4MuxOutStream::StartRecord(string fileName, uint32_t maxFrames) {
  bool ret=true;
  uint64_t timeStamp=(uint64_t)time(NULL);

  if (IsRecording()) {
    WARN("mp4 mux stream already start");
    return false;
  }
  //_curfileName=_tmpPath+SystemManager::GetCUID()+"_"+numToString<uint64_t>(timeStamp, 0);
  _muxState=MUX_START_RECORD;
  _videoInfo.clear();
  ret=BuildMP4Document(fileName, timeStamp, maxFrames);
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

bool BaseMP4MuxOutStream::StopRecord() {
  NYI;
  return false;
}
