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

#include "qic/qicmjpgvideocapture.h"
#include "qic/qicmjpgstream.h"
#include "video/basevideostream.h"
#include "video/devicevideostream.h"

BaseHardwareAbstractDevice* QICMJPGVideoCapture::_pDeviceInstance = NULL;

//static member function
BaseHardwareAbstractDevice* QICMJPGVideoCapture::GetInstance(string devNode) {
  if(_pDeviceInstance == NULL){
    int32_t fd;
    if((fd = open(STR(devNode), O_RDWR | O_NONBLOCK, 0)) < 0) {
      FATAL("Cannot open device %s", STR(devNode));
      return NULL;
    }
    _pDeviceInstance = (BaseHardwareAbstractDevice *) new QICMJPGVideoCapture(fd);
    if(!_pDeviceInstance->InitializeDevice()){
      delete _pDeviceInstance;
      return NULL;
    }
  }
  return _pDeviceInstance;
}

QICMJPGVideoCapture::QICMJPGVideoCapture(int32_t fd)
: BaseMJPGVideoCapture(HT_VIDEO_MJPG, fd),
  _fcnt(0),
  _lfcnt(0)
//jennifer
//: BaseV4L2VideoCapture(HT_VIDEO_MJPG, fd)
{
  _isSetFormat = false;
}

QICMJPGVideoCapture::~QICMJPGVideoCapture() {
  Close();
}

bool QICMJPGVideoCapture::Close() {
  // 1. stop capturing
  StopCapture();

  // 2. clean up buffer
  UnInitMmap();

  // 3. close device
  return BaseV4L2VideoCapture::Close();

  //if(_pCarrier != NULL)
  //  delete _pCarrier;

}

bool QICMJPGVideoCapture::InitializeDevice() {

  if(_deviceFD < 0 ) return false;

  _status = VCS_IDLE;

  QicSetDeviceHandle(_deviceFD);

  // 2. query capability
  struct v4l2_capability V4L2Capability;
  if(!v4l2_querycap(V4L2Capability)){
    return false;
  }

  // 3. set resolution
#if defined(__CL1830__)
  if(!v4l2_s_fmt(1280, 720, V4L2_PIX_FMT_MJPEG)){
    return false;
  }
  if(!InitMmap()) return false;
#endif

  INFO("Init MJPG video capture done. fd = %d", _deviceFD);
  _isSetFormat = true;
  return true;
}

bool QICMJPGVideoCapture::OnReadEvent(){
  struct v4l2_buffer buf = {0};
  // read data from driver and send to carrier
  //DEBUG("QICMJPGVideoCapture::OnReadEvent");
  if(!v4l2_dqbuf(buf)){
    DEBUG ("dequeue mjpg fails");
    HardwareManager::SetStatus(EC_VIDEO_MJPG, true);
    HardwareManager::SetStatus(EC_VIDEO_AVC, true);
    return false;
  }
  if(buf.index < _reqBufferCount && buf.index >=0){
    double ts;
    _fcnt++;
    GETTIMESTAMP(ts);
    SendDataToCarriers((uint8_t *)_pV4L2Buffers[buf.index].start, buf.bytesused, ts);
    if(!v4l2_qbuf(buf)){
      FATAL("Unable to queue buffer");
    }
  }

  return true;
}

uint8_t QICMJPGVideoCapture::CreateVideoStreams(StreamsManager *pSM, string streamName) {
  // 1. create video stream for application
  // video stream is created here but deleting in BaseV4L2VideoCapture because video stream is relevant to hardware device
  BaseVideoStream *pStream = (BaseVideoStream*) new QICMJPGStream(pSM, streamName);
  // 2. register video stream as carrier for receiving data
  RegisterCarrier((IDeviceCarrier *)pStream);
  // 3. register video stream as observer for obtaining notification
  RegisterObserver((IDeviceObserver *)pStream);
  // 4. create device stream as video source
  DeviceVideoStream *pDeviceStream = new DeviceVideoStream(pStream);
  // 5. Initialize video capability
  // There is ONLY ONE device video stream for MJPG
  // Just pick up the first constraint for initializing
  if(!pDeviceStream->GetCapability()->InitVideoCapability(0,false)) return false;
  //jennifer(TODO) : Remove this
      //pDeviceStream->GetCapability()->PrintDeviceVideoConstraint();
  // 6. bind video stream and device stream together
  _deviceVideoStreams.insert(pair<uint16_t, DeviceVideoStream*>(0, pDeviceStream));
  // 7. last device stream as default current stream
  _pCurrentDeviceStream.insert(pair<uint16_t, DeviceVideoStream*>(0, pDeviceStream));
  // 8. Customize video configuration according to NVRAM
  if(!CustomizeVideoSettings()){
    FATAL("Customizing video setting failed");
    return 0;
  }
  // return number of streams
  return 1;
}

bool QICMJPGVideoCapture::CustomizeVideoSettings(){
  uint32_t widthNo;

  // Just retrieve the value and store back as the configuration
  string temp = SystemManager::GetNVRam("Device_Video_MJPG_Width");
  if((temp != "") && isNumeric(temp)){
    // Temporarily keep stream width for resolution setting
    widthNo = stringToNum<uint32_t>(temp);
    temp = SystemManager::GetNVRam("Device_Video_MJPG_Height");
    if((temp != "") && isNumeric(temp)){
      if(!SetResolution(widthNo, stringToNum<uint32_t>(temp)))  return false;
    }
  }
  else{
    Resolution resolutionConstraint[AVAIL_RESOLUTION_SET];
    MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->GetResolutionConstraints(resolutionConstraint);
    //FATAL("Current device video stream height : %d", resolutionConstraint[0].height);
    //FATAL("Current device video stream width : %d", resolutionConstraint[0].width);
    if(!SetResolution(resolutionConstraint[0].width, resolutionConstraint[0].height))  return false;
    INFO("Setting about MJPG resolution in NVRAM is empty or invalid! Set to default resolution.");
  }

  return true;
}

bool QICMJPGVideoCapture::StartCapture() {
  INFO("Start MJPG video capture");

  if(_status == VCS_ON){
    INFO("video was already started");
    return true;
  }

  if(_deviceFD < 0){
    FATAL("Device is not open yet");
    return false;
  }
  _fcnt=_lfcnt=0;

  // Retrieve the stream configuration
  DeviceVideoStream *pDeviceStream = MAP_VAL(_pCurrentDeviceStream.begin());
  VideoConfig *pVideoConfig = pDeviceStream->GetConfig();
  // Below is for debug sake
  //pDeviceStream->PrintCurrentConfig();

#if ((defined(__x86__)) || (defined(__MT7620__)))
  if(!v4l2_s_fmt(pVideoConfig->width, pVideoConfig->height, V4L2_PIX_FMT_MJPEG)){
    FATAL("Unable to set format");
    return false;
  }

  if(!InitMmap()) return false;

#elif defined(__CL1830__)
  if(_isSetFormat){
    _isSetFormat = false;

    INFO("Already allocated for embedded system. Doing nothing here.");
  }
  else{
    if(!v4l2_s_fmt(pVideoConfig->width, pVideoConfig->height, V4L2_PIX_FMT_MJPEG)){
      FATAL("Unable to set format");
      return false;
    }
  }
#endif


  //re-queue buffer
  struct v4l2_buffer buf;
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  for(uint32_t i=0; i<_reqBufferCount; ++i){
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
bool QICMJPGVideoCapture::StopCapture() {

  if(_deviceFD < 0 ) return false;

  if(_status != VCS_ON){
    INFO("video was already stopped");
    return true;
  }
  if(!v4l2_streamoff()){
    FATAL("Unable to turn off stream");
    return false;
  }

  DISABLE_DEVICE_READ_DATA;

#if ((defined(__x86__)) || (defined(__MT7620__)))
  if(!UnInitMmap()) return false;
#endif

  SetStatus(VCS_OFF);
  INFO("Stop video");
  return true;
}

bool QICMJPGVideoCapture::VerifyResolution(uint16_t width, uint16_t height){
  return MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->VerifyResolution(width, height);
}

bool QICMJPGVideoCapture::SetResolution(uint32_t width, uint32_t height) {
  DeviceVideoStream *pDeviceStream = MAP_VAL(_pCurrentDeviceStream.begin());
  VideoConfig *pVideoConfig = pDeviceStream->GetConfig();

  bool restart = false;

  if(!MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->VerifyResolution(width, height))
    return false;

  if(pVideoConfig->width == width && pVideoConfig->height == height ){
    INFO("Resolution %dx%d is the same", width, height);
    return true;
  }

  if(_status == VCS_ON){
    if(!StopCapture()){
      return false;
    }
    // restart after changing
    restart = true;
  }
  // Set resolution
  if(!v4l2_s_fmt(width, height, V4L2_PIX_FMT_MJPEG)){
    FATAL("Unable to set format");
    return false;
  }
  // Get resolution
  // This is to get the actual video format from underlying device
  //if(!v4l2_g_fmt(V4L2_PIX_FMT_MJPEG)){
  //  FATAL("Unable to set format");
  //  return false;
  //}
  // Store back as current configuration
  pVideoConfig->width = width;
  pVideoConfig->height = height;

  if(restart){
    if(!StartCapture()){
      FATAL("Unable to restart");
      return false;
    }
  }

  NotifyObservers(VCM_RESOLUTION_CHANGED, format("resolution changeed (%dx%d)", width, height));
  return true;
}

bool QICMJPGVideoCapture::SetDefaultResolution(string width, string height) {
  if(!MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->VerifyResolution((int32_t)atoi(STR(width)), (int32_t)atoi(STR(height))))
    return false;

  if(!SystemManager::SetNVRam("Device_Video_MJPG_Width", width, false))
    return false;

  if(!SystemManager::SetNVRam("Device_Video_MJPG_Height", height, false))
    return false;

  return true;
}
Variant QICMJPGVideoCapture::GetDefaultResolution() {
  Variant resolution;
  resolution["width"] = stringToNum<uint32_t>(SystemManager::GetNVRam("Device_Video_MJPG_Width"));
  resolution["height"] = stringToNum<uint32_t>(SystemManager::GetNVRam("Device_Video_MJPG_Height"));
  return resolution;
}

bool QICMJPGVideoCapture::SetFrameRate(uint32_t fps) {
  /* Controlling fps of mjpg NEED to be supported by firmware.
  DeviceVideoStream *pDeviceStream = MAP_VAL(_pCurrentDeviceStream.begin());
  VideoConfig *pVideoConfig = pDeviceStream->GetConfig();

  if(pVideoConfig->fps == fps){
    INFO("Frame rate  %d is the same", fps);
    return true;
  }

  bool restart = false;

  if(_status == VCS_ON){
    if(!StopCapture()){
      return false;
    }
    // restart after changing
    restart = true;
  }

  if(!v4l2_s_fmt(pVideoConfig->width, pVideoConfig->height, V4L2_PIX_FMT_MJPEG)){
    FATAL("Unable to set format");
    return false;
  }

  if(!BaseV4L2VideoCapture::SetFrameRate(fps)) {
    return false;
  }

  pVideoConfig->fps = fps;

  if(restart){
    if(!StartCapture()){
      FATAL("Unable to restart");
      return false;
    }
  }

  NotifyObservers(VCM_FPS_CHANGED, format("set framerate %d", fps));
   */
  return true;
}

uint32_t QICMJPGVideoCapture::GetFrameRate() {
  return 0;
  // Controlling fps of mjpg NEED to be supported by firmware.
  //return MAP_VAL(_pCurrentDeviceStream.begin())->GetConfig()->fps;
}

Variant QICMJPGVideoCapture::GetConstraints(){
  return MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->GetConstraints();
}


void QICMJPGVideoCapture::CheckStatus(double ts) {
  if (_lfcnt==_fcnt && _fcnt>500 ) {
    //HardwareManager::SetStatus(EC_VIDEO_MJPG);
  }
  else {
    HardwareManager::ClearStatus(EC_VIDEO_MJPG);
    _lfcnt=_fcnt;
  }
}
