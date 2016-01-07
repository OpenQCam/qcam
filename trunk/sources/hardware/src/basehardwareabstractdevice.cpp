// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    :
// // Version :
// // Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//

#include "basehardwareabstractdevice.h"
#include "hardwaremanager.h"

//uint32_t BaseHardwareAbstractDevice::_observerUniqueID = 0;
//uint32_t BaseHardwareAbstractDevice::_carrierUniqueID = 0;
uint32_t IDeviceObserver::_idGenerator = 0;
uint32_t IDeviceCarrier::_idGenerator = 0;

BaseHardwareAbstractDevice::BaseHardwareAbstractDevice(HardwareType hwType,
    int32_t deviceFD, IOHandlerType ioType)
: IOHandler(deviceFD, deviceFD, ioType),
  _hwType(hwType),
  _deviceFD(deviceFD),
  _readDataEnabled(false),
  _writeDataEnabled(false)
{
  HardwareManager::RegisterHardwareDevice(this);
}

BaseHardwareAbstractDevice::BaseHardwareAbstractDevice(HardwareType hwType,
    IOHandlerType ioType)
: IOHandler(0, 0, ioType),
  _hwType(hwType),
  _deviceFD(0)
{
  HardwareManager::RegisterHardwareDevice(this);

}




BaseHardwareAbstractDevice::~BaseHardwareAbstractDevice() {
  HardwareManager::UnRegisterHardwareDevice(_hwType);
  _status.clear();
}


HardwareType BaseHardwareAbstractDevice::GetType() {
  return _hwType;
}

int32_t BaseHardwareAbstractDevice::GetDeviceFD() {
  return _deviceFD;
}

//string BaseHardwareAbstractDevice::GetDeviceNode() {
//  return _deviceNode;
//}


uint32_t BaseHardwareAbstractDevice::RegisterObserver(IDeviceObserver *pObserver) {
  //pObserver->SetObserverId(_observerUniqueID++);
  WARN ("Register Observer");
  _observers.push_back(pObserver);
  return pObserver->GetObserverId();
}

void BaseHardwareAbstractDevice::UnRegisterObserver(IDeviceObserver *pObserver) {
  FOR_VECTOR_ITERATOR_NOINC(IDeviceObserver*, _observers, i) {
    if(pObserver->GetObserverId() == VECTOR_VAL(i)->GetObserverId()){
      _observers.erase(i);
      break;
    }
    //else
    //  i++;
  }
}

void BaseHardwareAbstractDevice::NotifyObservers(DEVICE_MESSAGE_TYPE type, string info) {
  _message.type = type;
  _message.info = info;
  FOR_VECTOR_ITERATOR(IDeviceObserver*, _observers, i) {
    VECTOR_VAL(i)->OnDeviceMessage(_message);
  }
}

uint32_t BaseHardwareAbstractDevice::GetNumOfCarriers() {
  return _carriers.size();
}

uint32_t BaseHardwareAbstractDevice::RegisterCarrier(IDeviceCarrier *pCarrier) {
  //pCarrier->SetCarrierId(_carrierUniqueID);
  uint32_t carrierId=pCarrier->GetCarrierId();
  _carriers[carrierId] = pCarrier;
  INFO("carrier ID base %d", carrierId);
  return carrierId;
  //_carriers.push_back(pCarrier);
}

void BaseHardwareAbstractDevice::UnRegisterCarrier(IDeviceCarrier *pCarrier) {
  if(MAP_HAS1(_carriers, pCarrier->GetCarrierId())){
    _carriers.erase(pCarrier->GetCarrierId());
  }else{
    WARN("Unable to find carrier");
  }
  //FOR_VECTOR_ITERATOR_NOINC(IDeviceCarrier*, _carriers, i) {
  //  if(pCarrier->GetCarrierId() == VECTOR_VAL(i)->GetCarrierId()){
  //    _carriers.erase(i);
  //    break;
  //  }
  //  //else
  //  //  i++;
  //}
}

void BaseHardwareAbstractDevice::SendDataToCarriers(uint8_t *pData, uint32_t dataLength, uint64_t ts) {
  _data.buffer = pData;
  _data.length = dataLength;
  _data.ts = ts;
  FOR_MAP(_carriers, uint32_t, IDeviceCarrier *, i) {
    MAP_VAL(i)->OnDeviceData(_data);
  }
  //FOR_VECTOR_ITERATOR(IDeviceCarrier*, _carriers, i) {
  //  VECTOR_VAL(i)->OnDeviceData(_data);
  //}
}

void BaseHardwareAbstractDevice::SendDataToCarriers(uint8_t *pData, uint32_t dataLength,
                                                    uint64_t ts, uint32_t type) {
  _data.buffer = pData;
  _data.length = dataLength;
  _data.ts = ts;
  _data.datatype = type;
  FOR_MAP(_carriers, uint32_t, IDeviceCarrier *, i) {
    MAP_VAL(i)->OnDeviceData(_data);
  }
  //FOR_VECTOR_ITERATOR(IDeviceCarrier*, _carriers, i) {
  //  VECTOR_VAL(i)->OnDeviceData(_data);
  //}
}

void BaseHardwareAbstractDevice::SendDataToCarrier(uint8_t *pData, uint32_t dataLength, uint64_t ts, uint32_t id) {
  _data.buffer = pData;
  _data.length = dataLength;
  _data.ts = ts;
  //FIXME(Tim): program will crash if we don't check id is available
  // but it will take more time if using MAP_HAS1 to check id is available
  _carriers[id]->OnDeviceData(_data);
  //if(MAP_HAS1(id)){
  //  _carriers[id]->OnDeviceData(_data);
  //}
}

//jennifer
void BaseHardwareAbstractDevice::SendDataToCarrier(uint8_t *pData, uint32_t dataLength,
                                                   uint64_t ts, uint32_t id, uint32_t type) {
  _data.buffer = pData;
  _data.length = dataLength;
  _data.ts = ts;
  _data.datatype = type;
  //FIXME(Tim): program will crash if we don't check id is available
  // but it will take more time if using MAP_HAS1 to check id is available
  _carriers[id]->OnDeviceData(_data);
  //if(MAP_HAS1(id)){
  //  _carriers[id]->OnDeviceData(_data);
  //}
}

bool BaseHardwareAbstractDevice::OnEvent(select_event &event) {
  switch (event.type) {
    case SET_READ:
    {
      return OnReadEvent();
    }
    case SET_WRITE:
    {
      return OnWriteEvent();
    }
    default:
    {
      ASSERT("Invalid state: %hhu", event.type);
      return false;
    }
  }
}


bool BaseHardwareAbstractDevice::OnReadEvent() {
  return true;
}

bool BaseHardwareAbstractDevice::OnWriteEvent() {
  return true;
}

bool BaseHardwareAbstractDevice::WriteDataToDevice(uint8_t *pData, uint32_t dataLength) {
  return true;
}


bool BaseHardwareAbstractDevice::SignalOutputData() {
  return true;
}
BaseHardwareAbstractDevice::operator string() {
  //return _deviceNode;

  string hwString = "Unknown";

  switch(_hwType){
    case HT_VIDEO_MJPG:
      hwString = "HT_VIDEO_MJPG";
      break;
    case HT_VIDEO_AVC:
      hwString = "HT_VIDEO_AVC";
      break;
    case HT_VIDEO_MOTION:
      hwString = "HT_VIDEO_MOTION";
      break;
    case HT_MIC:
      hwString = "HT_MIC";
      break;
    case HT_SPEAKER:
      hwString = "HT_SPEAKER";
      break;
    case HT_GPIO:
      hwString = "HT_GPIO";
      break;
    case HT_NVRAM:
      hwString = "HT_NVRAM";
      break;
    case HT_STORAGE:
      hwString = "HT_STORAGE";
      break;
    case HT_NFS_STORAGE:
      hwString = "HT_NFS_STORAGE";
      break;
    case HT_CPU:
      hwString = "HT_CPU";
      break;
    case HT_MEM:
      hwString = "HT_MEM";
      break;
  }
  return hwString;
}

void BaseHardwareAbstractDevice::GetStats(Variant &info, uint32_t namespaceId){
}
