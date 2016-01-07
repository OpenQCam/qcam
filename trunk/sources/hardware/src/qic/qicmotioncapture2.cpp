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
 *  Author  : S.J.Hung
 *  Purpose : read motion detection results.
 * ==================================================================
 */

#ifdef QIC1832

#include "qic/qicmotioncapture2.h"

BaseHardwareAbstractDevice* QICMotionCapture2::GetInstance(string /* devNode */, int pipe_read_fd) {
  static BaseHardwareAbstractDevice* _pDeviceInstance = NULL;

  if(_pDeviceInstance == NULL){
    /* assign read end of pipe to QICMotionCapture2 */
    _pDeviceInstance = new QICMotionCapture2(pipe_read_fd);
    if(!_pDeviceInstance->InitializeDevice()){
      delete _pDeviceInstance;
      return NULL;
    }
  }
  return _pDeviceInstance;
}

//constructor
QICMotionCapture2::QICMotionCapture2(int32_t fd)
  : QICMotionCapture(fd),
   _eventInMiliSec(1000),
   _eventCountThreshold(15),
   _eventBufferSize(30)
{
  INFO("set QIC Interrupt: %d", QicMDSetInterruptMode(0));
  _motionBox.currentIndex=0;
}

QICMotionCapture2::~QICMotionCapture2() {
}

int QICMotionCapture2::getMotion(rect_t *myMotion, unsigned long &ts) {

  md_status_t myMotionStatus;

  /* read md_status_t from pipe */
  if(read(GetDeviceFD(), &myMotionStatus, sizeof(myMotionStatus)) == -1)
  {
      FATAL("read from pipe fail, %s\n", strerror(errno));
      return 0;
  }

  /* copy moving objects fro md_status_t to myMotion */
  //INFO("get motion object count %u", myMotionStatus.number_of_moving_objects);
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


bool QICMotionCapture2::OnReadEvent() {

  /* call getMotion and deliver to carrier */
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
  /*INFO("ts: %lu last ts:%lu", ts, _CtrlTs);
  if(_CtrlTs==0)
    _CtrlTs=ts-(1000/_FPSControl+1);

  if((ts-_CtrlTs)>(1000/_FPSControl)) {
    _CtrlTs=ts;
    SendDataToCarriers(motionbuffer,4*numofmotion, ts, HW_DATA_MOTION);
   }*/

  _motionBox.tsCache[_motionBox.currentIndex]=ts;
  _motionBox.currentIndex++;
  _motionBox.currentIndex%=_eventBufferSize;
  uint8_t boxCounter;
  uint8_t hitCounter=0;
  for (boxCounter=0;boxCounter<_eventBufferSize;boxCounter++){
    if((ts-_motionBox.tsCache[boxCounter]) < _eventInMiliSec)
      hitCounter++;
  }
  if(hitCounter >= _eventCountThreshold)
    SendDataToCarriers(motionbuffer,4*numofmotion, ts, HW_DATA_MOTION);
  return true;
 }

uint64_t QICMotionCapture2::getEventInMiliSec() {
  return _eventInMiliSec;
}

bool QICMotionCapture2::setEventInMiliSec(uint64_t msec) {
  if(msec != 0) {
    _eventInMiliSec=msec;
    return true;
  }else {
    return false;
  }
}

uint8_t QICMotionCapture2::getEventCountThreshold() {
  return _eventCountThreshold;
}

bool QICMotionCapture2::setEventCountThreshold(uint8_t count) {
  if(count<=_eventBufferSize) {
    _eventCountThreshold=count;
    return true;
  }else {
    return false;
  }

}

uint8_t QICMotionCapture2::getEventBufferSize() {
  return _eventBufferSize;
}

bool QICMotionCapture2::setEventBufferSize(uint8_t size) {
  if(size <256){
    _eventBufferSize=size;
    return true;
  }else {
    return false;
  }
}

#endif
