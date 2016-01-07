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

#ifndef _HARDWAREMANAGER_H
#define _HARDWAREMANAGER_H

#include "hardwaretypes.h"

class BaseHardwareAbstractDevice;
class BaseClientApplication;
class Variant;

class HardwareManager {
  private:
    static map<HardwareType, BaseHardwareAbstractDevice*> _hwInstances;
    static map<uint32_t, DeviceErrorStatus> _status;
    static int _pipe_fds[2];

  public:
    static bool InitHardwareDevice(Variant& configuration);
    static bool ShutdownHardware();
    static bool RegisterHardwareDevice(BaseHardwareAbstractDevice *pInstance);
    static bool UnRegisterHardwareDevice(HardwareType hwType);
    static BaseHardwareAbstractDevice* GetHardwareInstance(HardwareType hwType);

    static void CheckStatus(BaseClientApplication *pApp, double timeStamp);
    static void SetStatus(uint32_t errStatus, bool bSend);
    static void ClearStatus(uint32_t errStatus);
    static bool IsErrorStatus(uint32_t errStatus);
    static bool GetSendStatus(uint32_t statusCode);
    static void ClearSendStatus(uint32_t statusCode);

  private:
    static void CreateQICMJPGVideoCapture(string devNode);
    static void CreateQICAVCVideoCapture(string devNode);
    static void CreateQICMotionCapture(string devNode);
    static void CreateAudioCapture(string devNode);
    static void CreateAudioPlayer(string devNode);
    static void CreateNVRam(string filePath);
    static void CreateStorage(string mountPath);
    static void CreateNFSStorage(string mountPath);
    static void CreateCPU(string path);
    static void CreateMemory(uint32_t minSize);
    // static bool CreateGPIOxxx();
};


#endif
