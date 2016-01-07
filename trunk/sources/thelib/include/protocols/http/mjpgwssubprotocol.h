#ifndef _MJPGWSSUBPROTOCOL_H
#define _MJPGWSSUBPROTOCOL_H
#include "protocols/http/basewssubprotocol.h"

class BaseClientApplication;
class WebSocketDataFrame;

class MJPGWSSubProtocol
: public BaseWSSubProtocol {
  public:
	  MJPGWSSubProtocol();
	  virtual ~MJPGWSSubProtocol();

    virtual void SignalInputBinaryFrame(IOBuffer& data,
                                        WebSocketDataFrame& dataFrame);
    virtual void SignalInputTextFrame(IOBuffer& data,
                                      WebSocketDataFrame& dataFrame);
    virtual bool Initialize(Variant& config);
};

#endif
