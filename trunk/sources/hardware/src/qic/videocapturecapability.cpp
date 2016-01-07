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
 *  Author  : Jennifer Chao (jennifer.chao@quantatw.com)
 *  Purpose : Video Capture Capability Class for all video capture device
 * ==================================================================
 */


#include "qic/videocapturecapability.h"

#define MAX_GOP 120
#define MIN_GOP 30
/* video constraints set by designer
const VideoConstraint VideoCaptureCapability::videoConstraints[AVAIL_SUB_STREAMS] = {
  { 16000000, 500000, 30, 5, 120, 30}
  ,{ 5000000, 500000, 30, 5, 120, 30}
  ,{ 2000000, 500000, 30, 5, 120, 30}
  ,{ 2000000, 500000, 30, 5, 120, 30}
};
*/

// Defined by designer used to set maximum resolution for each stream layer of simulcast streams
const uint8_t VideoCaptureCapability::_kConstraintIndex[AVAIL_SUB_STREAMS] = {0,2,6,6};
// Defined by designer used to filter out unwanted resolution
const uint16_t VideoCaptureCapability::_kWidthArray[AVAIL_RESOLUTION_SET] = {1920, 1280, 640, 640, 320, 320, 160, 160};
const uint16_t VideoCaptureCapability::_kHeightArray[AVAIL_RESOLUTION_SET] = {1080, 720, 480, 360, 240, 180, 120, 90};
const string VideoCaptureCapability::_kResolutionNameArray[AVAIL_RESOLUTION_SET] = {"FHD", "HD","VGA", "360p","QVGA", "180p", "QQVGA", "90p"};
// Container for video device supported capabilities, such as resolution, fps, bitrate etc
VideoConstraintMap* VideoCaptureCapability::_videoConstraintMap = NULL;

VideoCaptureCapability::VideoCaptureCapability()
{
}

VideoCaptureCapability::~VideoCaptureCapability() {
}

bool VideoCaptureCapability::InitVideoCapability(uint16_t constraintIndex, bool isAVC){
  uint16_t i=0, first=0, lastSupportedIndex=0;
  uint32_t resolutionKey;
  Resolution resolution;

  _isAVC = isAVC;
  for(i=_kConstraintIndex[constraintIndex]; i<AVAIL_RESOLUTION_SET; i++){
    resolutionKey = ((_kWidthArray[i] << 16) | _kHeightArray[i]);

    // Check if selected resolution is supported by the device
    if(!MAP_HAS1(*_videoConstraintMap,resolutionKey)){
      FATAL("Resolution  %dx%d is NOT supported by the video device. Skip it", _kWidthArray[i],  _kHeightArray[i]);
      continue;
    }
    lastSupportedIndex = i;
    //FATAL("lastSupportedIndex : %d", lastSupportedIndex);

    // Set maximum bitrate for current video stream
    if(0 == first){
      _maxBitrate = (*_videoConstraintMap)[resolutionKey].maxBitrate;
      first++;
      //FATAL("MAX BITRATE of resolution %dx%d :%d", _kWidthArray[i],  _kHeightArray[i], (*_videoConstraintMap)[resolutionKey].maxBitrate);
      //FATAL("MIN BITRATE of resolution %dx%d :%d", _kWidthArray[i],  _kHeightArray[i], (*_videoConstraintMap)[resolutionKey].minBitrate);
    }

    // Save available resolution
    resolution.width = _kWidthArray[i];
    resolution.height = _kHeightArray[i];
    resolution.name = _kResolutionNameArray[i];
    _resolution[resolutionKey] = resolution;
  }

  // Set minimum bitrate for current video stream
  resolutionKey = ((_kWidthArray[lastSupportedIndex] << 16) | _kHeightArray[lastSupportedIndex]);
  _minBitrate = (*_videoConstraintMap)[resolutionKey].minBitrate;
  //FATAL("MIN BITRATE of resolution %dx%d :%d", _kWidthArray[lastSupportedIndex],  _kHeightArray[lastSupportedIndex], (*_videoConstraintMap)[resolutionKey].minBitrate);

  // NOTE: since the framerate boundary is the same for all resolution,
  // the boundary of any stream layer is assigned as
  // that of smallest resolution for convenience sake
  _maxFPS = (*_videoConstraintMap)[resolutionKey].maxFPS;
  _minFPS = (*_videoConstraintMap)[resolutionKey].minFPS;

  // NO GOP boundary for QIC, define it in here streamer
  _maxGOP = MAX_GOP;
  _minGOP = MIN_GOP;

  // For debug sake
  //PrintDeviceVideoConstraint();
  //PrintConstraintValues();
  return true;
}

Variant VideoCaptureCapability::GetConstraints(){
  Variant constraints;
  Variant resolution;
  Variant fps;
  Variant bitrate;
  Variant gop;

  Resolution res;
  map<uint32_t,Resolution>::iterator resolutionIterator;

  for(resolutionIterator=_resolution.begin(); resolutionIterator!=_resolution.end(); resolutionIterator++){
    res = MAP_VAL(resolutionIterator);
    resolution["width"] = res.width;
    resolution["height"] = res.height;
    resolution["name"] = res.name;
    constraints["resolution"].PushToArray(resolution);
  }

  fps["max"] = _maxFPS;
  fps["min"] = _minFPS;
  bitrate["max"] = _maxBitrate;
  bitrate["min"] = _minBitrate;
  gop["max"] = _maxGOP;
  gop["min"] = _minGOP;

  if(_isAVC){
    // jennifer(TODO) : provide this information for MJPG when firmware support it
    constraints["fps"] = fps;
    constraints["bitrate"] = bitrate;
    constraints["gop"] = gop;
  }

  return constraints;
}
void VideoCaptureCapability::GetResolutionConstraints(Resolution *resArray){
  map<uint32_t,Resolution>::iterator resolutionIterator;
  int i;

  for(i=0,resolutionIterator=_resolution.begin(); resolutionIterator!=_resolution.end(); resolutionIterator++, i++)
    resArray[i] = MAP_VAL(resolutionIterator);

}
void VideoCaptureCapability::GetAVCVideoConstraints(VideoConstraint *otherConstraint){
  otherConstraint->maxBitrate = _maxBitrate;
  otherConstraint->minBitrate = _minBitrate;
  otherConstraint->maxFPS = _maxFPS;
  otherConstraint->minFPS = _minFPS;
  otherConstraint->maxGOP = _maxGOP;
  otherConstraint->minGOP = _minGOP;
}
bool VideoCaptureCapability::VerifyResolution(uint16_t width, uint16_t height){
  //uint32_t resolutionKey = ((width << 16) | height);
  //if(MAP_HAS1(_resolution,resolutionKey)){
    return true;
  //}
  //FATAL("Invalid ! Resolution to be verified %dx%d", width, height);
  //return false;
}
bool VideoCaptureCapability::VerifyBitrate(uint32_t bitrate){
  if(bitrate < _minBitrate || bitrate > _maxBitrate){
    FATAL("Invalid bitrate %d. Bitrate should between %d and %d", bitrate, _minBitrate, _maxBitrate);
    return false;
  }
  return true;
}
bool VideoCaptureCapability::VerifyFPS(uint32_t fps){
  if(fps < _minFPS || fps > _maxFPS){
    FATAL("Invalid fps %d. Frame rate should between %d and %d", fps, _minFPS, _maxFPS);
    return false;
  }
  return true;
}
bool VideoCaptureCapability::VerifyGOP(uint32_t gop){
  if(gop < _minGOP || gop > _maxGOP){
    FATAL("Invalid gop %d. GOP should between %d and %d", gop, _minGOP, _maxGOP);
    return false;
  }
  return true;
}
void VideoCaptureCapability::SetDeviceVideoConstraint(VideoConstraintMap *videoConstraintMap){
  _videoConstraintMap = videoConstraintMap;
  // For debug sake
  //PrintDeviceVideoConstraint();
}
void VideoCaptureCapability::PrintDeviceVideoConstraint(){
  map<uint32_t,VideoConstraint>::iterator videoConstraintIterator;
  VideoConstraint videoConstraint;
  Resolution resolution;

  for(videoConstraintIterator=_videoConstraintMap->begin(); videoConstraintIterator!=_videoConstraintMap->end(); videoConstraintIterator++){
    videoConstraint = MAP_VAL(videoConstraintIterator);
    resolution.height = MAP_KEY(videoConstraintIterator) & 0x0000ffff;
    resolution.width = MAP_KEY(videoConstraintIterator) >> 16;
    INFO("resolution : %dx%d", resolution.width, resolution.height);
    INFO("Bitrate, max : %d, min : %d", videoConstraint.maxBitrate , videoConstraint.minBitrate);
    INFO("FPS, max : %d, min : %d", videoConstraint.maxFPS, videoConstraint.minFPS);
  }
}
bool VideoCaptureCapability::PrintConstraintValues(){
  map<uint32_t,Resolution>::iterator resolutionIterator;
  Resolution resolution;

  for(resolutionIterator=_resolution.begin(); resolutionIterator!=_resolution.end(); resolutionIterator++){
    resolution = MAP_VAL(resolutionIterator);
    INFO("Name : %s, %dx%d", STR(resolution.name), resolution.width, resolution.height);
  }

  INFO("Bitrate, max : %d, min : %d", _maxBitrate, _minBitrate);
  INFO("FPS, max : %d, min : %d", _maxFPS, _minFPS);
  INFO("GOP, max : %d, min : %d", _maxGOP, _minGOP);
  return true;
}
