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
 *  Purpose : AVC video capture for Skype API version
 * ==================================================================
 */

#ifdef QIC1832

#include "qic/qicmotioncapture.h"
#include <linux/input.h>
#include "qic/qicmotionstream.h"

BaseHardwareAbstractDevice* QICMotionCapture::_pDeviceInstance = NULL;

BaseHardwareAbstractDevice* QICMotionCapture::GetInstance(string devNode) {
  if(_pDeviceInstance == NULL){
    int32_t fd;
    if((fd = open(STR(devNode), O_RDWR | O_NONBLOCK, 0)) < 0) {
      FATAL("Cannot open device %s", STR(devNode));
      return NULL;
    }
    _pDeviceInstance = new QICMotionCapture(fd);
    if(!_pDeviceInstance->InitializeDevice()){
      delete _pDeviceInstance;
      return NULL;
    }
    //INFO("init ok");
  }
  return _pDeviceInstance;
}

//constructor
QICMotionCapture::QICMotionCapture(int32_t fd)
  : BaseHardwareAbstractDevice(HT_VIDEO_MOTION, fd, IOHT_QIC_MOTION),
    _fcnt(0),
    _lfcnt(0),
    _lastSendTime(0),
    _errCount(0),
    _FPSControl(30),
    _CtrlTs(0)
{
  //INFO("constructor was called");
  INFO("set QIC Interrupt: %d", QicMDSetInterruptMode(1));
}

QICMotionCapture::~QICMotionCapture() {
  Close();
}

bool QICMotionCapture::Close(){
  return true;
}

bool QICMotionCapture::InitializeDevice() {
  return true;
}

void QICMotionCapture::CheckStatus(double ts) {
  bool bSendError=HardwareManager::GetSendStatus(EC_VIDEO_AVC);

  // First time, we may have error
  if (!HardwareManager::IsErrorStatus(EC_VIDEO_AVC))
    bSendError=true;

  // Triger error event
  if (_lfcnt==_fcnt && _fcnt>900)
    _errCount ++;
  else {
    _lfcnt=_fcnt;
    _errCount=0;
  }

  if (_errCount>3) {
    HardwareManager::SetStatus(EC_VIDEO_AVC, bSendError);
  }
  // Clear error if we do not have any error to server
  else if (!HardwareManager::GetSendStatus(EC_VIDEO_AVC)) {
    HardwareManager::ClearStatus(EC_VIDEO_AVC);
  }

  //enable sending message to cloud
  if ((ts - _lastSendTime) > (3600000*4)) {
    HardwareManager::ClearSendStatus(EC_VIDEO_AVC);
    _lastSendTime = ts;
  }

}

bool QICMotionCapture::StartCapture() {
  //INFO("Start motion capture");
  md_config_t getinfo;
  int ret;
  ret=QicMDGetConfiguration(&getinfo);
   if(ret==0) {
    INFO("num of roi %d",getinfo.number_of_regions);
    if(getinfo.number_of_regions==0) {
      //assign default roi
      rect_t myrect;
      //myrect.x=2;
      //myrect.y=2;
      //myrect.width=76;
      //myrect.height=41;

      /* Temp set the default roi to VGA size from (0, 0)*/
      myrect.x=0;
      myrect.y=0;
      myrect.width=40;
      myrect.height=30;

      md_region_of_interest_t myroi;
      myroi.id=0;
      myroi.sensitivity=60;
      myroi.history=3;
      myroi.min_object_size=16;
      myroi.region_of_intrest=myrect;

      md_config_t setinfo;
      setinfo.number_of_regions=1;
      setinfo.region_of_interests[0]=myroi;

      QicMDSetConfiguration(&setinfo);
    }else {
      INFO("load ROIs on chip");
      int roiCount;
      for(roiCount=0;roiCount<getinfo.number_of_regions;roiCount++){
        int x=uint32_t(getinfo.region_of_interests[roiCount].region_of_intrest.x*16);
        int y=uint32_t(getinfo.region_of_interests[roiCount].region_of_intrest.y*16);
        int w=uint32_t(getinfo.region_of_interests[roiCount].region_of_intrest.width*16);
        int h=uint32_t(getinfo.region_of_interests[roiCount].region_of_intrest.height*16);
        INFO("x:%d, y:%d, w:%d, h:%d", x, y, w, h);
      }
    }
  }else {
    FATAL("fail to get configuration");
    return false;
  }

  //ENABLE_DEVICE_READ_DATA;
  ENABLE_DEVICE_READ_DATA
  if(QicMDSetEnable(true)!=0) {
    FATAL("fail to enable motion detection");
    return false;
  }else{
    //INFO("qicmotion enable ok, then get one motion");
    //OnReadEvent();
    return true;
  }
}

bool QICMotionCapture::StopCapture() {
  INFO("disable motion detection");
  if(QicMDSetEnable(false)!=0) {
    FATAL("fail to disable motion detection");
    return false;
  }else{
    DISABLE_DEVICE_READ_DATA
    return true;
  }
}

bool QICMotionCapture::GetROIINFO(Variant &getMessage) {


  return true;
}

bool QICMotionCapture::SetROIINFO(Variant setMessage) {

  int upperboundofROIcount=3;
  map<string, Variant>::iterator iter;
  int inputROIcount=0;
  md_region_of_interest_t myroi[upperboundofROIcount];
  for(iter=setMessage["ROIs"].begin(); iter!=setMessage["ROIs"].end(); iter++){
    FATAL("QICMotionCapture::SetROIINFO");
    //allow three roi only
    inputROIcount++;
    if(inputROIcount>upperboundofROIcount)
      break;

    //this roi of Variant
    Variant eachROI;
    eachROI=iter->second;

    //allocate data structure rect
    rect_t myrect;
    myrect.x=uint16_t(eachROI["x"])/16;
    myrect.y=uint16_t(eachROI["y"])/16;
    myrect.width=uint16_t(eachROI["width"])/16;
    myrect.height=uint16_t(eachROI["height"])/16;
    INFO("[SetROIINFO] x:%d, y:%d, w:%d, h:%d", uint16_t(eachROI["x"]),
        uint16_t(eachROI["y"]), uint16_t(eachROI["width"]),
        uint16_t(eachROI["height"]));

    //set attribute of roi
    myroi[inputROIcount-1].id=inputROIcount-1;
    myroi[inputROIcount-1].sensitivity=uint8_t(eachROI["sensitivity"]);
    myroi[inputROIcount-1].history=uint16_t(eachROI["updateRate"]);
    myroi[inputROIcount-1].min_object_size=uint8_t(eachROI["minObjSize"]);
    myroi[inputROIcount-1].region_of_intrest=myrect;
  }
  md_config_t setinfo;
  setinfo.number_of_regions=(inputROIcount>upperboundofROIcount)?3:inputROIcount;

  if(setinfo.number_of_regions==0){
    //set default ROI
    rect_t myrectD;
    myrectD.x=2;
    myrectD.y=2;
    myrectD.width=76;
    myrectD.height=41;

    md_region_of_interest_t myroiD;
    myroiD.id=0;
    myroiD.sensitivity=60;
    myroiD.history=3;
    myroiD.min_object_size=16;
    myroiD.region_of_intrest=myrectD;

    setinfo.region_of_interests[0]=myroiD;
    setinfo.number_of_regions=1;
  }

  for(int i=0;i<inputROIcount;i++)
    setinfo.region_of_interests[i]=myroi[i];

  if(QicMDSetConfiguration(&setinfo)!=0) {
    FATAL("fail to set configuration");
    return false;
  }

  return true;
}

bool QICMotionCapture::SetFPS(uint8_t fpsCtrl){
  _FPSControl=fpsCtrl;
  return true;
}

int QICMotionCapture::getMotion(rect_t *myMotion, unsigned long &ts) {

  md_status_t myMotionStatus;
  QicMDGetStatus(&myMotionStatus);


  INFO("get motion object count %u", myMotionStatus.number_of_moving_objects);
  ts=myMotionStatus.timestamp;
  for(int i=0;i<myMotionStatus.number_of_moving_objects;i++){
    myMotion[i] = myMotionStatus.moving_objects[i];
      /*INFO("x:%d, y:%d, w:%d, h:%d",
            myMotion.x,
        myMotion.y,
      myMotion.width,
      myMotion.height
      );*/
   }
  return myMotionStatus.number_of_moving_objects;
 }


bool QICMotionCapture::OnReadEvent() {

  //INFO("head of on OnreadEvent");
  struct input_event ev[64];
  int rd;
  rd = read(GetDeviceFD(), ev, sizeof(struct input_event) * 64);

  if (rd < (int) sizeof(struct input_event)) {
    printf("expected %d bytes, got %d\n", (int) sizeof(struct input_event), rd);
    perror("\nevtest: error reading");
    return 1;
  }

  rect_t mymotion[3];
  unsigned long ts;
  int numofmotion=getMotion(mymotion, ts);
//  INFO("numofmotion %d",numofmotion);

  uint8_t motionbuffer[4*numofmotion];
  for(int i=0;i<numofmotion;i++){
    motionbuffer[i]=mymotion->x;
    motionbuffer[i+1]=mymotion->y;
    motionbuffer[i+2]=mymotion->width;
    motionbuffer[i+3]=mymotion->height;
  }
  INFO("ts: %lu last ts:%lu", ts, _CtrlTs);
  if(_CtrlTs==0)
    _CtrlTs=ts-(1000/_FPSControl+1);

  if((ts-_CtrlTs)>(1000/_FPSControl)) {
    _CtrlTs=ts;
    SendDataToCarriers(motionbuffer,4*numofmotion, ts, HW_DATA_MOTION);
   }
  return true;
 }


uint8_t QICMotionCapture::CreateMotionStream(StreamsManager *pSM, string streamName) {
  // 1. create video stream for application
  // video stream is created here but deleting in BaseV4L2VideoCapture because video stream is relevant to hardware device
  QICMotionStream *pStream = new QICMotionStream(pSM, streamName);
  // 2. register video stream as carrier for receiving data
  RegisterCarrier((IDeviceCarrier *)pStream);
  // 3. register video stream as observer for obtaining notification
  RegisterObserver((IDeviceObserver *)pStream);
  return 1;
}

#endif
