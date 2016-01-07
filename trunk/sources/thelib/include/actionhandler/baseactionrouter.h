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


#ifndef _BASEACTIONROUTER_H
#define _BASEACTIONROUTER_H

#include "common.h"
#include "protocols/cli/statuscode.h"

class BaseActionHandler;
class BaseClientApplication;
class BaseProtocol;

struct RestCLIMessage {
  BaseProtocol *protocol;
  uint32_t cbId;
  Variant request;
  struct _response {
    // Response
    RestHTTPCode statusCode;
    Variant data; // JSON format
  } response;

  RestCLIMessage () {
    Reset();
  };
  virtual ~RestCLIMessage() {};

  void Reset() {
    request.Reset();
    response.statusCode = HC_000_RESERVED;
    response.data.Reset();
  }

  void Print() {
    DEBUG ("=======================================================");
    DEBUG ("request parameters:\n%s", STR(request.ToString()));
    DEBUG ("resp data:%s", STR(response.data.ToString()));
    DEBUG ("resp stautsCode:%d", response.statusCode);
  }
};

class BaseActionRouter {
  private:
    map<string, BaseActionHandler*> _actionHandlers;

  protected:
    BaseActionRouter();
    virtual bool IsReadyToAction(RestCLIMessage& restMessage) = 0;

  public:
    static bool ParseURI(const string &path, URI &uri);
    static bool ParseFirstLine(string &line, URI &uri, RestCLIMessage &msg);
    static string GetHTTPResponse(RestHTTPCode code);
    static void ConvertToJSONMessage(RestCLIMessage &cliMsg, Variant &jsonMsg, string cuid);
    virtual ~BaseActionRouter();
    virtual bool RouteRequestAction(RestCLIMessage &msg);
    virtual void RegisterActionHandler(string actionType, BaseActionHandler *pActionHandler);
    virtual void UnRegisterActionHandler(string actionType);
    virtual BaseActionHandler* GetActionHandler(string actionType);
    virtual bool CreateActionHandlers(BaseClientApplication *pBaseApp) = 0;
  private:
};

#endif /* _BASEACTIONROUTER_H */
