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
 *  =================================================================
 */

// Includes
// thelib
#include "netio/netio.h"
// hardware
#include "hardwaremanager.h"
#include "system/systemmanager.h"
#include "system/systemdefine.h"
#include "nvram/nvrammt7620.h"

// Constants
BaseHardwareAbstractDevice *NVRamCL1830::_pDeviceInstance = NULL;

BaseHardwareAbstractDevice* NVRamCL1830::GetInstance() {
  if(_pDeviceInstance == NULL){
    _pDeviceInstance = new NVRamCL1830();
    if(!_pDeviceInstance->InitializeDevice()){
      delete _pDeviceInstance;
      return NULL;
    }
  }
  return _pDeviceInstance;
}

// move all initialization code in this function
bool NVRamCL1830::InitializeDevice() {
  DEBUG("NVRAM InitializeDevice start");
  if(!RefreshNVRam()){
    return false;
  }
  DEBUG("NVRAM InitializeDevice done");
  return true;
}

NVRamCL1830::NVRamCL1830()
: BaseNVRamDevice()
{
}

NVRamCL1830::~NVRamCL1830()
{
  DEBUG("~NVRamCL1830");
}

// Private function
// Public function
void NVRamCL1830::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                uint32_t dataLength) {

  string result((char*)pData, dataLength);
  if (MAP_HAS1(_cmdTable, msgId)) {
    string key=_cmdTable[msgId];
    DEBUG ("update nvram table key:%s, value:%s", STR(key), STR(result));
    _nvramSetting[key] = result;
    _cmdTable.erase(msgId);
  }
  else {
    FATAL ("Set NVRam failed. msgId:%d, value:%s", msgId, STR(result));
    NotifyObservers(NV_SET_FAILURE, "nvram set failure");
  }
}

bool NVRamCL1830::Set(const string &section, const string &key,
                      const string &value, bool bSync) {
  DEBUG("NVRam Set: %s %s %s %d", STR(section), STR(key), STR(value), bSync);
  // 1. check if cache is hit
  if(_nvramSetting.HasKey(key)){
    // same value, return
    if(_nvramSetting[key] == value){
      return true;
    }
  }
  // 2. not hit or different value, update nvram
  string result;
  string cmd=format("%s %s %s %s ", ICAM_SET_NVRAM, STR(section), STR(key), STR(value));
  if (bSync) {
    SystemManager::DoSystemCommand(format("%s %s %s %s ", ICAM_SET_NVRAM, STR(section), STR(key), STR(value)), result);
    if(result != value){
      FATAL("nvram failed: key=%s, value=%s, result=%s", STR(key), STR(value), STR(result));
      return false;
    }
    _nvramSetting[key] = value;
  }
  else {
    uint32_t msgId=UnixDomainSocketManager::ExecuteSystemCommand(cmd, GetCallbackId());
    _cmdTable[msgId] = key;
  }
  // 3. done. update variant for cache
  return true;

}

bool NVRamCL1830::Set(const string &key, const string &value, bool bSync)
{
  return Set(SystemManager::GetNVRamSection(), key, value, bSync);
}


bool NVRamCL1830::Get(const string &section, const string &key, string &value)
{
  if(_nvramSetting.HasKey(key)){
    value = (string)_nvramSetting[key];
//    DEBUG("NVRam Get: %s %s", STR(key), STR(value));
    return true;
  }
  string result = "";
  SystemManager::DoSystemCommand(format("%s %s %s", ICAM_GET_NVRAM, STR(section), STR(key)), result);
  value = result;
  // update variant for cache
  _nvramSetting[key] = result;
  DEBUG("No key exists, NVRam Get: %s %s", STR(key), STR(value));
  return true;
}

bool NVRamCL1830::Get(const string &key, string &value)
{
  return Get(SystemManager::GetNVRamSection(), key, value);
}

bool NVRamCL1830::RefreshNVRam() {
  Variant tmp;
  string dummy;
  string filePath = "/tmp/nvram_setting.lua";
  SystemManager::DoSystemCommand(format("icam_all_setting file %s 2>/dev/null", STR(filePath)), dummy);
  if(!ReadLuaFile(filePath, "", tmp)){
    FATAL("Unable to get nvram setting");
    return false;
  }
  uint32_t index = 1;
  _nvramSetting = tmp[index];
  //INFO("%s", STR(_nvramSetting.ToString()));

  //string tmpStr;
  //uint32_t start=0;
  //SystemManager::DoSystemCommand("icam_all_setting json", tmpStr);
  //if(!Variant::DeserializeFromJSON(tmpStr, _nvramSetting, start)){
  //  FATAL("Unable to get nvram setting");
  //  return false;
  //}
  return true;
}

void NVRamCL1830::GetAll(Variant& value) {
  value=_nvramSetting;
}
void NVRamCL1830::CheckStatus (double ts) {
//  NYI;
}

bool NVRamCL1830::Close() {
}
