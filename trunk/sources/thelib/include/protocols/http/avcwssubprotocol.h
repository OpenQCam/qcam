#ifndef _AVCWSSUBPROTOCOL_H
#define _AVCWSSUBPROTOCOL_H
#include "protocols/http/basewssubprotocol.h"

class BaseClientApplication;

class AVCWSSubProtocol
: public BaseWSSubProtocol {
  public:
	  AVCWSSubProtocol();
	  virtual ~AVCWSSubProtocol();

    virtual bool SignalInputBinaryFrame(IOBuffer& data,
                                        WebSocketDataFrame& dataFrame);
    virtual bool SignalInputTextFrame(IOBuffer& data,
                                      WebSocketDataFrame& dataFrame);
};

#endif
