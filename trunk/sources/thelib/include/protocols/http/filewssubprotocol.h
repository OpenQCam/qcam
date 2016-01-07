#ifndef _FileWSSUBPROTOCOL_H
#define _FileWSSUBPROTOCOL_H
#include "protocols/http/basewssubprotocol.h"

class BaseClientApplication;

class FileWSSubProtocol
: public BaseWSSubProtocol {
  private:
    string _filename;
    string _directory;
    string _chksum;

    string GetSha1Sum();
  public:
	  FileWSSubProtocol();
	  virtual ~FileWSSubProtocol();

    virtual void SignalInputBinaryFrame(IOBuffer& data,
                                        WebSocketDataFrame& dataFrame);
    virtual void SignalInputTextFrame(IOBuffer& data,
                                      WebSocketDataFrame& dataFrame);
    virtual bool Initialize(Variant& config);
};

#endif
