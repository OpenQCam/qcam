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


#ifndef _ACTIONROUTER_H
#define _ACTIONROUTER_H

#include "actionhandler/baseactionrouter.h"

class BaseClientApplication;

namespace app_qicstreamer {

  class ActionRouter
  : public BaseActionRouter
  {

    protected:
      virtual bool IsReadyToAction(RestCLIMessage& restMessage);

    public:
      ActionRouter();
      virtual ~ActionRouter();
      virtual bool CreateActionHandlers(BaseClientApplication *pBaseApp);
      virtual bool CreateProductionActionHandlers(BaseClientApplication *pBaseApp);
  };

}
#endif /* _ACTIONROUTER_H */
