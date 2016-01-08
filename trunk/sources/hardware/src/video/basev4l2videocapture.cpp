/* ==================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 * (C) COPYRIGHT Quanta Computer Inc. ALL RIGHTS RESERVED
 *
 * ==================================================================
 * ------------------------------------------------------------------
 *  Date    : 2013/03/31
 *  Version :
 *  Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 *  Purpose : Video Capture Abstract Class
 *  =================================================================
 */

#include "video/basev4l2videocapture.h"
#include "video/basevideostream.h"
#include "video/devicevideostream.h"
#include <sys/ioctl.h>

VideoCaptureFilters* BaseV4L2VideoCapture::_pVideoCaptureFilters = NULL;
VideoCaptureFiltersCapability* BaseV4L2VideoCapture::_pVideoCaptureFiltersCapability = NULL;

BaseV4L2VideoCapture::BaseV4L2VideoCapture(HardwareType hwType, int32_t deviceFD)
: BaseHardwareAbstractDevice(hwType, deviceFD, IOHT_QIC_VIDEO)
  ,_pV4L2Buffers(0)
  ,_numOfV4L2Buffers(kDefaultBufferCount)
{
  _isInit = false;
}

BaseV4L2VideoCapture::~BaseV4L2VideoCapture() {

  // delete application stream and device stream
  FOR_MAP(_deviceVideoStreams, uint16_t, DeviceVideoStream *, i) {
    if(MAP_VAL(i) != NULL){
      DeviceVideoStream *pDeviceStream = MAP_VAL(i);
      if (pDeviceStream) {
        delete pDeviceStream->GetVideoStream();
      }
      delete pDeviceStream;
    }
  }
  if( _pVideoCaptureFilters != NULL ){
    delete _pVideoCaptureFilters;
    _pVideoCaptureFilters = NULL;
  }
  if( _pVideoCaptureFiltersCapability != NULL ){
    delete _pVideoCaptureFiltersCapability;
    _pVideoCaptureFiltersCapability = NULL;
  }
}

bool BaseV4L2VideoCapture::InitializeDevice() {
  Limitation brightness;
  Limitation contrast;
  Limitation hue;
  Limitation saturation;
  Limitation sharpness;
  Limitation gamma;
  Limitation backlightCompensation;

  // 1. Get constraint of each capture filters
  if(!v4l2_query_ctrl(V4L2_CID_BRIGHTNESS, &brightness))
    return false;
  if(!v4l2_query_ctrl(V4L2_CID_CONTRAST, &contrast))
    return false;
  if(!v4l2_query_ctrl(V4L2_CID_HUE, &hue))
    return false;
  if(!v4l2_query_ctrl(V4L2_CID_SATURATION, &saturation))
    return false;
  if(!v4l2_query_ctrl(V4L2_CID_SHARPNESS, &sharpness))
    return false;
  if(!v4l2_query_ctrl(V4L2_CID_GAMMA, &gamma))
    return false;
  if(!v4l2_query_ctrl(V4L2_CID_BACKLIGHT_COMPENSATION, &backlightCompensation))
    return false;

  // 2. Instantiate VideoCaptureFiltersCapability
  // TODO : polymorphic the constructor for different support
  _pVideoCaptureFiltersCapability = new VideoCaptureFiltersCapability(brightness, contrast, hue, saturation, sharpness, gamma, backlightCompensation);
  // Below is for debug sake
  //_pVideoCaptureFiltersCapability->PrintConstraintValues();

  // Set capture filters to default here if needed

  // 3. Initialize VideoCaptureFilters
  InitVideoCaptureFilters();

  return true;
}

bool BaseV4L2VideoCapture::InitVideoCaptureFilters() {
  if(_pVideoCaptureFilters == NULL)
    _pVideoCaptureFilters = new VideoCaptureFilters();

  _pVideoCaptureFilters->brightness = GetBrightness();
  _pVideoCaptureFilters->contrast = GetContrast();
  _pVideoCaptureFilters->hue = GetHUE();
  _pVideoCaptureFilters->saturation = GetSaturation();
  _pVideoCaptureFilters->sharpness = GetSharpness();
  _pVideoCaptureFilters->gamma = GetGamma();
  _pVideoCaptureFilters->backlightCompensation = GetBacklightCompensation();

  // Below is for debug sake
  //FATAL("brightness : %d\n contrast : %d\n hue : %d\n saturation : %d\n sharpness : %d\n gamma : %d\n backlightCompensation :%d", _pVideoCaptureFilters->brightness , _pVideoCaptureFilters->contrast, _pVideoCaptureFilters->hue, _pVideoCaptureFilters->saturation , _pVideoCaptureFilters->sharpness, _pVideoCaptureFilters->gamma, _pVideoCaptureFilters->backlightCompensation);
  return true;
}

bool BaseV4L2VideoCapture::SelectDeviceStream(string streamName) {
  FOR_MAP(_deviceVideoStreams, uint16_t, DeviceVideoStream *, i) {
    if(streamName.compare(MAP_VAL(i)->GetVideoStreamName()) == 0){
      _pCurrentDeviceStream.clear();
      _pCurrentDeviceStream.insert(pair<uint16_t, DeviceVideoStream*>(MAP_KEY(i), MAP_VAL(i)));
      return true;
    }
  }
  //if(MAP_HAS1(_deviceVideoStreams, deviceStreamId)){
  //  _pCurrentDeviceStream = MAP_VAL(_deviceVideoStreams.find(deviceStreamId));
  //  return true;
  //}
  WARN("Unable to find stream by name: %s", STR(streamName));
  return false;
}

DeviceVideoStream* BaseV4L2VideoCapture::GetCurrentDeviceStream() {
  return MAP_VAL(_pCurrentDeviceStream.begin());
}

uint8_t BaseV4L2VideoCapture::GetNumOfDeviceStreams() {
  return _deviceVideoStreams.size();
}


bool BaseV4L2VideoCapture::SetResolution(uint32_t width, uint32_t height) {
  return true;
}

uint32_t BaseV4L2VideoCapture::GetWidth() {
  return MAP_VAL(_pCurrentDeviceStream.begin())->GetConfig()->width;
}

uint32_t BaseV4L2VideoCapture::GetHeight() {
  return MAP_VAL(_pCurrentDeviceStream.begin())->GetConfig()->height;
}

bool BaseV4L2VideoCapture::SetFrameRate(uint32_t fps) {
  // Controlling fps of mjpg NEED to be supported by firmware.
  /*
  struct v4l2_streamparm setfps;

  CLEAR(setfps);

  setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  setfps.parm.capture.timeperframe.numerator = 1;
  setfps.parm.capture.timeperframe.denominator = fps;

  if (xioctl(VIDIOC_S_PARM, &setfps) < 0 ) {
    FATAL("Setting frame rate FAILED");
    FATAL("errno : %d, description : %s", errno, strerror(errno));
    return false;
  }
   */
  return true;
}

bool BaseV4L2VideoCapture::SetBrightness(int32_t value) {
  //return v4l2_s_ctrl(V4L2_CID_BRIGHTNESS, value);
  if(_pVideoCaptureFiltersCapability->VerifyBrightness(value)){
    if(v4l2_s_ctrl(V4L2_CID_BRIGHTNESS, value)){
      _pVideoCaptureFilters->brightness = GetBrightness();
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

int32_t BaseV4L2VideoCapture::GetBrightness() {
  return v4l2_g_ctrl(V4L2_CID_BRIGHTNESS);
}

bool BaseV4L2VideoCapture::SetContrast(int32_t value) {
  //return v4l2_s_ctrl(V4L2_CID_CONTRAST, value);
  if(_pVideoCaptureFiltersCapability->VerifyContrast(value)){
    if(v4l2_s_ctrl(V4L2_CID_CONTRAST, value)){
      _pVideoCaptureFilters->contrast = GetContrast();
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

int32_t BaseV4L2VideoCapture::GetContrast() {
  return v4l2_g_ctrl(V4L2_CID_CONTRAST);
}

bool BaseV4L2VideoCapture::SetHUE(int32_t value) {
  //return v4l2_s_ctrl(V4L2_CID_HUE, value);
  if(_pVideoCaptureFiltersCapability->VerifyHue(value)){
    if(v4l2_s_ctrl(V4L2_CID_HUE, value)){
      _pVideoCaptureFilters->hue = GetHUE();
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

int32_t BaseV4L2VideoCapture::GetHUE() {
  return v4l2_g_ctrl(V4L2_CID_HUE);
}

bool BaseV4L2VideoCapture::SetSaturation(int32_t value) {
  //return v4l2_s_ctrl(V4L2_CID_SATURATION, value);
  if(_pVideoCaptureFiltersCapability->VerifySaturation(value)){
    if(v4l2_s_ctrl(V4L2_CID_SATURATION, value)){
      _pVideoCaptureFilters->saturation = GetSaturation();
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

int32_t BaseV4L2VideoCapture::GetSaturation() {
  return v4l2_g_ctrl(V4L2_CID_SATURATION);
}

bool BaseV4L2VideoCapture::SetSharpness(int32_t value) {
  //return v4l2_s_ctrl(V4L2_CID_SHARPNESS, value);
  if(_pVideoCaptureFiltersCapability->VerifySharpness(value)){
    if(v4l2_s_ctrl(V4L2_CID_SHARPNESS, value)){
      _pVideoCaptureFilters->sharpness = GetSharpness();
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

int32_t BaseV4L2VideoCapture::GetSharpness() {
  return v4l2_g_ctrl(V4L2_CID_SHARPNESS);
}

bool BaseV4L2VideoCapture::SetGamma(int32_t value) {
  //return v4l2_s_ctrl(V4L2_CID_GAMMA, value);
  if(_pVideoCaptureFiltersCapability->VerifyGamma(value)){
    if(v4l2_s_ctrl(V4L2_CID_GAMMA, value)){
      _pVideoCaptureFilters->gamma = GetGamma();
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

int32_t BaseV4L2VideoCapture::GetGamma() {
  return v4l2_g_ctrl(V4L2_CID_GAMMA);
}

bool BaseV4L2VideoCapture::SetBacklightCompensation(int32_t value) {
  //return v4l2_s_ctrl(V4L2_CID_BACKLIGHT_COMPENSATION, value);
  if(_pVideoCaptureFiltersCapability->VerifyBacklightCompensation(value)){
  if(v4l2_s_ctrl(V4L2_CID_BACKLIGHT_COMPENSATION, value)){
      _pVideoCaptureFilters->backlightCompensation = GetBacklightCompensation();
    return true;
    }
    else
      return false;
  }
  else
    return false;

}

int32_t BaseV4L2VideoCapture::GetBacklightCompensation() {
  return v4l2_g_ctrl(V4L2_CID_BACKLIGHT_COMPENSATION);
}

bool BaseV4L2VideoCapture::SetFlip(int8_t value) {
  return v4l2_s_ctrl(V4L2_CID_HFLIP, value);
}

int32_t BaseV4L2VideoCapture::SetFlipMode(uint8_t vertical, uint8_t horizontal) {
  QicChangeFD(_deviceFD);
  return QicSetFlipMode(vertical,horizontal);
}

int32_t BaseV4L2VideoCapture::GetFlipMode(uint8_t *vertical, uint8_t *horizontal) {
  QicChangeFD(_deviceFD);
  return QicGetFlipMode(vertical,horizontal);
}

Variant BaseV4L2VideoCapture::GetCaptureFiltersConstraint() {
  Variant constraint;
  Limitation limitation = _pVideoCaptureFiltersCapability->GetBrightnessConstraint();
  constraint[VIDEO_BRIGHTNESS][VIDEO_CONSTRAINT_MAX] = limitation.max;
  constraint[VIDEO_BRIGHTNESS][VIDEO_CONSTRAINT_MIN] = limitation.min;

  limitation = _pVideoCaptureFiltersCapability->GetContrastConstraint();
  constraint[VIDEO_CONTRAST][VIDEO_CONSTRAINT_MAX] = limitation.max;
  constraint[VIDEO_CONTRAST][VIDEO_CONSTRAINT_MIN] = limitation.min;

  limitation = _pVideoCaptureFiltersCapability->GetHueConstraint();
  constraint[VIDEO_HUE][VIDEO_CONSTRAINT_MAX] = limitation.max;
  constraint[VIDEO_HUE][VIDEO_CONSTRAINT_MIN] = limitation.min;

  limitation = _pVideoCaptureFiltersCapability->GetSaturationConstraint();
  constraint[VIDEO_SATURATION][VIDEO_CONSTRAINT_MAX] = limitation.max;
  constraint[VIDEO_SATURATION][VIDEO_CONSTRAINT_MIN] = limitation.min;

  limitation = _pVideoCaptureFiltersCapability->GetSharpnessConstraint();
  constraint[VIDEO_SHARPNESS][VIDEO_CONSTRAINT_MAX] = limitation.max;
  constraint[VIDEO_SHARPNESS][VIDEO_CONSTRAINT_MIN] = limitation.min;

  limitation = _pVideoCaptureFiltersCapability->GetGammaConstraint();
  constraint[VIDEO_GAMMA][VIDEO_CONSTRAINT_MAX] = limitation.max;
  constraint[VIDEO_GAMMA][VIDEO_CONSTRAINT_MIN] = limitation.min;

  limitation = _pVideoCaptureFiltersCapability->GetBacklightCompensationConstraint();
  constraint[VIDEO_BACKLIGHTCOMPENSATION][VIDEO_CONSTRAINT_MAX] = limitation.max;
  constraint[VIDEO_BACKLIGHTCOMPENSATION][VIDEO_CONSTRAINT_MIN] = limitation.min;

  return constraint;
}

bool BaseV4L2VideoCapture::SetCaptureFiltersToDefault() {
  if(!SetBrightness(_pVideoCaptureFiltersCapability->GetBrightnessConstraint().dflt))
    return false;
  if(!SetContrast(_pVideoCaptureFiltersCapability->GetContrastConstraint().dflt))
    return false;
  if(!SetHUE(_pVideoCaptureFiltersCapability->GetHueConstraint().dflt))
    return false;
  if(!SetSaturation(_pVideoCaptureFiltersCapability->GetSaturationConstraint().dflt))
    return false;
  if(!SetSharpness(_pVideoCaptureFiltersCapability->GetSharpnessConstraint().dflt))
    return false;
  if(!SetGamma(_pVideoCaptureFiltersCapability->GetGammaConstraint().dflt))
    return false;
  if(!SetBacklightCompensation(_pVideoCaptureFiltersCapability->GetBacklightCompensationConstraint().dflt))
    return false;

  return true;
}

// jennifer : This may NO longer needed
Variant BaseV4L2VideoCapture::GetCurrentCaptureFilterValues() {
  Variant captureFilters;
  captureFilters[VIDEO_BRIGHTNESS] = _pVideoCaptureFilters->brightness;
  captureFilters[VIDEO_BRIGHTNESS] = _pVideoCaptureFilters->brightness;
  captureFilters[VIDEO_CONTRAST] = _pVideoCaptureFilters->contrast;
  captureFilters[VIDEO_HUE] = _pVideoCaptureFilters->hue;
  captureFilters[VIDEO_SATURATION] = _pVideoCaptureFilters->saturation;
  captureFilters[VIDEO_SHARPNESS] = _pVideoCaptureFilters->sharpness;
  captureFilters[VIDEO_GAMMA] = _pVideoCaptureFilters->gamma;
  captureFilters[VIDEO_BACKLIGHTCOMPENSATION] = _pVideoCaptureFilters->backlightCompensation;
  return captureFilters;
}

VideoCaptureFilters* BaseV4L2VideoCapture::GetCaptureFilters(){
  return _pVideoCaptureFilters;
}

bool BaseV4L2VideoCapture::SetPowerLineFrequency(int32_t value) {
  return true;
}

int32_t BaseV4L2VideoCapture::GetPowerLineFrequency() {
  return 0;
}

bool BaseV4L2VideoCapture::SetExposure(uint32_t mode, int32_t value) {
  switch(mode){
    case CAMERA_FUNC_CTRL_EXPOSURE_AUTO :
      return v4l2_s_ctrl(V4L2_CID_EXPOSURE_AUTO, value);
      break;
    case CAMERA_FUNC_CTRL_EXPOSURE_ABSOLUTE :
      return v4l2_s_ctrl(V4L2_CID_EXPOSURE_ABSOLUTE, value);
      break;
    default:
      return false;
  }
  return true;
}

int32_t BaseV4L2VideoCapture::GetExposure(uint32_t mode) {
  switch(mode){
    case CAMERA_FUNC_CTRL_EXPOSURE_AUTO :
      return v4l2_g_ctrl(V4L2_CID_EXPOSURE_AUTO);
      break;
    case CAMERA_FUNC_CTRL_EXPOSURE_ABSOLUTE :
      return v4l2_g_ctrl(V4L2_CID_EXPOSURE_ABSOLUTE);
      break;
    default:
      return -1;
  }
}

bool BaseV4L2VideoCapture::SetGain(uint32_t mode, int32_t value) {
  return v4l2_s_ctrl(mode, value);
}

int32_t BaseV4L2VideoCapture::GetGain() {
  return v4l2_g_ctrl(V4L2_CID_GAIN);
}

bool BaseV4L2VideoCapture::SetZoom(int32_t value) {
  return v4l2_s_ctrl(V4L2_CID_ZOOM_ABSOLUTE, value);
}

int32_t BaseV4L2VideoCapture::GetZoom() {
  return v4l2_g_ctrl(V4L2_CID_ZOOM_ABSOLUTE);
}

// This is working only after setting ZOOM control
/* TODO(jennifer) : remove this
bool BaseV4L2VideoCapture::SetPanTilt(int16_t pan, int16_t tilt) {
// TODO(jennifer) : TO BE IMPLEMENTED. It really depends on if CID is supported in videodev2.h
  return true;
}

void BaseV4L2VideoCapture::GetPanTilt(int16_t &pan, int16_t &tilt) {
// TODO(jennifer) : TO BE IMPLEMENTED. It really depends on if CID is supported in videodev2.h
}
 */

bool BaseV4L2VideoCapture::SetFocus(uint32_t mode, int32_t value) {
  return true;
}

int32_t BaseV4L2VideoCapture::GetFocus() {
  return 0;
}

bool BaseV4L2VideoCapture::SetWhiteBalance(uint32_t mode, int32_t value) {
  return true;
}

int32_t BaseV4L2VideoCapture::GetWhiteBalance() {
  return 0;
}

void BaseV4L2VideoCapture::SetStatus(VIDEO_CAPTURE_STATUS status) {
  _status = status;
  switch(_status){
    case VCS_ON:
      NotifyObservers(VCM_STARTED, "video started");
      break;
    case VCS_OFF:
      NotifyObservers(VCM_STOPPED, "video stopped");
      break;
    default:
      break;
  }
}

VIDEO_CAPTURE_STATUS BaseV4L2VideoCapture::GetStatus() {
  return _status;
}

string BaseV4L2VideoCapture::GetFirmwareVersion(){
  return "undefined";
}

/*
bool BaseV4L2VideoCapture::Open() {

  if((_deviceFD = open(STR(_deviceNode), O_RDWR | O_NONBLOCK, 0)) < 0) {
    FATAL("Cannot open device %s", STR(dev));
    return false;
  }
  INFO("device %s is open w/ fd=%d", STR(_deviceNode), _deviceFD);
  return true;
}
*/

bool BaseV4L2VideoCapture::Close() {
  //FATAL("Closing BaseV4L2VideoCapture : %d", _deviceFD);
  if(_deviceFD > 0){
    close(_deviceFD);
    return true;
  }
  return false;
}

bool BaseV4L2VideoCapture::IsStarted() {
  return _status == VCS_ON;
}

bool BaseV4L2VideoCapture::InitMmap() {

  if(_deviceFD < 0 ) return false;

  if(_isInit ) return true;

  _reqBufferCount = _numOfV4L2Buffers;
  //FATAL("_numOfV4L2Buffers : %d ", _numOfV4L2Buffers);
  // 1. try to request default buffer number
  if(!v4l2_reqbufs(_reqBufferCount)) {
    return false;
  }

  if (_reqBufferCount < 2) {
    //FATAL("Insufficient buffer memory on fd %d buf # %d", _deviceFD, _reqBufferCount);
    return false;
  }

  // 2.  allocate buffer pointers for mmap
  if(_pV4L2Buffers == NULL){
    _pV4L2Buffers = new V4L2Buffer[_reqBufferCount];
  }else{
    FATAL("Memory is already mapped, cannot map again");
    return false;
  }

  if (_pV4L2Buffers == NULL){
    FATAL("Allocate memory failed");
    return false;
  }

  // 3. query buf and perform memory maping
  for (uint32_t bi = 0; bi < _reqBufferCount; ++bi) { // bi: buffer index
    if(v4l2_querybuf(_pV4L2Buffers[bi], bi) < 0){
      FATAL("Query buffer failed");
      return -1;
    }
  }

  _isInit  = true;
  //FATAL("Init mmap done. Buf #:%d", _reqBufferCount);
  return true;
}

bool BaseV4L2VideoCapture::UnInitMmap() {

  if(_deviceFD < 0 ) return false;

  if(_isInit == false) return true;

  if(_pV4L2Buffers == NULL) return true;

  for (uint32_t bi = 0; bi < _reqBufferCount; ++bi) { // bi: buffer index
    if(munmap(_pV4L2Buffers[bi].start, _pV4L2Buffers[bi].length) < 0){
      FATAL("unmap failed");
      delete (_pV4L2Buffers+bi);
      return false;
    }
    //FINEST("unmap buffer %d", bi);
  }
  _pV4L2Buffers = NULL;
  _reqBufferCount = 0;

  if (!v4l2_reqbufs(_reqBufferCount)) {
    FATAL("clean buffer failed");
    return false;
  }

  //FATAL("Number of buffer after releasing : %d", _reqBufferCount);
  _isInit = false;
  //INFO("UnInit mmap done");

  return true;
}

//----------------------------------------------------------------------------
// V4L2 helper function
//----------------------------------------------------------------------------

int32_t BaseV4L2VideoCapture::xioctl(int32_t req, void *arg)
{
  int r = 0;
  do r = ioctl(_deviceFD, req, arg);
  while(-1 == r && EINTR == errno);
  //FATAL("In xioctl, errno : %d, description : %s", errno, strerror(errno));
  return r;
}

bool BaseV4L2VideoCapture::v4l2_s_ctrl(uint32_t id, int32_t value)
{
  struct v4l2_control ctrl;
  CLEAR(ctrl);

  ctrl.id = id;
  ctrl.value = value;
  if( xioctl(VIDIOC_S_CTRL, &ctrl) < 0){
    FATAL("Set control failed. id: %d, value: %d", id , value);
    return false;
  }
  return true;

}

int32_t BaseV4L2VideoCapture::v4l2_g_ctrl(uint32_t id)
{
  struct v4l2_control ctrl;
  CLEAR(ctrl);

  ctrl.id = id;
  if( xioctl(VIDIOC_G_CTRL, &ctrl) < 0){
    FATAL("Get control failed id: %d", id);
    return -1;
  }

  return ctrl.value;
}



bool BaseV4L2VideoCapture::v4l2_query_ctrl(uint32_t id, Limitation *limitation)
{
  struct v4l2_queryctrl queryctrl;
  CLEAR(queryctrl);

  queryctrl.id = id;
  if( xioctl(VIDIOC_QUERYCTRL, &queryctrl) < 0){
    FATAL("Query control failed. id: %d", id);
    return false;
  }
  limitation->max = queryctrl.maximum;
  limitation->min = queryctrl.minimum;
  //limitation->max = queryctrl.maximum/queryctrl.step;
  //limitation->min = queryctrl.minimum/queryctrl.step;
  limitation->dflt = queryctrl.default_value;
  //TODO : remove below if stable
  //FATAL("Step : %d", queryctrl.step);
  //FATAL("Original Max : %d", queryctrl.maximum);
  //FATAL("Original Min : %d", queryctrl.minimum);
  //FATAL("Max : %d", limitation->max);
  //FATAL("Min : %d", limitation->min);
  return true;

}

bool BaseV4L2VideoCapture::v4l2_querycap(struct v4l2_capability &cap)
{
  CLEAR(cap);
  if( xioctl(VIDIOC_QUERYCAP, &cap) < 0) {
    FATAL("Query capabilit failed");
    return false;
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    FATAL("device %d is not a capture devices", _deviceFD);
    return false;
  }

  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    FATAL("device %d doesn't support streaming I/O", _deviceFD);
    return false;
  }
  return true;
}

bool BaseV4L2VideoCapture::v4l2_s_fmt(uint32_t width, uint32_t height, uint32_t fmt)
{
  struct v4l2_format v4l2_fmt;
  CLEAR(v4l2_fmt);

  v4l2_fmt.type                 = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  v4l2_fmt.fmt.pix.width        = width;
  v4l2_fmt.fmt.pix.height       = height;
  v4l2_fmt.fmt.pix.pixelformat  = fmt;
  v4l2_fmt.fmt.pix.field        = V4L2_FIELD_ANY;

  if( xioctl(VIDIOC_S_FMT, &v4l2_fmt) < 0 ){
    FATAL("Set format failed");
    return false;
  }

   // QIC XU control
  if(fmt == V4L2_PIX_FMT_MPEG){
    EncoderParams_t params;
    CLEAR(params);
    QicChangeFD(_deviceFD);
    //c1 functions
    /*QicEncoderGetParams(&params);
    params.usWidth = width;
    params.usHeight = height;
    if( QicEncoderSetParams(&params, QIC_XU1_ENCODER_RESOLUTION) < 0){
      FATAL("QicEncoderSetParams failed");
      return false;
    }*/
    if (QicEuSetVideoResolution(width,height) <0 ) {
      FATAL("QicEuSetVideoResolution failed");
    }
    //c1 function is not supported now
    //QicSetStreamFormat(FORMAT_STREAM_H264_RAW_3);
    QicSetEncoderFrameSizeFormat(FORMAT_STREAM_H264_ADD_PADDING);
  }
  return true;
}

bool BaseV4L2VideoCapture::v4l2_g_fmt(uint32_t fmt)
{
  struct v4l2_format v4l2_fmt;
  CLEAR(v4l2_fmt);

  v4l2_fmt.type                 = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  v4l2_fmt.fmt.pix.pixelformat  = fmt;
  v4l2_fmt.fmt.pix.field        = V4L2_FIELD_ANY;

  if( xioctl(VIDIOC_G_FMT, &v4l2_fmt) < 0 ){
    FATAL("ERROR(%s)", __func__);
    return false;
  }

  printf("type: %d, w: %d, h: %d, fmt: %d, field: %d\n",
  v4l2_fmt.type,
  v4l2_fmt.fmt.pix.width,
  v4l2_fmt.fmt.pix.height,
  v4l2_fmt.fmt.pix.pixelformat,
  v4l2_fmt.fmt.pix.field);

  return true;
}


bool BaseV4L2VideoCapture::v4l2_reqbufs(uint32_t &reqBufferCount)
{
  struct v4l2_requestbuffers req;
  CLEAR(req);

  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.count = reqBufferCount;
  req.memory = V4L2_MEMORY_MMAP;

  if( xioctl(VIDIOC_REQBUFS, &req) < 0) {
    FATAL("Reqest buffer failed");
    return false;
  }
  //INFO("request buffer number %d and get %d", reqBufferCount, req.count);
  reqBufferCount = req.count;
  return true;

}

bool BaseV4L2VideoCapture::v4l2_querybuf(V4L2Buffer &buffer, uint32_t index)
{

  struct v4l2_buffer v4l2_buf;
  CLEAR(v4l2_buf);

  v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  v4l2_buf.memory = V4L2_MEMORY_MMAP;
  v4l2_buf.index = index;

  if ( xioctl(VIDIOC_QUERYBUF, &v4l2_buf) < 0 ) {
    FATAL("Query buffer failed");
    return false;
  }

  buffer.length = v4l2_buf.length;
  buffer.start = mmap( NULL,
            v4l2_buf.length,
            PROT_READ | PROT_WRITE, MAP_SHARED,
            _deviceFD,
            v4l2_buf.m.offset);

  if(MAP_FAILED == buffer.start){
    FATAL("MMAP Failed");
    return false;
  }

  return true;
}

bool BaseV4L2VideoCapture::v4l2_streamon()
{
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(VIDIOC_STREAMON, &type) < 0 ) {
    FATAL("Stream on failed");
    return false;
  }
  // VIDIOC_STREAMON may take some times to finish.
  // Doing io control before it finish may result in timeout
  // The dalay time here is set by experience
  //usleep(1500000);
  usleep(1200000);

  return true;
}

bool BaseV4L2VideoCapture::v4l2_streamoff()
{
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (xioctl(VIDIOC_STREAMOFF, &type) < 0 ) {
    FATAL("Stream off failed");
    return false;
  }

  return true;
}

bool BaseV4L2VideoCapture::v4l2_qbuf(struct v4l2_buffer &buf)
{
  if ( xioctl(VIDIOC_QBUF, &buf) < 0 ) {
    FATAL("Queue buffer failed");
    return false;
  }
  return true;
}

bool BaseV4L2VideoCapture::v4l2_dqbuf(struct v4l2_buffer &buf)
{
  //struct v4l2_buffer v4l2_buf;
  CLEAR(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  if ( xioctl(VIDIOC_DQBUF, &buf) < 0 ) {
    FATAL("Dequeue buffer failed, %d(%s)", errno, strerror(errno));
    //FATAL("Dequeue buffer failed");
    //jennifer
    return false;
  }
  return true;

}

bool BaseV4L2VideoCapture::CreateRelayStream(StreamsManager *pSM, string streamName) {
  NYI;
  return true;
}
