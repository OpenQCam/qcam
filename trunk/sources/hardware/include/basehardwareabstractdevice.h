// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    :  2013/02/26
// // Version :
// // Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose : Hardware abstract class
// // ======================================================================
//


#ifndef _BASEHARDWAREABSTRACTDEVICE_H
#define _BASEHARDWAREABSTRACTDEVICE_H

//thelib
#include "system/errordefine.h"
//hardware
#include "netio/select/iohandler.h"
#include "netio/select/iohandlermanager.h"
#include "interface/deviceinterface.h"
#include "hardwaretypes.h"
#include "hardwaremanager.h"


#define ENABLE_DEVICE_READ_DATA \
if (!_readDataEnabled) { \
    _readDataEnabled = true; \
    IOHandlerManager::EnableReadData(this); \
} \

#define DISABLE_DEVICE_READ_DATA \
if (_readDataEnabled) { \
    _readDataEnabled = false; \
    IOHandlerManager::DisableReadData(this); \
} \

#define ENABLE_DEVICE_WRITE_DATA \
if (!_writeDataEnabled) { \
    _writeDataEnabled = true; \
    IOHandlerManager::EnableWriteData(this); \
} \

#define DISABLE_DEVICE_WRITE_DATA \
if (_writeDataEnabled) { \
    _writeDataEnabled = false; \
    IOHandlerManager::DisableWriteData(this); \
} \

typedef enum{
  HW_DATA_VIDEO=0x0,
  HW_DATA_AUDIO=0x1,
  HW_DATA_MOTION=0x2
} HW_DATATYPE;



class BaseHardwareAbstractDevice
: public IOHandler
{
  private:
//    static uint32_t _observerUniqueID;
    static uint32_t _carrierUniqueID;
    //vector<IDeviceCarrier *> _carriers;
    map<uint32_t, IDeviceCarrier *> _carriers;
    vector<IDeviceObserver *> _observers;

  protected:
    HardwareType _hwType;
    int32_t _deviceFD;
    DeviceMessage _message;
    DeviceData _data;
    bool _readDataEnabled;
    bool _writeDataEnabled;
    map<uint32_t, DeviceErrorStatus> _status;

    BaseHardwareAbstractDevice(HardwareType hwType, int32_t deviceFD, IOHandlerType ioType);
    BaseHardwareAbstractDevice(HardwareType hwType, IOHandlerType ioType);
  public:
    //static BaseHardwareAbstractDevice* GetInstance();

    virtual ~BaseHardwareAbstractDevice();

    virtual bool InitializeDevice() = 0;
    virtual bool Close()=0;

    // event callback
    virtual bool OnEvent(select_event &event);
    virtual bool OnReadEvent();
    virtual bool OnWriteEvent();
    virtual void CheckStatus(double ts)=0;

    // generic
    HardwareType GetType();
    int32_t GetDeviceFD();
    //string GetDeviceNode();

    // observer
    uint32_t RegisterObserver(IDeviceObserver *pObserver);
    void UnRegisterObserver(IDeviceObserver *pObserver);
    /**
     * @Brief Notify all observers by IDeviceObserver::OnDeviceMessage interface
     *
     * @Param type - notification type
     * @Param info - information for notification
     */
    void NotifyObservers(DEVICE_MESSAGE_TYPE type, string info);

    //void NotifyObserver(DEVICE_MESSAGE_TYPE type, string info, uint32_t index);

    // carrier
    uint32_t GetNumOfCarriers();
    uint32_t RegisterCarrier(IDeviceCarrier *pCarrier);
    void UnRegisterCarrier(IDeviceCarrier *pCarrier);

    // interface for sending data from device to carriers,
    virtual void SendDataToCarriers(uint8_t *pData, uint32_t dataLength, uint64_t ts);

    // interface for sending data from device to carriers,
    virtual void SendDataToCarriers(uint8_t *pData, uint32_t dataLength,
                                    uint64_t ts, uint32_t type);

    // interface for sending data from device to designated carrier
    virtual void SendDataToCarrier(uint8_t *pData, uint32_t dataLength, uint64_t ts, uint32_t id);
    virtual void SendDataToCarrier(uint8_t *pData, uint32_t dataLength,
                                   uint64_t ts, uint32_t id, uint32_t type);

    // interface for sending data from carriers to deivce
    virtual bool WriteDataToDevice(uint8_t *pData, uint32_t dataLength);


    // iohandler
    virtual bool SignalOutputData();
    virtual operator string();
    virtual void GetStats(Variant &info, uint32_t namespaceId = 0);
};

#endif
