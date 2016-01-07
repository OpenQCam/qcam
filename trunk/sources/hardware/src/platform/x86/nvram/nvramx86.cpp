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
// // Author  :
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//

// Includes
#include "hardwaremanager.h"
#include "system/systemmanager.h"
#include "nvram/nvramx86.h"
#include "utils/misc/variant.h"
// Constants


// log
//#define HAS_NVRAM_DEBUG
#ifdef HAS_NVRAM_DEBUG
#define NVRAM_FINE FINE
#define NVRAM_DEBUG DEBUG
#define NVRAM_INFO INFO
#else
#define NVRAM_FINE(...)
#define NVRAM_DEBUG(...)
#define NVRAM_INFO(...)
#endif


BaseHardwareAbstractDevice *NVRamX86::_pDeviceInstance = NULL;

BaseHardwareAbstractDevice* NVRamX86::GetInstance(string filePath) {
  if(_pDeviceInstance == NULL){
    _pDeviceInstance = new NVRamX86(filePath);
    if(!_pDeviceInstance->InitializeDevice()){
      delete _pDeviceInstance;
      return NULL;
    }
  }
  return _pDeviceInstance;
}

// move all initialization code in this function
bool NVRamX86::InitializeDevice() {
  NVRAM_FINE("NVRAM InitializeDevice start");
  if(!RefreshNVRam()){
    return false;
  }
  //INFO("%s", STR(_infoVar.ToString()));
  NVRAM_FINE("NVRAM InitializeDevice done");
  return true;
}

NVRamX86::NVRamX86(string filePath)
: BaseNVRamDevice(),
  _setTimes(0)
{
  _filePath = filePath;
}

NVRamX86::~NVRamX86()
{
#if (defined(HAS_TINYXML))
  if(_infoVar.SerializeToXmlFile(_filePath) ){  // Write to file
    DEBUG("Serialize nvram data to xml file %s", STR(_filePath));
  }
#else
  FATAL("Unable to serialize to XML, must enable tinyXML");
#endif
}




// Private function
bool NVRamX86::WriteToNVRam()
{
#if (defined(HAS_TINYXML))
  DEBUG("write out nvram cache into xml file");
  if(_infoVar.SerializeToXmlFile(_filePath) ){  // Write to file
    INFO("Write content into %s", _filePath.c_str());
    return true;
  }
  else{
    FATAL("Unable to write content, please check the filepath:%s", _filePath.c_str());
    return false;
  }
#else
  FATAL("Unable to serialize to XML, must enable tinyXML");
  return false;
#endif
}

void NVRamX86::CheckStatus(double ts) {
  NYI;
}

// Public function
bool NVRamX86::Set(const string &key, const string &value, bool bSync) {
  return Set(SystemManager::GetNVRamSection(), key, value, bSync);
}

bool NVRamX86::Set(const string &section, const string &key,
                   const string &value, bool bSync) {
  DEBUG("NVRam Set: %s %s %s", section.c_str(), key.c_str(), value.c_str());
  _infoVar[section][key] = value;

  _setTimes++;
  if(_setTimes > 10) {
    WriteToNVRam();
    _setTimes = 0;
  }

  return true;
}

bool NVRamX86::Get(const string &key, string &value) {
  return Get(SystemManager::GetNVRamSection(), key, value);
}

bool NVRamX86::Get(const string &section, const string &key, string &value) {
  // string command = "nvram_get rtdev " + section;
  //DEBUG("NVRam Get: %s %s", section.c_str(), key.c_str());

  if(_infoVar.HasKey(section) ){
    if(_infoVar[section].HasKey(key)){
      value = STR(_infoVar[section][key]);
      //DEBUG("%s", value.c_str());
      return true;
    } else{
      FATAL("cannot found the value in NVRam:%s", key.c_str());
      return false;
    }
  }
  return false;
}

bool NVRamX86::RefreshNVRam(){
#if (defined(HAS_TINYXML))
  if(!Variant::DeserializeFromXmlFile(_filePath, _infoVar)){  // Read from file
    INFO("cannot found the previous file %s for reading nvram setting, ignored", _filePath.c_str());
    return false;
  }
  return true;
#else
  FATAL("Unable to deserialize from XML, must enable tinyXML");
  return false;
#endif
}

void NVRamX86::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                             uint32_t dataLength) {
  NYI;
}

void NVRamX86::GetAll(Variant& value) {
  value=_infoVar;
}

bool NVRamX86::Close() {
  NYI;
  return true;
}
