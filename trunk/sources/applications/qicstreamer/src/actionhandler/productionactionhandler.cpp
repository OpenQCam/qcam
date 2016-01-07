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

#include "hardwaremanager.h"
#include "system/systemmanager.h"
#include "system/systemdefine.h"
#include "actionhandler/productionactionhandler.h"
#include "clidefine.h"
#include <signal.h>

#include "protocols/http/websocket/websocketdefine.h"
#include "protocols/http/outboundhttpwsprotocol.h"
#include "gpio/basegpiodevice.h"

using namespace app_qicstreamer;


ProductionActionHandler::ProductionActionHandler(string version, BaseClientApplication *pApplication)
: BaseActionHandler(AT_PRODUCTION, version, pApplication)
{
}

ProductionActionHandler::~ProductionActionHandler() {
}

bool ProductionActionHandler::Initialize() {


  CollectionMap *pTestMap= new CollectionMap;
  ActionMap *pTestGetActionMap = new ActionMap;
  ActionMap *pTestPostActionMap = new ActionMap;

  (*pTestMap)[true] = pTestGetActionMap;
  (*pTestMap)[false] = pTestPostActionMap;

  _actionMaps[QIC_CLI_IO_TEST] = pTestMap;

  // action functions
  /*Gernal Command*/
  /*Production Test API*/
  //Get API
  (*pTestGetActionMap)[QIC_CLI_IO_TEST_COMMAND] = reinterpret_cast<Action>(&ProductionActionHandler::TestCommand);
  //Post API
  (*pTestPostActionMap)[QIC_CLI_IO_TEST_COMMAND] = reinterpret_cast<Action>(&ProductionActionHandler::TestCommand);

#if 0
  /*LED*/
  _testActionMap[QIC_CLI_IO_TEST_LED] = reinterpret_cast<Action>(&ProductionActionHandler::TestLED);
  /*Button*/
  _testActionMap[QIC_CLI_IO_TEST_BUTTON] = reinterpret_cast<Action>(&ProductionActionHandler::TestButton);
  /*SWITCH*/
  _testActionMap[QIC_CLI_IO_TEST_SWITCH] = reinterpret_cast<Action>(&ProductionActionHandler::TestSwitch);
  /*SD Card&Slot*/
  _testActionMap[QIC_CLI_IO_TEST_SD] = reinterpret_cast<Action>(&ProductionActionHandler::TestSD);
  /*Flash Partition*/
  _testActionMap[QIC_CLI_IO_TEST_FLASH] = reinterpret_cast<Action>(&ProductionActionHandler::TestFlash);
#endif

  return true;
}

bool ProductionActionHandler::IsValidAction(map<string, CollectionMap*> *map,
                                         string collection,
                                         string id,
                                         bool isReadyOnly) {
  return true;
}

RestHTTPCode ProductionActionHandler::DoAction(BaseProtocol *pFrom,
                                            bool isRO,
                                            vector<string> &resource,
                                            Variant &payload,
                                            Variant &message) {
  string collection = resource[3];
  string id = resource[4];

  if(!MAP_HAS1(_actionMaps, collection)) {
    return SendFailure(HC_400_BAD_REQUEST, EC_106_COLLECTION_NOT_FOUND,
        "Unable to find collection: " + collection, message);
  }
  if(!MAP_HAS1(*_actionMaps[collection], isRO)){
    return SendFailure(HC_400_BAD_REQUEST, EC_107_RESOURCE_ID_NOT_FOUND,
        "Invalid method: " + isRO, message);
  }
  if(!MAP_HAS1(*(*_actionMaps[collection])[isRO], id)){
    return SendFailure(HC_400_BAD_REQUEST, EC_107_RESOURCE_ID_NOT_FOUND,
        "Unable to find resource id: " + id, message);
  }

  Action pAction = (*(*_actionMaps[collection])[isRO])[id];
  return ((this->*pAction)(payload[REST_PARAM_STR], message));
}

// Public Functions
/*General Command*/
RestHTTPCode ProductionActionHandler::TestCommand(Variant &parameters, Variant &message) {
  DEBUG("Generl Test Command");
  uint32_t start = 0;
  string output;
  string cmd;
  string p1 = " ", p2 = " ", p3 = " ", p4 = " ", p5 = " ", p6= " ", p7 =" ";

  if(parameters.HasKey("cmd")){
    cmd = (string) parameters["cmd"];
    if(!fileExists( ((string) "/sbin/"+ cmd) ) && cmd.compare("icam_test_ack") != 0){
      //FATAL("Unable to find command: " + cmd);
      return SendFailure(HC_400_BAD_REQUEST, EC_108_INVALID_PARAMETERS,
          ("Unable to find command: " + cmd), message);
    }
  }
  else{
    FATAL("parameters cmd is undefined");
    return SendFailure(HC_400_BAD_REQUEST, EC_109_MISSING_MANDATORY_PARAMETERS,
          ("parameters cmd is undefined"), message);
  }

  if(parameters.HasKey("p1")){
    p1 = (string) parameters["p1"];
    message["p1"]= p1;
  }
  if(parameters.HasKey("p2")){
    p2 = (string) parameters["p2"];
    message["p2"]= p2;
  }
  if(parameters.HasKey("p3")){
    p3 = (string) parameters["p3"];
    message["p3"]= p3;
  }
  if(parameters.HasKey("p4")){
    p4 = (string) parameters["p4"];
    message["p4"]= p4;
  }
  if(parameters.HasKey("p5")){
    p5 = (string) parameters["p5"];
    message["p5"]= p5;
  }
  if(parameters.HasKey("p6")){
    p6 = (string) parameters["p6"];
    message["p6"]= p6;
  }
  if(parameters.HasKey("p7")){
    p7 = (string) parameters["p7"];
    message["p7"]= p7;
  }

  string command = cmd + " " + p1 + " " + p2 + " " + p3 + " " + p4 + " " + p5 + " " + p6 + " " +p7;
  SystemManager::DoSystemCommand(command, output);

  message["cmd"] = cmd;

  FATAL("cmd: %s", STR(cmd));
  FATAL("output: %s", STR(output));

  Variant result;
  Variant::DeserializeFromJSON(output, result, start);
  if(result.HasKey("error")){
    if(result.HasKey("data")){
      message["data"] = result["data"];
    }
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_108_INVALID_PARAMETERS, result["error"], message);
  }
  else{
    message = result;
    return HC_200_OK;
  }
  return HC_200_OK;
}

#if 0
RestHTTPCode ProductionActionHandler::TestLED(Variant &parameters, Variant &message) {
  DEBUG("Test LED");
  string output;
  string type, state;

  if(parameters.HasKey("type")){
    type = (string) parameters["type"];
  }
  else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_301_INVALID_PARAMETERS,"type is undefined", message);
  }

  if(parameters.HasKey("state")){
    state = (string) parameters["state"];
  }
  else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_301_INVALID_PARAMETERS,"state is undefined", message);
  }

  string cmd="icam_test_led " + type + " " + state;
  SystemManager::DoSystemCommand(cmd, output);

  message["type"]= type;
  message["state"]= state;
  if(message.HasKey("error")){
    return HC_417_EXPECTATION_FAILED;
  }
  else{
    return HC_200_OK;
  }
   return HC_200_OK;
}

RestHTTPCode ProductionActionHandler::TestButton(Variant &parameters, Variant &message) {
  DEBUG("Test Button");
  string output;
  string type, state, timeout;

  if(parameters.HasKey("type")){
    type = (string) parameters["type"];
  }
  else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_301_INVALID_PARAMETERS,"type is undefined", message);
  }

  if(parameters.HasKey("state")){
    state = (string) parameters["state"];
  }
  else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_301_INVALID_PARAMETERS,"state is undefined", message);
  }

  if(parameters.HasKey("timeout")){
    timeout = (string) parameters["timeout"];
  }
  else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_301_INVALID_PARAMETERS,"timeout is undefined", message);
  }

  string cmd="icam_test_button " + type + " " + state + " " + timeout;
  SystemManager::DoSystemCommand(cmd, output);

  message["type"]= type;
  message["state"]= state;
  message["timeout"]= timeout;
  if(message.HasKey("error")){
    return HC_417_EXPECTATION_FAILED;
  }
  else{
    return HC_200_OK;
  }
  return HC_200_OK;
}

RestHTTPCode ProductionActionHandler::TestSwitch(Variant &parameters, Variant &message) {
  DEBUG("Test Switch");
  string output;
  string state, timeout;

  if(parameters.HasKey("state")){
    state = (string) parameters["state"];
  }
  else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_301_INVALID_PARAMETERS,"state is undefined", message);
  }

  if(parameters.HasKey("timeout")){
    timeout = (string) parameters["timeout"];
  }
  else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_301_INVALID_PARAMETERS,"timeout is undefined", message);
  }

  string cmd="icam_test_switch " + state + " " + timeout;
  SystemManager::DoSystemCommand(cmd, output);

  message["state"]= state;
  message["timeout"]= timeout;
  if(message.HasKey("error")){
    return HC_417_EXPECTATION_FAILED;
  }
  else{
    return HC_200_OK;
  }
}


RestHTTPCode ProductionActionHandler::TestSD(Variant &parameters, Variant &message) {
  DEBUG("Test SD");
  string output;
  string url, sha1sum;

  if(parameters.HasKey("url")){
    url = (string) parameters["url"];
  }
  else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_301_INVALID_PARAMETERS,"url is undefined", message);
  }

  if(parameters.HasKey("sha1sum")){
    sha1sum = (string) parameters["sha1sum"];
  }
  else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_301_INVALID_PARAMETERS,"sha1sum is undefined", message);
  }

  string cmd="icam_test_sd " + url + " " + sha1sum;
  SystemManager::DoSystemCommand(cmd, output);

  message["url"]= url;
  message["sha1sum"]= sha1sum;
  if(message.HasKey("error")){
    return HC_417_EXPECTATION_FAILED;
  }
  else{
    return HC_200_OK;
  }
}

RestHTTPCode ProductionActionHandler::TestFlash(Variant &parameters, Variant &message) {
  DEBUG("Test Flash");
  string output;
  string partition, sha1sum;

  if(parameters.HasKey("partition")){
    partition = (string) parameters["partition"];
  }
  else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_301_INVALID_PARAMETERS,"partition is undefined", message);
  }

  if(parameters.HasKey("sha1sum")){
    sha1sum = (string) parameters["sha1sum"];
  }
  else{
    return SendFailure(HC_417_EXPECTATION_FAILED, EC_301_INVALID_PARAMETERS,"sha1sum is undefined", message);
  }

  string cmd="icam_check_flash_sha1sum " + partition + " " + sha1sum;
  SystemManager::DoSystemCommand(cmd, output);

  message["partition"]= partition;
  message["sha1sum"]= sha1sum;
  if(message.HasKey("error")){
    return HC_417_EXPECTATION_FAILED;
  }
  else{
    return HC_200_OK;
  }
}
#endif

#endif
