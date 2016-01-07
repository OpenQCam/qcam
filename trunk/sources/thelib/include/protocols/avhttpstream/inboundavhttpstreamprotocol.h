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


#ifndef _INBOUNDAVHTTPSTREAMPROTOCOL_H
#define _INBOUNDAVHTTPSTREAMPROTOCOL_H

#include "protocols/http/inboundhttpprotocol.h"

class BaseAVHTTPStreamAppProtocolHandler;
class BaseClientApplication;
class BaseOutHTTPStream;
class BaseInStream;

class InboundAVHTTPStreamProtocol
: public InboundHTTPProtocol {
  private:
    Variant _message;
    BaseAVHTTPStreamAppProtocolHandler *_pProtocolHandler;
    IOBuffer _outputBuffer;
    BaseOutHTTPStream *_pOutStream;
    BaseInStream *_pInStream;
    string _filePath;
    bool _isDeleteOutStream;

  protected:
    virtual bool SignalDataProcessed();

  public:
    InboundAVHTTPStreamProtocol();
    virtual ~InboundAVHTTPStreamProtocol();
    virtual void SetApplication(BaseClientApplication *pApplication);
    virtual IOBuffer * GetOutputBuffer();
    virtual IOBuffer* GetProtocolOutputBuffer();
    virtual bool EnqueueForOutbound();

    void RegisterOutStream(BaseOutHTTPStream *pStream);
    void RegisterInStream(BaseInStream *pStream, string filePath);
    void SendHttpResponseHeader (uint32_t statusCode, uint64_t fileSize, uint64_t offset);

    bool IsRegisteredOutStream(string filePath);
    bool IsOutStreamCreated() {return _pOutStream!=NULL;}
    bool RePlayStream(uint32_t offset);
  private:
    bool ParseURL(string url);
};
#endif /* _INBOUNDAVHTTPSTREAMPROTOCOL_H */
