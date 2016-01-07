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


#ifndef _DEVICEINTERFACE_H
#define _DEVICEINTERFACE_H
#include "common.h"

typedef int32_t DEVICE_MESSAGE_TYPE;

enum DEVICE_MESSAGE_ENUM {
  //Device(0)

  //Video(1)
  VCM_STARTED = 0x1000,
  VCM_STOPPED,
  VCM_RESOLUTION_CHANGED,
  VCM_FPS_CHANGED,
  VCM_BITRATE_CHANGED,
  VCM_PROFILE_CHANGED,
  VCM_KEYFRAME_DONE,
  //Record(2)

  //Audio(3)
  ACM_STARTED = 0x3000,
  ACM_STOPPED,
  ACM_DETECTED,
  ACM_RMSVALUE,

  APM_STARTED,
  APM_STOPPED,
  APM_CLOSED,
  APM_DATAREADY,

  //PTZ(4)
  //IO(5)
  GPIO_PIRALRAM = 0x5000,

  //NVRAM
  NV_SET_FAILURE = 0x6000,
};

struct DeviceMessage {
  DEVICE_MESSAGE_TYPE type;
  string info;
};

struct DeviceData {
  uint8_t *buffer;
  uint32_t length;
  uint64_t ts;
  uint32_t datatype;
};

class IDeviceObserver{
  private:
    static uint32_t _idGenerator;
    uint32_t _observerId;
  public:
    virtual ~IDeviceObserver() {};
    IDeviceObserver() {
      _observerId = ++_idGenerator;
    }
//    void SetObserverId(uint32_t id) { _observerId = id; }
    uint32_t GetObserverId() { return _observerId; }
    virtual void OnDeviceMessage(DeviceMessage &msg) = 0;
};

class IDeviceCarrier {
  private:
    static uint32_t _idGenerator;
    uint32_t _carrierId;
  public:
    virtual ~IDeviceCarrier() {};
    IDeviceCarrier() {
      _carrierId = ++_idGenerator;
    }
    //void SetCarrierId(uint32_t id) { _carrierId = id; }
    uint32_t GetCarrierId() { return _carrierId; }
    virtual void OnDeviceData(DeviceData &data) = 0;
};

#endif /* _DEVICEINTERFACE_H */
