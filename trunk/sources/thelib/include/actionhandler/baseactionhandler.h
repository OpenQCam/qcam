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

#ifndef _BASEACTIONHANDLER_H
#define _BASEACTIONHANDLER_H

#include "actionhandler/actionhandlertypes.h"
#include "protocols/cli/statuscode.h"
#include "protocols/cli/restclidefine.h"
#include "netio/netio.h"
#include "system/errordefine.h"

class BaseClientApplication;
class BaseProtocol;

class BaseActionHandler
: public ISystemCommandCallback {
  private:
    uint64_t _actionHandlerType;
    string _apiVersion;
  protected:
    BaseClientApplication *_pApplication;
    BaseProtocol *_pProtocol;
    BaseActionHandler(uint64_t actionType, string apiversion, BaseClientApplication *pApplication);

  public:
    static RestHTTPCode SendFailure(RestHTTPCode statusCode,
        RestErrorCode errorCode, string errorMessage, Variant &message);
    static RestHTTPCode SendSystemFailure(RestHTTPCode statusCode,
        uint16_t errorCode, string errorMessage, Variant &message);
    static RestHTTPCode SendBlockingCmd(const string& cmd,bool sync,
                                        Variant *scriptParam, Variant &message);
    virtual ~BaseActionHandler();
    virtual bool IsValidAPIVersion(string version);
    virtual string GetAPIVersion();
    virtual uint64_t GetType();


    virtual bool IsValidAction(string action) {return false;}

    /* --------------------------------------------------------------------------*/
    /**
    * @Synopsis - Initialize action map
    *
    * @Returns  - return true if initialization is completed, otherwise false.
    */
    /* ----------------------------------------------------------------------------*/
    virtual bool Initialize() = 0;

    /* --------------------------------------------------------------------------*/
    /**
    * @Synopsis - The fuctnion which every action handler should implement
    *             to perform action
    *
    * @Param pFrom - the protocol requests action
    * @Param resource - resource path contained by vector
    * @Param parameters - the parameters from viewer
    * @Param message - return message from action handler
    *
    * @Returns - return true if action is successfully performed, otherwise false.
    */
    /* ----------------------------------------------------------------------------*/
    virtual RestHTTPCode DoAction(BaseProtocol *pFrom,
                                  bool isReadOnly,
                                  vector<string> &resource,
                                  Variant &payload,
                                  Variant &message) = 0;

    virtual void OnCmdFinished(uint32_t msgId, uint8_t *pData,
                               uint32_t dataLength);

    virtual void SetSystemParams(Variant& message, const string& cmd,
                                 bool sync, Variant* scriptParam);
};

#endif
