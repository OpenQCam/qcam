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

#ifdef HAS_PROTOCOL_CLI

#include "actionhandler/networkactionhandler.h"
#include "clidefine.h"

#include "application/baseclientapplication.h"
#include "hardwaremanager.h"

#include <sys/time.h>
#include <arpa/inet.h>

#include "network/networkmanager.h"
#include "system/systemmanager.h"
#include "system/systemdefine.h"
#include "system/eventdefine.h"
#include "system/systeminfo.h"
#include "video/videodefine.h"
#include "video/basev4l2videocapture.h"
#include "gpio/basegpiodevice.h"


using namespace app_qicstreamer;


NetworkActionHandler::NetworkActionHandler(string version, BaseClientApplication *pApplication)
: BaseActionHandler(AT_NETWORK, version, pApplication){
}

NetworkActionHandler::~NetworkActionHandler() {
  (*_actionMaps[QIC_CLI_NETWORK_MANAGEMENT])[true]->clear();
  (*_actionMaps[QIC_CLI_NETWORK_MANAGEMENT])[false]->clear();
  (*_actionMaps[QIC_CLI_NETWORK_WIFI])[true]->clear();
  (*_actionMaps[QIC_CLI_NETWORK_WIFI])[false]->clear();
  _actionMaps[QIC_CLI_NETWORK_MANAGEMENT]->clear();
  _actionMaps[QIC_CLI_NETWORK_WIFI]->clear();
  _actionMaps.clear();
}

bool NetworkActionHandler::Initialize() {
  CollectionMap *pManagementMap= new CollectionMap;
  CollectionMap *pWifiMap= new CollectionMap;
  ActionMap *pManagementGetActionMap = new ActionMap;
  ActionMap *pManagementPostActionMap = new ActionMap;
  ActionMap *pWifiGetActionMap = new ActionMap;
  ActionMap *pWifiPostActionMap = new ActionMap;

  (*pManagementMap)[true] = pManagementGetActionMap;
  (*pManagementMap)[false] = pManagementPostActionMap;
  (*pWifiMap)[true] = pWifiGetActionMap;
  (*pWifiMap)[false] = pWifiPostActionMap;

  _actionMaps[QIC_CLI_NETWORK_MANAGEMENT] = pManagementMap;
  _actionMaps[QIC_CLI_NETWORK_WIFI] = pWifiMap;

  //Management API
  (*pManagementGetActionMap)[QIC_CLI_NETWORK_MANAGEMENT_ID_INTERFACES] =
    reinterpret_cast<Action>(&NetworkActionHandler::GetNetworkIFs);
  (*pManagementGetActionMap)[QIC_CLI_NETWORK_MANAGEMENT_ID_EXTERNAL_IP] =
    reinterpret_cast<Action>(&NetworkActionHandler::GetExternalIP);
  (*pManagementGetActionMap)[QIC_CLI_NETWORK_MANAGEMENT_ID_PING] =
    reinterpret_cast<Action>(&NetworkActionHandler::Ping);
  (*pManagementGetActionMap)[QIC_CLI_NETWORK_MANAGEMENT_ID_STATISTICS] =
    reinterpret_cast<Action>(&NetworkActionHandler::GetStatistics);

  (*pManagementGetActionMap)[QIC_CLI_NETWORK_MANAGEMENT_ID_IP_SETTING] =
    reinterpret_cast<Action>(&NetworkActionHandler::GetIPSetting);
  (*pManagementPostActionMap)[QIC_CLI_NETWORK_MANAGEMENT_ID_IP_SETTING] =
    reinterpret_cast<Action>(&NetworkActionHandler::SetIPSetting);

  //Wifi API
  (*pWifiGetActionMap)[QIC_CLI_NETWORK_WIFI_ID_AVAILABLE_INTERFACES] =
    reinterpret_cast<Action>(&NetworkActionHandler::GetWiFiIFs);

  (*pWifiPostActionMap)[QIC_CLI_NETWORK_WIFI_ID_SECURITY] =
    reinterpret_cast<Action>(&NetworkActionHandler::SetWiFiSecurity);
  (*pWifiGetActionMap)[QIC_CLI_NETWORK_WIFI_ID_SECURITY] =
    reinterpret_cast<Action>(&NetworkActionHandler::GetWiFiSecurity);

  (*pWifiGetActionMap)[QIC_CLI_NETWORK_WIFI_ID_SITE_SURVEY] =
    reinterpret_cast<Action>(&NetworkActionHandler::GetSiteSurvey);

  (*pWifiPostActionMap)[QIC_CLI_NETWORK_WIFI_ID_DO_WPS] =
    reinterpret_cast<Action>(&NetworkActionHandler::DoWPS);

  return true;
}

void NetworkActionHandler::OnCmdFinished(uint32_t msgId, uint8_t*pData,
                                        uint32_t dataLength) {
  DEBUG ("OnCmdFinished");
}


bool NetworkActionHandler::IsValidAction(map<string, CollectionMap*> *map,
                                         string collection,
                                         string id,
                                         bool isReadyOnly) {
  return true;
}

RestHTTPCode NetworkActionHandler::DoAction(BaseProtocol *pFrom,
                                           bool isRO,
                                           vector<string> &resource,
                                           Variant &payload,
                                           Variant &message) {
  if(resource.size() < 5) {
    FATAL("IO action URI is not enough...(must include collection and id)");
    return SendFailure(HC_400_BAD_REQUEST, EC_101_INVALID_API_REQUEST,
        "URI is not enough...(must include collection and id)", message);
  }

  string collection = resource[3];
  string id = resource[4];

  if(!MAP_HAS1(_actionMaps, collection)) {
    return SendFailure(HC_400_BAD_REQUEST, EC_106_COLLECTION_NOT_FOUND,
        "Unable to find collection: " + collection, message);
  }
  if(!MAP_HAS1(*_actionMaps[collection], isRO)){
    return SendFailure(HC_400_BAD_REQUEST, EC_103_METHOD_NOT_ALLOWED,
        "Invalid method: " + isRO, message);
  }
  if(!MAP_HAS1(*(*_actionMaps[collection])[isRO], id)){
    return SendFailure(HC_400_BAD_REQUEST, EC_107_RESOURCE_ID_NOT_FOUND,
        "Unable to find resource id: " + id, message);
  }

  Action pAction = (*(*_actionMaps[collection])[isRO])[id];
  return ((this->*pAction)(payload[REST_PARAM_STR], message));
}

// API Functions
// Management API
RestHTTPCode NetworkActionHandler::GetNetworkIFs(Variant &params, Variant &message) {
  //message = SystemManager::GetNetworkInterfaces();
  if(!fileExists("/sbin/icam_get_network_interfaces")){
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_900_NETWORK_ERROR,
        "Unable to find command icam_get_network_interfaces", message);
  }
  string output;
  uint32_t start = 0;
  SystemManager::DoSystemCommand("icam_get_network_interfaces", output);

  Variant result;
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("error")){
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_900_NETWORK_ERROR, result["error"], message);
  }

  message = result;
  return HC_200_OK;
}

RestHTTPCode NetworkActionHandler::GetIPSetting(Variant &params, Variant &message) {
  //message = SystemManager::GetNetworkInterfaces();
  if(!fileExists("/sbin/icam_get_ip_setting")){
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_900_NETWORK_ERROR,
        "Unable to find command icam_get_ip_setting", message);
  }
  string output;
  uint32_t start = 0;
  SystemManager::DoSystemCommand("icam_get_ip_setting", output);

  Variant result;
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("error")){
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_900_NETWORK_ERROR, result["error"], message);
  }

  message = result;
  return HC_200_OK;
}

RestHTTPCode NetworkActionHandler::SetIPSetting(Variant &params, Variant &message) {
  string cmd = "/sbin/icam_set_ip_setting";
  string ip, subnet;
  string output;
  uint32_t start = 0;

  if(!fileExists(cmd)){
    //FIXME: error code for no system command ?
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_900_NETWORK_ERROR,
        "Unable to find command:" + cmd, message);
  }

  //IP (dhcp or valid_ip)
  if(params.HasKey("ip")){
    ip = (string)params["ip"];
    // Check ip address format is valid or not
    struct sockaddr_in sa;
    if(inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) == 0){
      if(ip.compare("dhcp") != 0){
        return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_108_INVALID_PARAMETERS,
            "invalid ip address", message);
      }
      else{  //DHCP
        cmd = cmd + " dhcp ";
      }
    }
    else{ //Static IP
      //SUBNET MASK
      if(params.HasKey("subnet")){
        subnet = (string)params["subnet"];
        // Check subnet format is valid or not
        struct sockaddr_in sa;
        if(inet_pton(AF_INET, subnet.c_str(), &(sa.sin_addr)) == 0){
          return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_108_INVALID_PARAMETERS,
              "invalid subnet mask", message);
        }
      }
      else{
        FATAL("parameters subnet is undefined");
        return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
          "parameters subnet is undefined", message);
      }
      cmd = cmd + " static " + ip + " " + subnet;
    }
  }
  else{
    FATAL("parameters ip is undefined");
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "parameters ip is undefined", message);
  }

  SystemManager::DoSystemCommand(cmd, output);
  Variant result;
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("error")){
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS, result["error"], message);
  }
  else if(!result.HasKey("success")){
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_900_NETWORK_ERROR, "no responce...", message);
  }
  message = result;
  return HC_200_OK;
}

RestHTTPCode NetworkActionHandler::GetWiFiIFs(Variant &params, Variant &message) {
  if(!fileExists("/sbin/icam_get_wifi_interfaces")){
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_900_NETWORK_ERROR,
        "Unable to find command icam_get_wifi_interfaces", message);
  }
  string output;
  uint32_t start = 0;
  SystemManager::DoSystemCommand("icam_get_wifi_interfaces", output);

  Variant result;
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("error")){
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_900_NETWORK_ERROR, result["error"], message);
  }

  message = result;
  return HC_200_OK;
}

RestHTTPCode NetworkActionHandler::GetExternalIP(Variant &params, Variant &message) {
  string ip = SystemManager::GetExternalIP();
  if(ip.length() == 0){
    return SendFailure(HC_400_BAD_REQUEST, EC_901_NETWORK_INTERFACE_NOT_FOUND,
        "Unable to find external interface", message);
  }
  message["ip"] = ip;
  return HC_200_OK;
}

RestHTTPCode NetworkActionHandler::Ping(Variant &params, Variant &message) {
  string interface;
  string output;
  uint32_t start = 0;

  if(!fileExists("/sbin/icam_ping_ip")){
    //FIXME: error code for no system command ?
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_900_NETWORK_ERROR,
        "Unable to find command icam_ping_ip", message);
  }

  //IP
  string ip, packets, expiredTime;
  if(params.HasKey("ip")){
    ip = (string)params["ip"];
    // Check ip address format is valid or not
    struct sockaddr_in sa;
    if(inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) == 0){
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_108_INVALID_PARAMETERS,
          "invalid ip address", message);
    }
  }
  else{
    FATAL("parameters ip is undefined");
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "parameters ip is undefined", message);
  }

  if(params.HasKey("packets")){
    packets = (string) params["packets"];
    if(!isNumeric(packets)){
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_108_INVALID_PARAMETERS,
        "Must set numeric value for packets", message);
    }
  }
  else{
    packets = "1";
  }

  if(params.HasKey("expiredTime")){
    expiredTime = (string) params["expiredTime"];
    if(!isNumeric(expiredTime)){
      return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_108_INVALID_PARAMETERS,
        "Must set numeric value for expiredTime", message);
    }
  }
  else{
    expiredTime = "1";
  }

  string cmd = "icam_ping_ip " + ip + " " + packets + " " + expiredTime;
  SystemManager::DoSystemCommand(cmd, output);

  Variant result;
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("error")){
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_900_NETWORK_ERROR, result["error"], message);
  }

  message = result;
  return HC_200_OK;
}

RestHTTPCode NetworkActionHandler::GetStatistics(Variant &params, Variant &message) {
  if(!fileExists("/sbin/icam_get_network_statistics")){
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_900_NETWORK_ERROR,
        "Unable to find command icam_get_network_statistics", message);
  }
  string output;
  uint32_t start = 0;
  SystemManager::DoSystemCommand("icam_get_network_statistics", output);

  Variant result;
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("error")){
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_900_NETWORK_ERROR, result["error"], message);
  }

  message = result;
  return HC_200_OK;
}


// WiFi API
RestHTTPCode NetworkActionHandler::GetSiteSurvey(Variant &params, Variant &message) {
  if(!fileExists("/sbin/icam_site_survey")){
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_900_NETWORK_ERROR,
        "Unable to find command icam_site_survey", message);
  }
  string output;
  uint32_t start = 0;
  SystemManager::DoSystemCommand("icam_site_survey", output);

  Variant result;
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("error")){
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_900_NETWORK_ERROR, result["error"], message);
  }

  message = result;
  return HC_200_OK;
}

RestHTTPCode NetworkActionHandler::SetWiFiSecurity(Variant &params, Variant &message) {
  string cmd = "/sbin/icam_set_wifi_security";
  string name, ssid, mode, encrypt, key, wps;
  string output;
  uint32_t i, interfaces_size;

  if(!fileExists(cmd)){
    //FIXME: error code for no system command ?
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_900_NETWORK_ERROR,
        "Unable to find command:" + cmd, message);
  }

  //Interface
  if(!params.HasKey("interfaces")){
    return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
        "parameters interfaces is undefined", message);
  }

  if (params["interfaces"]!=V_MAP) {
    return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
        "parameter interfaces is invalid", message);
  }

  Variant interfaces = params["interfaces"];
  interfaces_size = interfaces.MapSize();

  for (i=0; i <interfaces_size; i++){
    cmd = "/sbin/icam_set_wifi_security ";
    //Name
    if(interfaces[i].HasKey("name")){
      name = (string) interfaces[i]["name"];
      if( (name.compare("5g") != 0) && (name.compare("2.4g") != 0) ){
        FATAL("parameters name is invalid:%s", name.c_str());
        return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
            "parameters name is invalid", message);
      }
    }else{
      FATAL("parameters name is undefined");
      return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
          "parameters name is undefined", message);
    }

    //SSID
    if(interfaces[i].HasKey("ssid")){
      ssid = (string) interfaces[i]["ssid"];
    }else{
      FATAL("parameters ssid is undefined");
      return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
          "parameters ssid is empty", message);
    }

    //Security Mode
    if(interfaces[i].HasKey("mode")){
      mode = (string) interfaces[i]["mode"];
    }else{
      mode = "?";
    }

    //Encryption Type
    if(interfaces[i].HasKey("encrypt")){
      encrypt = (string) interfaces[i]["encrypt"];
    }
    else{
      encrypt ="?";
    }

    //Pass Phrase Key
    if(interfaces[i].HasKey("key")){
      key = (string) interfaces[i]["key"];
    }
    else{
      key = "?";
    }

    //WPS enable
    if(interfaces[i].HasKey("wps")){
      wps = (string) interfaces[i]["wps"];
    }
    else{
      wps = " ";
    }

    cmd = cmd + " " + name + " " + ssid + " " + mode + " " + " " + encrypt + " " + key + " " + wps;
    //DEBUG("cmd: %s", cmd.c_str());

    Variant result;
    uint32_t start = 0;
    SystemManager::DoSystemCommand(cmd, output);
    Variant::DeserializeFromJSON(output, result, start);
    //DEBUG("result:%s", STR(result.ToString()));
    if(result.HasKey("error")){
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS, result["error"], message);
    }
    else if(!result.HasKey("success")){
      return SendFailure(HC_417_EXPECTATION_FAILED, EC_900_NETWORK_ERROR, "no responce...", message);
    }
    else{
      message = result;
    }
  }
  return HC_200_OK;
}

RestHTTPCode NetworkActionHandler::GetWiFiSecurity(Variant &params, Variant &message) {
  string interface;
  string output;
  uint32_t start = 0;

  if(!fileExists("/sbin/icam_get_wifi_security")){
    //FIXME: error code for no system command ?
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_900_NETWORK_ERROR,
        "Unable to find command icam_get_wifi_security", message);
  }

  string cmd = "icam_get_wifi_security";
  SystemManager::DoSystemCommand(cmd, output);

  Variant result;
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("error")){
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_900_NETWORK_ERROR, result["error"], message);
  }

  message = result;
  return HC_200_OK;
}

RestHTTPCode NetworkActionHandler::DoWPS(Variant &params, Variant &message) {
  string interface;
  string output;
  uint32_t start = 0;

  if(!fileExists("/sbin/icam_wps_pairing")){
    //FIXME: error code for no system command ?
    return SendFailure(HC_500_INTERNAL_SERVER_ERROR, EC_900_NETWORK_ERROR,
        "Unable to find command icam_wps_pairing", message);
  }

  //Interface
  if(params.HasKey("interface")){
    interface = (string) params["interface"];
    if( (interface.compare("5g") != 0) && (interface.compare("2.4g") != 0) ){
      FATAL("parameters interface is invalid:%s", interface.c_str());
      return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
          "parameters interface is invalid", message);
    }
  }else{
    FATAL("parameters interface is undefined");
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
        "parameters interface is undefined", message);
  }

  string cmd = "icam_wps_pairing " + interface;
  SystemManager::DoSystemCommand(cmd, output);

  Variant result;
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("error")){
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_900_NETWORK_ERROR, result["error"], message);
  }

  message = result;
  return HC_200_OK;
}

//Debug only API
RestHTTPCode NetworkActionHandler::ExecuteCommand(Variant &parameters, Variant &message) {
  return HC_200_OK;
}



#endif
