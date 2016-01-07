/* ==================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 * (C) COPYRIGHT Quanta Computer Inc. ALL RIGHTS RESERVED
 *
 * ==================================================================
 * ------------------------------------------------------------------
 *  Date    :
 *  Version :
 *  Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 *  Purpose :
 * ==================================================================
 */

#ifdef QIC1816

#include "qic/qicavcvideocapture.h"
#include "qic/qicavcstream.h"
#include "streaming/streamsmanager.h"
#include "video/basevideostream.h"
#include "video/devicevideostream.h"

BaseHardwareAbstractDevice* QICAVCVideoCapture::_pDeviceInstance = NULL;

BaseHardwareAbstractDevice* QICAVCVideoCapture::GetInstance(string devNode) {
  if(_pDeviceInstance == NULL){
    int32_t fd;
    if((fd = open(STR(devNode), O_RDWR | O_NONBLOCK, 0)) < 0) {
      FATAL("Cannot open device %s", STR(devNode));
      return NULL;
    }
    _pDeviceInstance = new QICAVCVideoCapture(fd);
    if(!_pDeviceInstance->InitializeDevice()){
      delete _pDeviceInstance;
      return NULL;
    }
  }
  return _pDeviceInstance;
}

QICAVCVideoCapture::QICAVCVideoCapture(int32_t fd)
  : BaseAVCVideoCapture(HT_VIDEO_AVC, fd)
{
}

QICAVCVideoCapture::~QICAVCVideoCapture() {
  // 1. stop capturing
  StopCapture();

  // 2. clean up buffer
  UnInitMmap();

  // 3. close device
  Close();

  // 4. clean up everything we created
  if(_pVideoCapability != NULL)
    delete _pVideoCapability;
}

bool QICAVCVideoCapture::InitializeDevice() {

  _status = VCS_IDLE;
  // AVC initialization
  QicSetDeviceHandle(_deviceFD); // essential and very very early stage

  _pVideoCapability = new VideoCaptureCapability();

  // 2. query capability
  if(!v4l2_querycap(_pVideoCapability->V4L2Capability)){
    return false;
  }

  // 3. set resolution
  _pVideoCapability->width = 1280;
  _pVideoCapability->height = 720;
  _pVideoCapability->fps = 30;
  _pVideoCapability->gop = 30;
  _pVideoCapability->bitrate = 2000000;
  _pVideoCapability->videoCodec = VC_AVC;
  _pVideoCapability->format = FORMAT_STREAM_H264_RAW_3;
  _pVideoCapability->sizeFormat = FORMAT_STREAM_H264_ADD_PADDING;

  if(!v4l2_s_fmt(_pVideoCapability->width, _pVideoCapability->height, V4L2_PIX_FMT_MPEG)){
    FATAL("Set format failed");
    return false;
  }

  // 4. set avc
  EncoderParams_t param;
  CLEAR(param);
  param.ucFramerate = _pVideoCapability->fps;
  param.uiBitrate = _pVideoCapability->bitrate;
  param.usWidth = _pVideoCapability->width;
  param.usHeight = _pVideoCapability->height;
  param.ucSlicesize = 0;

  if(QicEncoderSetParams(&param, QIC_XU1_ENCODER_CONFIG_ALL) < 0){
    FATAL("Config encoder param failed camera fd:%d", _deviceFD);
    return false;
  }
  QicEncoderSetGOP (_pVideoCapability->gop);
  QicSetStreamFormat(_pVideoCapability->format);
  QicSetEncoderFrameSizeFormat(_pVideoCapability->sizeFormat);

  if(!InitMmap()){
    FATAL("Init mmap failed");
    return false;
  }

  INFO("Init AVC video capture done. fd = %d", _deviceFD);
  //FirmwareVersion_t fw_version;
  //if(QicGetFirmwareVersion (&fw_version) < 0){
  //  FATAL("Get Firmware version failed");
  //  return false;
  //}
  //_pCamInfo->usb.pid = strdup((char *)fw_version.szPID);
  //_pCamInfo->usb.vid = strdup((char *)fw_version.szVID);
  //_pCamInfo->fw_version = strdup((char *)fw_version.szREV);

  return true;
}



bool QICAVCVideoCapture::OnReadEvent() {
  struct v4l2_buffer buf;
  if(!v4l2_dqbuf(buf)){
    return false;
  }
  if(buf.index < _numOfV4L2Buffers && buf.index >=0){
    SendDataToCarriers((uint8_t *)_pV4L2Buffers[buf.index].start, buf.bytesused, 0);
    if(!v4l2_qbuf(buf)){
      FATAL("Unable to queue buffer");
    }
  }
  return true;
}

//BaseVideoStream* QICAVCVideoCapture::CreateVideoStream(StreamsManager *pSM, string streamName, uint16_t deviceStreamId) {
uint8_t QICAVCVideoCapture::CreateVideoStreams(StreamsManager *pSM, string streamName) {
  // 1. create video stream for application
  // video stream is created here but deleting in BaseV4L2VideoCapture because video stream is relevant to hardware device
  BaseVideoStream *pStream = (BaseVideoStream*) new QICAVCStream(pSM, streamName);
  // 2. register video stream as carrier for receiving data
  RegisterCarrier((IDeviceCarrier *)pStream);
  // 3. register video stream as observer for obtaining notification
  RegisterObserver((IDeviceObserver *)pStream);
  // 4. create device stream as video source
  DeviceVideoStream *pDeviceStream = new DeviceVideoStream(pStream);
  // 5. bind video stream and device stream together
  //FIXME(Tim): How to get device stream id?
  _deviceVideoStreams.insert(pair<uint16_t, DeviceVideoStream*>(0, pDeviceStream));
  // 6. last device stream as default current stream
  _pCurrentDeviceStream.insert(pair<uint16_t, DeviceVideoStream*>(0, pDeviceStream));
  // return number of streams
  return 1;
}


bool QICAVCVideoCapture::StartCapture() {
  INFO("Start video capture");

  if(_status == VCS_ON){
    INFO("video was already started");
    return true;
  }

  if(_deviceFD < 0){
    FATAL("Device is not open yet");
    return false;
  }

  //re-queue buffer
  struct v4l2_buffer buf;
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  for(uint32_t i=0; i<_numOfV4L2Buffers; ++i){
    buf.index = i;
    if(!v4l2_qbuf(buf)){
      FATAL("Unable to queue buffers");
      return false;
    }
  }

  if(!v4l2_streamon()){
    FATAL("Unable to turn on stream");
    return false;
  }else{
    ENABLE_DEVICE_READ_DATA;
    SetStatus(VCS_ON);
  }
  return true;
}

bool QICAVCVideoCapture::StopCapture() {
  if(_status != VCS_ON){
    INFO("video was already stopped");
    return true;
  }
  if(!v4l2_streamoff()){
    FATAL("Unable to turn off stream");
    return false;
  }
  DISABLE_DEVICE_READ_DATA;
  SetStatus(VCS_OFF);

  return true;
}

bool QICAVCVideoCapture::SetResolution(uint32_t width, uint32_t height) {

  bool restart = false;
  if(_deviceFD < 0 ) return false;

  if(_pVideoCapability->width == width && _pVideoCapability->height == height ){
    INFO("Resolution %dx%d is the same", width, height);
    return true;
  }

  if(IsStarted()){
    if(!StopCapture()){
      return false;
    }
    // restart after changing
    restart = true;
  }

  //unmap memory fist
  if(!UnInitMmap()) return false;

  if(!v4l2_s_fmt(width, height, V4L2_PIX_FMT_MPEG)){
    FATAL("Unable to set format");
    return false;
  }

  if(!InitMmap()) return false;

  _pVideoCapability->width = width;
  _pVideoCapability->height = height;

  if(restart){
    if(!StartCapture()){
      FATAL("Unable to restart");
      return false;
    }
  }
  NotifyObservers(VCM_RESOLUTION_CHANGED, format("resolution changeed (%dx%d)", width, height));
  return true;
}


bool QICAVCVideoCapture::SetFrameRate(uint32_t fps) {
  EncoderParams_t param;
  CLEAR(param);
  QicChangeFD(_deviceFD);

  /*if(_pCamInfo->video1.width == CAMERA_MAX_PREVIEW_WIDTH &&
     _pCamInfo->video1.height == CAMERA_MAX_PREVIEW_HEIGHT &&
     framerate > CAMERA_MAX_HD_FRAMERATE){
    FATAL("Invalid framerate %d at resolution %dx%d", framerate, _pCamInfo->video1.width, _pCamInfo->video1.height);
    return 0;
  }*/

  QicEncoderGetParams(&param);
  param.ucFramerate = fps;
  if(QicEncoderSetParams(&param, QIC_XU1_ENCODER_FRAMERATE)){
    FATAL("QicEncoderSetParams set framerate failed %d", fps);
    return false;
  }

  _pVideoCapability->fps = fps;
  NotifyObservers(VCM_FPS_CHANGED, format("fps changed %d", fps));
  return true;
}


bool QICAVCVideoCapture::SetBitRate(uint32_t bitrate) {

  EncoderParams_t param;
  CLEAR(param);
  QicChangeFD(_deviceFD);
  QicEncoderGetParams(&param);
  param.uiBitrate = bitrate;
  if(QicEncoderSetParams (&param, QIC_XU1_ENCODER_BITRATE)){
    FATAL("Set bitrate %d is failed", bitrate);
    return false;
  }

  _pVideoCapability->bitrate = bitrate;
  return true;
}

uint32_t QICAVCVideoCapture::GetBitRate() {
  return _pVideoCapability->bitrate;
}

//uint8_t SetSliceSize(uint8_t slicesize);
//uint8_t GetSliceSize();

bool QICAVCVideoCapture::SetGOP(uint32_t gop) {

  if(QicEncoderSetGOP(gop)){
    FATAL("Set GOP %d failed", gop);
    return false;
  }
  _pVideoCapability->gop = gop;
  return true;
}

uint32_t QICAVCVideoCapture::GetGOP() {
  return _pVideoCapability->gop;
}

bool QICAVCVideoCapture::GetKeyFrame() {
  QicEncoderSetIFrame();
  NotifyObservers(VCM_KEYFRAME_DONE, "request keyframe done");
  return true;
}

void QICAVCVideoCapture::GetFirmwareVersion(string &version) {
}

uint32_t QICAVCVideoCapture::GetNumOfProfiles() {
  int32_t num;
  QicChangeFD(_deviceFD);
   if(QicEncoderGetNumberOfProfiles(&num) < 0){
    FATAL("Get num of profiles failed");
    return -1;
  }
  return num;
}

bool QICAVCVideoCapture::GetProfile(int idx, int *level, int *profile, int *constraint_flags) {
  QicChangeFD(_deviceFD);
  if(QicEncoderGetProfile(idx, level, profile, constraint_flags) < 0){
    FATAL("Get profile failed");
    return false;
  }
  return true;

}

bool QICAVCVideoCapture::SetProfileLevel(int level, int profile, int constraint_flags) {
  bool restart = false;

  if(IsStarted()){
    if(!StopCapture()) return false;
    restart = true;
  }

  QicChangeFD(_deviceFD);
  if( QicEncoderSetProfileAndLevel(level, profile, constraint_flags) < 0){
    FATAL("Set profile level failed");
    return false;
  }
  if(restart)
    if(StartCapture()) return false;

  NotifyObservers(VCM_PROFILE_CHANGED, "set profile level done");
  return true;
}

bool QICAVCVideoCapture::GetProfileLevel(int *level, int *profile, int *constraint_flags) {
  QicChangeFD(_deviceFD);
  if( QicEncoderGetProfileAndLevel(level, profile, constraint_flags) < 0){
    FATAL("Get profile level failed");
    return false;
  }
  return true;
}


bool QICAVCVideoCapture::SetPanTilt(int16_t pan, int16_t tilt) {
  int16_t gotPan;
  int16_t gotTilt;
  if(10 <pan||-10> pan)
  {
    FATAL("Pan out of range!!");
    return -1;
  }

  if(10 <tilt||-10> tilt)
  {
    FATAL("Tilt out of range!!");
    return -1;
  }
  if(pan<0)
  {
    pan=(~pan)+0x8001;
  }
  if(tilt<0)
  {
    tilt=(~tilt)+0x8001;
  }
  QicChangeFD(_deviceFD);
  if(1 == QicSetPanTilt(pan,tilt)){
    FATAL("Setting Pan and Tilt failed!");
    return -1;
  }
  QicGetPanTilt(&gotPan, &gotTilt);
  _pVideoCapability->pan = gotPan;
  _pVideoCapability->tilt = gotTilt;
  //FATAL("In QICHWCamera Pan : %hd\tTilt : %hd", gotPan, gotTilt);
  return 0;

}

void QICAVCVideoCapture::GetPanTilt(int16_t &pan, int16_t &tilt) {
  pan = _pVideoCapability->pan;
  tilt = _pVideoCapability->tilt;
}

#endif
