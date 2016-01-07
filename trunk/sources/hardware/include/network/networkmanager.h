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


#ifndef _NETWORKMANAGER_H
#define _NETWORKMANAGER_H

#include "common.h"

class NetworkManager {
  public:
    static bool GetInterfaceIP(string netIF, string &ipAddr);
    static bool GetInterfaceMacAddress(string netIF, string &macAddr);
    static bool GetInterface(vector<string> &netIf);
    static bool GetNetworkConfiguration(Variant &config);
};


#endif  /* _NETWORKMANAGER_H */

