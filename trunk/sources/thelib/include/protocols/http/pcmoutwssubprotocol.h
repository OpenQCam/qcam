#ifndef _PCMOUTWSSUBPROTOCOL_H
#define _PCMOUTWSSUBPROTOCOL_H
#include "protocols/http/basewssubprotocol.h"

class BaseClientApplication;

class PCMOutWSSubProtocol
: public BaseWSSubProtocol {
  public:
	  PCMOutWSSubProtocol();
	  virtual ~PCMOutWSSubProtocol();

    virtual void SignalInputBinaryFrame(IOBuffer& buffer,
                                        WebSocketDataFrame& WebSocketDataFrame);
    virtual void SignalInputTextFrame(IOBuffer& buffer,
                                      WebSocketDataFrame& WebSocketDataFrame);
    virtual bool Initialize(Variant& config);
};

#endif
