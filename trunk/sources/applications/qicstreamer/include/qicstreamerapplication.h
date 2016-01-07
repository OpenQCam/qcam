/*
// ==================================================================
// This confidential and proprietary software may be used only as
// authorized by a licensing agreement from Quanta Computer Inc.
//
//            (C) COPYRIGHT Quanta Computer Inc.
//                   ALL RIGHTS RESERVED
//
// ==================================================================
// ------------------------------------------------------------------
// Date    :
// Version :
// Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// ------------------------------------------------------------------
// Purpose :
// ==================================================================
*/

#ifndef _QICSTREAMERAPPLICATION_H
#define _QICSTREAMERAPPLICATION_H

#include "application/baseclientapplication.h"

// media stream
class BaseVideoStream;
class BaseAudioStream;
class PCMPlayStream;
class AudioFileStream;
class InMSGStream;
class CVRMuxOutStream;
class AVMuxStream;
class RelayMuxStream;

/*#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
class UXThreadCVR;
class UXThreadInfo;
class UXThreadSystem;
endif*/

namespace app_qicstreamer {
#ifdef HAS_PROTOCOL_RTMP
  class RTMPAppProtocolHandler;
#endif /* HAS_PROTOCOL_RTMP */
#ifdef HAS_PROTOCOL_LIVEFLV
  class LiveFLVAppProtocolHandler;
#endif /* HAS_PROTOCOL_LIVEFLV */
#ifdef HAS_PROTOCOL_TS
  class TSAppProtocolHandler;
#endif /* HAS_PROTOCOL_TS */
#ifdef HAS_PROTOCOL_RTP
  class RTPAppProtocolHandler;
  class RTSPAppProtocolHandler;
#endif /* HAS_PROTOCOL_RTP */
#ifdef HAS_PROTOCOL_MMS
  class MMSAppProtocolHandler;
#endif /* HAS_PROTOCOL_MMS */
#ifdef HAS_PROTOCOL_RAWHTTPSTREAM
  class RawHTTPStreamAppProtocolHandler;
#endif /* HAS_PROTOCOL_RAWHTTPSTREAM */
#ifdef HAS_PROTOCOL_AVHTTPSTREAM
  class AVHTTPStreamAppProtocolHandler;
#endif /* HAS_PROTOCOL_AVHTTPSTREAM */
#ifdef HAS_PROTOCOL_HTTP
  class HTTPAppProtocolHandler;
  class HTTPSAppProtocolHandler;
#endif /* HAS_PROTOCOL_HTTP */
#ifdef HAS_PROTOCOL_CLI
  class RestCLIAppProtocolHandler;
#endif /* HAS_PROTOCOL_HTTP */
  class WSClientAppProtocolHandler;
  class WSServerAppProtocolHandler;
/*#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
  class UDSRecordAppProtocolHandler;
  class UDSCommandAppProtocolHandler;
  class UDSIVAAppProtocolHandler;
#endif*/
  class ActionRouter;
  class QICStreamerApplication
  : public BaseClientApplication,
    public ISystemCommandCallback {

  private:
#ifdef HAS_PROTOCOL_RTMP
    RTMPAppProtocolHandler *_pRTMPHandler;
#endif /* HAS_PROTOCOL_RTMP */
#ifdef HAS_PROTOCOL_LIVEFLV
    LiveFLVAppProtocolHandler *_pLiveFLVHandler;
#endif /* HAS_PROTOCOL_LIVEFLV */
#ifdef HAS_PROTOCOL_TS
    TSAppProtocolHandler *_pTSHandler;
#endif /* HAS_PROTOCOL_TS */
#ifdef HAS_PROTOCOL_RTP
    RTPAppProtocolHandler *_pRTPHandler;
    RTSPAppProtocolHandler *_pRTSPHandler;
#endif /* HAS_PROTOCOL_RTP */
#ifdef HAS_PROTOCOL_MMS
    MMSAppProtocolHandler *_pMMSHandler;
#endif /* HAS_PROTOCOL_MMS */
#ifdef HAS_PROTOCOL_RAWHTTPSTREAM
    RawHTTPStreamAppProtocolHandler *_pRawHTTPStreamHandler;
#endif /* HAS_PROTOCOL_RAWHTTPSTREAM */
#ifdef HAS_PROTOCOL_AVHTTPSTREAM
    AVHTTPStreamAppProtocolHandler *_pAVHTTPStreamHandler;
#endif /* HAS_PROTOCOL_AVHTTPSTREAM */
#ifdef HAS_PROTOCOL_HTTP
    HTTPAppProtocolHandler *_pHTTPHandler;
    HTTPSAppProtocolHandler *_pHTTPSHandler;
    WSClientAppProtocolHandler *_pWSOutHandler;
    WSServerAppProtocolHandler *_pWSInHandler;
#endif /* HAS_PROTOCOL_HTTP */
#ifdef HAS_PROTOCOL_CLI
    RestCLIAppProtocolHandler *_pCLIHandler;
#endif /* HAS_PROTOCOL_CLI */


/*#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
    UDSRecordAppProtocolHandler *_pUDSRecordHandler;
    UDSCommandAppProtocolHandler *_pUDSCommandHandler;
    UDSIVAAppProtocolHandler *_pUDSIVAHandler;
    //UXDomainSocketAppProtocolHandler *_pUXHandler;
    //UXThreadSystem *_pUXThreadSystem;
    //UXThreadCVR *_pUXThreadCVR;
    //UXThreadInfo *_pUXThreadInfo;
#endif*/
    ActionRouter *_pActionRouter;
    //audio
    BaseAudioStream *_pWaveStream;
    PCMPlayStream *_pWavePlayStream;
    //msg stream
    InMSGStream *_pSystemInfoStream;

    //AV mux stream
    AVMuxStream *_pMuxStream;
    RelayMuxStream *_pRelayMuxStream;

    enum LIVE_STREAM_STATUS {
      LSS_IDLE = 0,
      LSS_HANDSHAKE,
      LSS_LIVE
    };
    struct liveAVCStream {
      LIVE_STREAM_STATUS status;
      uint32_t streamUniqueID;
      string streamName;
      uint32_t protocolId;
    } _liveAVCStream;

  private:
    bool InitCloudService();
    void CloseCloudService();

    bool InitProtocolHandlers();
    void DeleteProtocolHandlers();

    bool InitActionHandlers();
    void DeleteActionHandlers();

    bool InitMediaStreams();
    void DeleteMediaStreams();


/*#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
    bool InitUXThreads();
    void DeleteUXThreads();
#endif*/

    bool InitSystemInformation();

  public:
    QICStreamerApplication(Variant &configuration);
    virtual ~QICStreamerApplication();
    virtual bool Initialize();
    virtual bool PushLocalStream(Variant streamConfig);
    // In order to catch live stream, it is necessary to overwrite SignalStreamRegistered
    virtual void SignalStreamRegistered(BaseStream *pStream);
    virtual void SignalStreamUnRegistered(BaseStream *pStream);
    virtual void RegisterProtocol(BaseProtocol *pProtocol);
    virtual void UnRegisterProtocol(BaseProtocol *pProtocol);
    // Notification
    virtual bool OnNotifyEvent(uint16_t type, uint16_t subType,
                               uint64_t eventInfo, string payload="");
    virtual bool OnNotifyData(uint16_t subType, uint64_t info,
                              uint8_t *pMeta, uint32_t metaLength,
                              uint8_t *pData, uint32_t length);
    bool SendCmdResp(string& data, uint64_t utid, uint32_t protId);
    bool SendSync(Variant& data);
    bool SendError(const uint16_t subType, const string& description);
    bool SendEvent(uint16_t subType, uint64_t info, string payload="");
    bool SendInfo(uint16_t subType, uint8_t* pData, uint32_t length);
    virtual bool OnCVRNotify(IOBuffer& event);
    ///Authentication
    virtual bool IsAuthedSuccess(string uriString);
    virtual bool OnServerRegisterAuthed(Variant& authInfo);
    bool IsAVCStreamLive();
    uint32_t GetLiveAVCStreamId();

    ActionRouter* GetActionRouter();
    void InitAllNetworkServices();
    void OnCmdFinished(uint32_t msgId, uint8_t* pData,
                       uint32_t dataLength);
  };
}

#endif  /* _QICSTREAMERAPPLICATION_H */
