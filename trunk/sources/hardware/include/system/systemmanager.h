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
//

#ifndef _SYSTEMMANAGER_H
#define _SYSTEMMANAGER_H

#include "common.h"
#include "system/errordefine.h"

#define ERROR_DESCRIPTION(code) SystemManager::GetErrorDescription(code)
#define ERROR_CODE(code) SystemManager::GetErrorCode(code)

typedef struct _errorType {
  uint16_t code;
  string description;
} errorType;

struct NetBandwidthInfo {
  bool isTxValid;
  bool isRxValid;
  uint32_t txTotal;
  uint32_t rxTotal;
};

struct DeviceInfo {
  Variant Network;
  Variant General;
  Variant Firmware;
  Variant Middleware;
  Variant Service;
  Variant Hardware;
  Variant Config;
  string NVRamSection;
  string ScriptPath;
};

struct DeviceStatus {
  Variant Video;
  Variant Audio;
  Variant IO;
};

class StreamsManager;

class SystemManager {
  private:
    static string _CUID;
    static string _privateKey;
    static bool _bNTPSynced;
    static DeviceInfo _deviceInfo;
    static DeviceStatus _deviceStatus;
    static string _externalNetworkIF;
    static errorType _errorMap[];
    static uint64_t _curSyncTime;
    static string _operationMode;

  public:

    static bool UpdateDeviceInfo();
    static uint32_t GetFreeMemory();
    static uint64_t GetSystemTime();
    static bool DoSystemCommand(string cmd, string &output);
    // Network related geter/seter functions
    static Variant& GetNetworkInfo();
    static void UpdateNetworkInfo();
    static void SetExternalNetworkIF(string extIF);
    static string GetExternalIP();
    static string GetExternalIF();
    static string GetExternalMacAddr();
    static string GetNetworkInterfaces();
    static Variant& GetGeneralInfo();
    static Variant& GetFirmwareInfo();
    static Variant& GetMiddlewareInfo();
    static Variant& GetServiceInfo();
    static Variant& GetHardwareInfo();
    // Lua configuration related functions
    static Variant& GetConfigInfo(string key);
    static void SetConfigInfo(string key, Variant &config);
    static string GetNVRamSection();
    static void SetNVRamSection(string path);
    static string GetNVRam(const string &section, const string &key);
    static string GetNVRam(const string &key);
    static bool SetNVRam(const string &section, const string &key,
                         const string &value, bool bSync);
    static bool SetNVRam(const string &key, const string &value, bool bSync=true);
    static bool SetSystemTime(uint64_t ts,  string tz);
    static bool SetSystemTime(string tz);
    static bool SetSystemTime(uint64_t ts);

    // TODO: should read from eeprom, need to write a SN in eeprom
    static string GetDeviceSN();
    static bool SetDeviceSN(string deviceSN);
    static bool IsCloudRegistered();
    static bool SetPrivateKey(string privateKey);
    static string GetPrivateKey();
    static bool SetCUID(string cuid);
    static string GetCUID();
    static string GetHashCode();
    static string GetEventId();
    static bool IsNTPSynced();
    static bool IsFactoryMode();

    // GetError description
    static const string& GetErrorDescription(uint16_t index) {
      return _errorMap[index].description;
    }
    // GetError code
    static const uint16_t GetErrorCode(uint16_t index) {
      return _errorMap[index].code;
    }

    static void SetSyncTime(uint64_t syncTime) {
      _curSyncTime=syncTime;
    }

    static uint64_t GetSyncTime() {
      return _curSyncTime;
    }
};

#endif
