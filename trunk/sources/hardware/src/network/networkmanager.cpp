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

#include "network/networkmanager.h"

#include <sstream>
#include <fstream>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>


bool NetworkManager::GetInterfaceIP(string netIF, string &ipAddr) {

  struct ifreq ifr;
  int skfd = 0;
  if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    WARN("open socket error");
    return false;
  }

  strncpy(ifr.ifr_name, netIF.c_str(), IF_NAMESIZE);
  if(ioctl(skfd, SIOCGIFADDR, &ifr) < 0){
    close(skfd);
    WARN("socket error");
    return false;
  }

  ipAddr += inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
  close(skfd);

  return true;
}

bool NetworkManager::GetInterfaceMacAddress(string netIF, string &macAddr) {
  ifstream ifs;
  string path = "/sys/class/net/"+netIF+"/address";
  ifs.open(STR(path), ifstream::in);
  if(!ifs.is_open()){
    ifs.close();
    return false;
  }else{
    getline(ifs, macAddr);
  }
  return true;
}

bool NetworkManager::GetInterface(vector<string> &netIFs) {

  //return listFolder("/sys/class/net", netIFs, true, false, false);

  string path = "/sys/class/net";
  DIR *pDir = NULL;
  pDir = opendir(STR(path));
  if (pDir == NULL) {
    int err = errno;
    FATAL("Unable to open folder: %s %d %s", STR(path), err, strerror(err));
    return false;
  }
  struct dirent *pDirent = NULL;
  while ((pDirent = readdir(pDir)) != NULL) {
    string entry = pDirent->d_name;
    if ((entry == ".")
        || (entry == "..")) {
      continue;
    }
    if (entry == "")
      continue;

      ADD_VECTOR_END(netIFs, entry);
    //if (pDirent->d_type == DT_DIR) {
    //  ADD_VECTOR_END(netIFs, entry);
    //}
  }

  closedir(pDir);
  return true;

  //ifstream ifs;
  //string line, netif;

  //ifs.open("/proc/net/dev", ifstream::in);
  //if(ifs.is_open()){
  //  getline(ifs, line); //first line
  //  getline(ifs, line); //second line
  //  while(ifs.good()){
  //    getline(ifs, line);
  //    stringstream ss(line.substr(0, 6));
  //    ss >> netif;
  //    if(netif.length() > 0){
  //      netIF.push_back(netif);
  //      netif.clear();
  //    }
  //  }
  //}
  //ifs.close();
  //return true;
}
bool NetworkManager::GetNetworkConfiguration(Variant &config) {
  return true;
}
