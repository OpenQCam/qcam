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

#include "system/systemmanager.h"
//thelib
#include "netio/netio.h"
#include "version.h"
#include "system/systemdefine.h"
#include "system/nvramdefine.h"
#include "streaming/streamsmanager.h"
#include "streaming/streamstypes.h"
#include "streaming/inmsgstream.h"
#include "protocols/cli/restclidefine.h"
#include "protocols/avhttpstream/avstreamdefine.h"
#include "protocols/unixdomainsocket/unixdomainsocketprotocol.h"
#include "protocols/unixdomainsocket/unixdomainsocketprotocol.h"
//hardware
#include "hardwaremanager.h"
#include "network/networkmanager.h"
#include "nvram/basenvramdevice.h"
//primitive
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>



DeviceInfo SystemManager::_deviceInfo;
DeviceStatus SystemManager::_deviceStatus;

string SystemManager::_externalNetworkIF;
string SystemManager::_CUID;
string SystemManager::_privateKey;
string SystemManager::_operationMode;
bool SystemManager::_bNTPSynced=false;
uint64_t SystemManager::_curSyncTime=0;

errorType SystemManager::_errorMap[]= {
#include "system/error.h"
};


bool SystemManager::DoSystemCommand(string cmd, string &output)
{
  FILE *fp;
  char buf[2048];

  output.clear();

  //Open the command for reading.
  fp = popen(cmd.c_str(), "r");
  if (fp == NULL) {
    DEBUG("Failed to run command\n");
    return false;
  }
  //Read the output a line at a time - output it.
  while (fgets(buf, sizeof(buf)-1, fp) != NULL) {
    output = output + buf;
  }
  //close
  //printf ("output:%s", output.c_str());
  pclose(fp);

  return true;
}

bool SystemManager::UpdateDeviceInfo() {
  BaseNVRamDevice *pNVRam = (BaseNVRamDevice *) HardwareManager::GetHardwareInstance(HT_NVRAM);
  return pNVRam->RefreshNVRam();
}

uint64_t SystemManager::GetSystemTime(){
  struct timeval ts;
  gettimeofday(&ts,NULL);
  return ((uint64_t)ts.tv_sec*1000+(uint64_t)(ts.tv_usec/1000));
}

bool SystemManager::SetSystemTime(string tz) {
  SystemManager::SetNVRam("NtpTimeZone", tz, false);
  //1. generate tz file
  string dontcare;
  SystemManager::DoSystemCommand(format("echo \"%s\" > /etc/TZ", STR(tz)), dontcare);
  //2. generate gmtoff file
  time_t utctime=time(NULL);
  struct tm *lxtime=localtime(&utctime);
  SystemManager::DoSystemCommand(format("echo \"%u\" > /etc/LT", (uint32_t)lxtime->tm_gmtoff), dontcare);
  return true;
}

bool SystemManager::SetSystemTime(uint64_t ts) {
  struct timeval tv;

  gettimeofday(&tv, 0);
  tv.tv_sec = (time_t)ts;
  settimeofday(&tv, 0);
  IOHandlerManager::UpdateTimerManagerLastTime();
  _bNTPSynced=true;
  return true;
}

bool SystemManager::SetSystemTime(uint64_t ts,  string tz) {
  struct timeval tv;

  gettimeofday(&tv, 0);
  tv.tv_sec = (time_t)ts;
  settimeofday(&tv, 0);
  IOHandlerManager::UpdateTimerManagerLastTime();

  SystemManager::SetNVRam("NtpTimeZone", tz, false);
  string dontcare;
  SystemManager::DoSystemCommand(format("echo \"%s\" > /etc/TZ", STR(tz)), dontcare);
  //2. generate gmtoff file
  time_t utctime=time(NULL);
  struct tm *lxtime=localtime(&utctime);
  SystemManager::DoSystemCommand(format("echo \"%u\" > /etc/LT", (uint32_t)lxtime->tm_gmtoff), dontcare);
  _bNTPSynced=true;
  return true;
}


void SystemManager::SetExternalNetworkIF(string extNetworkIF) {
  _externalNetworkIF = extNetworkIF;
}

void SystemManager::UpdateNetworkInfo() {
  vector<string> netIFs;
  NetworkManager::GetInterface(netIFs);
  _deviceInfo.Network.Reset();
  if(netIFs.size() > 0){
    FOR_VECTOR(netIFs, i){
      Variant netIF;
      string addr;
      //interface
      netIF[DEVICE_NETWORK_INTERFACE] = netIFs[i];
      //ip
      if(NetworkManager::GetInterfaceIP(netIFs[i], addr)){
        netIF[DEVICE_NETWORK_IP] = addr;
      }
      //mac
      if(NetworkManager::GetInterfaceMacAddress(netIFs[i], addr)){
        netIF[DEVICE_NETWORK_MAC] = addr;
      }
      _deviceInfo.Network[DEVICE_NETWORK_INTERFACES].PushToArray(netIF);

      if(netIFs[i] == _externalNetworkIF){
        _deviceInfo.Network[DEVICE_NETWORK_EXTERNAL] = netIF;
      }
    }
  }
  // unable to find external interface
  if(!_deviceInfo.Network.HasKey(DEVICE_NETWORK_EXTERNAL)){
    Variant undefinedExternal;
    undefinedExternal[DEVICE_NETWORK_INTERFACE] = "undefined";
    undefinedExternal[DEVICE_NETWORK_IP] = "undefined";
    undefinedExternal[DEVICE_NETWORK_MAC] = "undefined";
    _deviceInfo.Network[DEVICE_NETWORK_EXTERNAL] = undefinedExternal;
  }
}

string SystemManager::GetExternalMacAddr() {
  if (!_deviceInfo.Network[DEVICE_NETWORK_EXTERNAL].HasKey(DEVICE_NETWORK_MAC)) {
    return "";
  }
  string external_mac;
  NetworkManager::GetInterfaceMacAddress(_externalNetworkIF, external_mac);
  _deviceInfo.Network[DEVICE_NETWORK_EXTERNAL][DEVICE_NETWORK_MAC] = external_mac;
  return (_deviceInfo.Network[DEVICE_NETWORK_EXTERNAL][DEVICE_NETWORK_MAC]);
}


string SystemManager::GetExternalIP() {
  if (!_deviceInfo.Network[DEVICE_NETWORK_EXTERNAL].HasKey(DEVICE_NETWORK_IP)) {
    return "";
  }
  string external_ip;
  NetworkManager::GetInterfaceIP(_externalNetworkIF, external_ip);
  if(_deviceInfo.Network[DEVICE_NETWORK_EXTERNAL][DEVICE_NETWORK_IP] != external_ip){
    _deviceInfo.Network[DEVICE_NETWORK_EXTERNAL][DEVICE_NETWORK_IP] = external_ip;
  }
  return (_deviceInfo.Network[DEVICE_NETWORK_EXTERNAL][DEVICE_NETWORK_IP]);
}

string SystemManager::GetExternalIF() {
  if(!_deviceInfo.Network.HasKey(DEVICE_NETWORK_EXTERNAL)){
    SystemManager::UpdateNetworkInfo();
  }
  return (_deviceInfo.Network[DEVICE_NETWORK_EXTERNAL][DEVICE_NETWORK_INTERFACE]);
}

Variant& SystemManager::GetNetworkInfo() {
  return _deviceInfo.Network;
}

string SystemManager::GetNetworkInterfaces() {
  if(!_deviceInfo.Network.HasKey(DEVICE_NETWORK_INTERFACES)){
    SystemManager::UpdateNetworkInfo();
  }
  return (_deviceInfo.Network[DEVICE_NETWORK_INTERFACES]);
}

Variant& SystemManager::GetFirmwareInfo() {
  string nvramSection = SystemManager::GetNVRamSection();
  _deviceInfo.Firmware[DEVICE_FIRMWARE_IMAGE] =
    SystemManager::GetNVRam(nvramSection, NVRAM_FIRMWARE_IMAGE);
  _deviceInfo.Firmware[DEVICE_FIRMWARE_SDK] =
    SystemManager::GetNVRam(nvramSection, NVRAM_FIRMWARE_SDK);
  _deviceInfo.Firmware[DEVICE_FIRMWARE_DSP] =
    SystemManager::GetNVRam(nvramSection, NVRAM_FIRMWARE_DSP);
  _deviceInfo.Firmware[DEVICE_FIRMWARE_APPLICATION] =
    SystemManager::GetNVRam(nvramSection, NVRAM_FIRMWARE_APPLICATION);
  _deviceInfo.Firmware[DEVICE_FIRMWARE_SCRIPT] =
    SystemManager::GetNVRam(nvramSection, NVRAM_FIRMWARE_SCRIPT);

  //add for cloud request once
  _deviceInfo.Firmware[DEVICE_HARDWARE_MODEL] =
    SystemManager::GetNVRam(nvramSection, NVRAM_MODEL);
  return _deviceInfo.Firmware;
}

Variant& SystemManager::GetMiddlewareInfo() {
  _deviceInfo.Middleware["gitCommitHash"] = GIT_COMMIT_HASH;
  _deviceInfo.Middleware["gitBranch"] = GIT_BRANCH;
  _deviceInfo.Middleware["gitTagRevision"] = GIT_APPLICATION_REVISION;
  return _deviceInfo.Middleware;
}
Variant& SystemManager::GetServiceInfo() {
  return _deviceInfo.Service;
}
Variant& SystemManager::GetHardwareInfo() {
  return _deviceInfo.Hardware;
}

Variant& SystemManager::GetGeneralInfo() {
  string nvramSection = SystemManager::GetNVRamSection();
  _deviceInfo.General[DEVICE_CUID] =
    SystemManager::GetNVRam(nvramSection, NVRAM_CUID);
  _deviceInfo.General[DEVICE_SERIAL_NO] =
    SystemManager::GetNVRam(nvramSection, NVRAM_SERIALNO);
  _deviceInfo.General[DEVICE_HASH_CODE] =
    SystemManager::GetNVRam(nvramSection, NVRAM_HASHCODE);
  _deviceInfo.General[DEVICE_VENDOR] =
    SystemManager::GetNVRam(nvramSection, NVRAM_VENDOR);
  _deviceInfo.General[DEVICE_HARDWARE_MODEL] =
    SystemManager::GetNVRam(nvramSection, NVRAM_MODEL);
  _deviceInfo.General[DEVICE_HARDWARE_VERSION] =
    SystemManager::GetNVRam(nvramSection, NVRAM_VERSION);
  return _deviceInfo.General;
}

Variant& SystemManager::GetConfigInfo(string key) {
  return _deviceInfo.Config[key];
}

void SystemManager::SetConfigInfo(string key, Variant &config) {
  _deviceInfo.Config[key] = config;
}

uint32_t SystemManager::GetFreeMemory() {
  struct sysinfo myinfo;
  uint32_t free_bytes;

  sysinfo (&myinfo);
  free_bytes = myinfo.mem_unit * myinfo.freeram;
  return free_bytes;
}

void SystemManager::SetNVRamSection(string path) {
  _deviceInfo.NVRamSection = path;
}

string SystemManager::GetNVRamSection() {
  return _deviceInfo.NVRamSection;
}

string SystemManager::GetNVRam(const string &section, const string &key) {
  string nvramValue;
  BaseNVRamDevice *pNVRam = (BaseNVRamDevice *) HardwareManager::GetHardwareInstance(HT_NVRAM);
  if (!pNVRam)
    return "";
  if(pNVRam->Get(section, key, nvramValue)){
    return nvramValue;
  }else{
    return "";
  }
}

string SystemManager::GetNVRam(const string &key) {
  return SystemManager::GetNVRam(SystemManager::GetNVRamSection(), key);
}

bool SystemManager::SetNVRam(const string &section, const string &key,
                             const string &value, bool bSync) {
  BaseNVRamDevice *pNVRam =
      (BaseNVRamDevice *) HardwareManager::GetHardwareInstance(HT_NVRAM);
  return (pNVRam->Set(section, key, value, bSync));
}

bool SystemManager::SetNVRam(const string &key, const string &value, bool bSync) {
  //DEBUG ("key:%s, value:%s", STR(key), STR(value));
  return SystemManager::SetNVRam(SystemManager::GetNVRamSection(), key, value, bSync);
}

bool SystemManager::SetDeviceSN(string deviceSN){
  return SystemManager::SetNVRam(NVRAM_SERIALNO, deviceSN);
}

string SystemManager::GetDeviceSN(){
  return SystemManager::GetNVRam(NVRAM_SERIALNO);
}

bool SystemManager::SetPrivateKey(string privateKey) {
  _privateKey=privateKey;
  return SystemManager::SetNVRam(NVRAM_PRIVATEKEY, privateKey, true);
}

string SystemManager::GetPrivateKey() {
  if (_privateKey.empty()) {
    _privateKey=SystemManager::GetNVRam(NVRAM_PRIVATEKEY);
  }
  return _privateKey;
}

bool SystemManager::IsCloudRegistered() {
  if (!_privateKey.empty() && !_CUID.empty())
    return true;
  return false;
}

bool SystemManager::SetCUID(string cuid) {
  _CUID=cuid;
  return SystemManager::SetNVRam(NVRAM_CUID, cuid, true);
}

string SystemManager::GetCUID() {
  if (_CUID.empty()) {
    _CUID=SystemManager::GetNVRam(NVRAM_CUID);
  }
  return _CUID;
}

string SystemManager::GetHashCode() {
  return SystemManager::GetNVRam(NVRAM_HASHCODE);
}

string SystemManager::GetEventId() {
  uint64_t localTS;

  GETLOCALTIMESTAMP(localTS);
  string tsString = format("%013llu", localTS);
  return tsString;
}

bool SystemManager::IsNTPSynced() {
  return _bNTPSynced;
}

bool SystemManager::IsFactoryMode() {
  return true;
  if (_operationMode.empty()) {
    string jsonData;
    Variant mode;
    DoSystemCommand("icam_check_operation_mode", jsonData);
    uint32_t start=0;
    if (Variant::DeserializeFromJSON(jsonData, mode, start)) {
      _operationMode=(string)mode["mode"];
    }
    else {
      return false;
    }
  }
  return (_operationMode=="production");
}
