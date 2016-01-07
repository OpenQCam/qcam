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
#include "qicstreamerapplication.h"

#include "protocols/protocoltypes.h"
#include "protocols/rtmp/basertmpprotocol.h"
#include "protocols/http/outboundhttpwsprotocol.h"
#include "protocols/http/httpauthhelper.h"
#include "protocols/avhttpstream/avstreamdefine.h"

//protocolhandler
#include "appprotocolhandler/rtmpappprotocolhandler.h"
#include "appprotocolhandler/liveflvappprotocolhandler.h"
#include "appprotocolhandler/tsappprotocolhandler.h"
#include "appprotocolhandler/rtpappprotocolhandler.h"
#include "appprotocolhandler/rtspappprotocolhandler.h"
#include "appprotocolhandler/mmsappprotocolhandler.h"
#include "appprotocolhandler/rawhttpstreamappprotocolhandler.h"
#include "appprotocolhandler/httpappprotocolhandler.h"
#include "appprotocolhandler/restcliappprotocolhandler.h"
#include "appprotocolhandler/httpsappprotocolhandler.h"
#include "appprotocolhandler/avhttpstreamappprotocolhandler.h"
#include "appprotocolhandler/uxdomainsocketappprotocolhandler.h"

//websocket handler
#include "appprotocolhandler/wsclientappprotocolhandler.h"
#include "appprotocolhandler/wsserverappprotocolhandler.h"
#include "servicetimer.h"
#include "regservicetimer.h"

//actionhandler
#include "clidefine.h"
#include "actionhandler/actionrouter.h"

//hardware
#include "hardwaremanager.h"
#include "gpio/basegpiodevice.h"
//video
#include "video/basev4l2videocapture.h"
#include "video/basevideostream.h"
#include "video/devicevideostream.h"
#include "qic/qicmotioncapture.h"

//Audio Device
#include "audio/baseaudiodevice.h"
#include "audio/speaker.h"

//Audio capture stream
#include "audio/g711stream.h"
#include "audio/pcmstream.h"
#include "audio/pcmplaystream.h"
#include "streaming/audiofilestream.h"
#if HAS_CODEC_AACSTREAM
#include "audio/aacstream.h"
#endif
#if HAS_CODEC_MP3STREAM
#include "audio/mp3stream.h"
#endif

//system message stream
#include "streaming/inmsgstream.h"

#include "netio/netio.h"
#include "system/systemdefine.h"
#include "system/eventdefine.h"
#include "system/nvramdefine.h"
#include "system/systemmanager.h"
#include "application/clientapplicationmanager.h"

#include "system/thread/threadmanager.h"

/*#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
//#include "uxthread/uxthreadcvr.h"
//#include "uxthread/uxthreadsystem.h"
//#include "uxthread/uxthreadinfo.h"
#include "uxthread/udsrecordthread.h"
#include "uxthread/udscommandthread.h"
#include "uxthread/udsivathread.h"
#include "appprotocolhandler/udsrecordappprotocolhandler.h"
#include "appprotocolhandler/udscommandappprotocolhandler.h"
#include "appprotocolhandler/udsivaappprotocolhandler.h"
#endif*/

#include "protocols/cli/restclidefine.h"
#include "protocols/protocolmanager.h"
#include "mux/cvrmuxoutstream.h"
#include "mux/avmuxstream.h"
#include "mux/relaymuxstream.h"

using namespace app_qicstreamer;

QICStreamerApplication::QICStreamerApplication(Variant &configuration)
: BaseClientApplication(configuration) {

#ifdef HAS_PROTOCOL_RTMP
  _pRTMPHandler = NULL;
#endif /* HAS_PROTOCOL_RTMP */
#ifdef HAS_PROTOCOL_LIVEFLV
  _pLiveFLVHandler = NULL;
#endif /* HAS_PROTOCOL_LIVEFLV */
#ifdef HAS_PROTOCOL_TS
  _pTSHandler = NULL;
#endif /* HAS_PROTOCOL_TS */
#ifdef HAS_PROTOCOL_RTP
  _pRTPHandler = NULL;
  _pRTSPHandler = NULL;
#endif /* HAS_PROTOCOL_RTP */
#ifdef HAS_PROTOCOL_MMS
  _pMMSHandler = NULL;
#endif /* HAS_PROTOCOL_MMS */
#ifdef HAS_PROTOCOL_RAWHTTPSTREAM
  _pRawHTTPStreamHandler = NULL;
#endif /* HAS_PROTOCOL_RAWHTTPSTREAM */
#ifdef HAS_PROTOCOL_AVHTTPSTREAM
  _pAVHTTPStreamHandler = NULL;
#endif /* HAS_PROTOCOL_AVHTTPSTREAM */
#ifdef HAS_PROTOCOL_HTTP
  _pHTTPHandler = NULL;
  _pHTTPSHandler = NULL;
  _pWSOutHandler = NULL;
#endif /* HAS_PROTOCOL_HTTP */
#ifdef HAS_PROTOCOL_CLI
  _pCLIHandler = NULL;
#endif /* HAS_PROTOCOL_CLI */
/*#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
  _pUDSRecordHandler = NULL;
  _pUDSCommandHandler = NULL;
  _pUDSIVAHandler = NULL;
  //_pUXHandler = NULL;
  //_pUXThreadSystem = NULL;
  //_pUXThreadCVR = NULL;
#endif *//* HAS_PROTOCOL_UNIXDOMAIN_SOCKET */
  _pActionRouter = NULL;
  _pWaveStream = NULL;
  _pSystemInfoStream = NULL;
  _pMuxStream = NULL;
  _pRelayMuxStream = NULL;
}

QICStreamerApplication::~QICStreamerApplication() {
/*#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
  DeleteUXThreads();
#endif*/
  DeleteProtocolHandlers();
  DeleteActionHandlers();
  DeleteMediaStreams();
  CloseCloudService();
}

// public
bool QICStreamerApplication::Initialize() {

  HTTPAuthHelper::SetAuthKey(SystemManager::GetNVRam("Login"),
                             SystemManager::GetNVRam("Password"));

  if (!BaseClientApplication::Initialize()) {
    FATAL("Unable to initialize application");
    return false;
  }

  INFO ("Init System Information");
  if(!InitSystemInformation()){
    FATAL("Unable to initialize system information");
    return false;
  }

  if(!InitProtocolHandlers()){
    FATAL("Unable to initialize protocol handlers");
    return false;
  }

  INFO ("Init MediaStreams");
  if(!InitMediaStreams()){
    FATAL("Unable to initialize hardware media streams");
    return false;
  }

 INFO ("Init ActionHandler");
  if(!InitActionHandlers()){
    FATAL("Unable to initialize action handlers");
    return false;
  }

  // Raylin temp remove UXThreads to prevent error
/*#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
  INFO ("Init UXThreads");
  if(!InitUXThreads()){
    FATAL("Unable to initialize UXThreadMISC");
    return false;
  }
#endif*/

  //must After Init protocolhandler
// Raylin Disable CloudService to prevent crash. TODO: Fix or remove?
#if 0
  INFO ("Init CloudService");
  if(!InitCloudService()){
    FATAL("Unable to initialize cloud service");
    return false;
  }
#endif

  // get nvram into systemmanager
  // Raylin temp remove to prevent crash
  SystemManager::UpdateDeviceInfo();
  // Raylin temp remove to prevent crash
  SystemManager::SetSystemTime(SystemManager::GetNVRam("NtpTimeZone"));

  // default live avc stream is IDLE
  _liveAVCStream.status = LSS_IDLE;
  _liveAVCStream.streamUniqueID = 0;
  _liveAVCStream.streamName = "";
  _liveAVCStream.protocolId = 0;
  return PullExternalStreams();
}

bool QICStreamerApplication::IsAVCStreamLive() {
  return (_liveAVCStream.status != LSS_IDLE);
}

uint32_t QICStreamerApplication::GetLiveAVCStreamId() {
  return _liveAVCStream.streamUniqueID;
}

bool QICStreamerApplication::PushLocalStream(Variant streamConfig) {
  // check streamName
  if(!streamConfig.HasKey("targetStreamName")){
    INFO("Unable to find targetStreamName");
    return false;
  }
  //3. do handshake for live avc stream
  _liveAVCStream.status = LSS_HANDSHAKE;
  _liveAVCStream.streamName = (string) streamConfig["targetStreamName"];

  return BaseClientApplication::PushLocalStream(streamConfig);
}

//Tim: overwrite SignalStreamRegistered and SignalStreamUnRegistered to handle live avc
void QICStreamerApplication::SignalStreamRegistered(BaseStream *pStream) {
  if(_liveAVCStream.status == LSS_HANDSHAKE){
    // live stream is ready
    if(pStream->GetName() == _liveAVCStream.streamName
        && pStream->GetType() == ST_OUT_NET_RTMP_4_RTMP) {
      _liveAVCStream.status = LSS_LIVE;
      _liveAVCStream.streamUniqueID = pStream->GetUniqueId();
      INFO("live avc stream (%s) is on air", STR(_liveAVCStream.streamName));
    }
  }
  BaseClientApplication::SignalStreamRegistered(pStream);
}

void QICStreamerApplication::SignalStreamUnRegistered(BaseStream *pStream) {
  if(_liveAVCStream.status == LSS_LIVE){
    // live stream is ready
    if(pStream->GetName() == _liveAVCStream.streamName
        && pStream->GetType() == ST_OUT_NET_RTMP_4_RTMP) {
      INFO("live avc stream (%s) is disconnected", STR(_liveAVCStream.streamName));
      _liveAVCStream.status = LSS_IDLE;
      _liveAVCStream.streamUniqueID = 0;
      _liveAVCStream.streamName = "";
    }
  }
  BaseClientApplication::SignalStreamUnRegistered(pStream);
}

//Tim: overwrite RegisterProtocol and Unregisterprotocol to handle incomplete handshake
void QICStreamerApplication::RegisterProtocol(BaseProtocol *pProtocol) {
  if(_liveAVCStream.status == LSS_HANDSHAKE){
    _liveAVCStream.protocolId = pProtocol->GetId();
  }
  BaseClientApplication::RegisterProtocol(pProtocol);
}

void QICStreamerApplication::UnRegisterProtocol(BaseProtocol *pProtocol) {
  if(_liveAVCStream.status == LSS_HANDSHAKE && _liveAVCStream.protocolId == pProtocol->GetId()){
    _liveAVCStream.status = LSS_IDLE;
    _liveAVCStream.protocolId = 0;
  }
  BaseClientApplication::UnRegisterProtocol(pProtocol);
}

ActionRouter* QICStreamerApplication::GetActionRouter() {
  return _pActionRouter;
}

void QICStreamerApplication::InitAllNetworkServices() {
}

//private
bool QICStreamerApplication::InitCloudService() {

    return true;
  if (SystemManager::IsFactoryMode()) {
    return true;
  }

  uint32_t timeout=stringToNum<uint32_t>(SystemManager::GetNVRam(NVRAM_NETWORK_WS_TIMEOUT));
  bool cloudEnabled=stringToNum<bool>(SystemManager::GetNVRam(NVRAM_NETWORK_CLOUD_ENABLED));
  bool isSSL=true;
  string serverAgent=SystemManager::GetNVRam(NVRAM_NETWORK_CLOUD_SERVERAGENT);
  string regAgent=SystemManager::GetNVRam(NVRAM_NETWORK_CLOUD_REGAGENT);
  string defaultServer;

  if (!_pWSOutHandler)
    return false;
  if (!_configuration.HasKey("defaultConfig"))
    return false;

  Variant defaultConfig = _configuration["defaultConfig"];

  if (defaultConfig.HasKey("cloud", false)) {
    Variant cloudConfig = defaultConfig["cloud"];
    FOR_MAP (cloudConfig, string, Variant, i) {
        BaseServiceTimer* pServiceTimer=NULL;
        ws_param_t wsParam=
          {cloudEnabled, false, isSSL, 0, 0, timeout, defaultServer, ""};

        Variant temp = MAP_VAL(i);
        if (temp.HasKey(CONF_CLOUD_CONNECTNAME)) {
          wsParam.name = (string)temp[CONF_CLOUD_CONNECTNAME];
        }

        if (temp.HasKey(CONF_CLOUD_ISREG)) {
          wsParam.isReg= true;
          wsParam.enabled=true;
          if (!SystemManager::GetCUID().empty() && !SystemManager::GetPrivateKey().empty())
            continue;
          pServiceTimer = new RegServiceTimer (this, wsParam, regAgent);
        }
        else {
          pServiceTimer = new ServiceTimer (this, wsParam, serverAgent);
        }

        pServiceTimer->EnqueueForTimeEvent(1);
        _pWSOutHandler->RegisterServiceTimer(wsParam.name, pServiceTimer);
    } //for map
  }//wsoutconfig
  else
    return false;

  return true;
}


bool QICStreamerApplication::InitProtocolHandlers() {

#ifdef HAS_PROTOCOL_RTMP
  _pRTMPHandler = new RTMPAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_INBOUND_RTMP, _pRTMPHandler);
  RegisterAppProtocolHandler(PT_INBOUND_RTMPS_DISC, _pRTMPHandler);
  RegisterAppProtocolHandler(PT_OUTBOUND_RTMP, _pRTMPHandler);
#endif /* HAS_PROTOCOL_RTMP */
#ifdef HAS_PROTOCOL_LIVEFLV
  _pLiveFLVHandler = new LiveFLVAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_INBOUND_LIVE_FLV, _pLiveFLVHandler);
#endif /* HAS_PROTOCOL_LIVEFLV */

#ifdef HAS_PROTOCOL_TS
  _pTSHandler = new TSAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_INBOUND_TS, _pTSHandler);
#endif /* HAS_PROTOCOL_TS */

#ifdef HAS_PROTOCOL_RTP
  _pRTPHandler = new RTPAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_INBOUND_RTP, _pRTPHandler);
  RegisterAppProtocolHandler(PT_RTCP, _pRTPHandler);

  _pRTSPHandler = new RTSPAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_RTSP, _pRTSPHandler);
#endif /* HAS_PROTOCOL_RTP */
#ifdef HAS_PROTOCOL_MMS
  _pMMSHandler = new MMSAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_OUTBOUND_MMS, _pMMSHandler);
#endif /* HAS_PROTOCOL_MMS */
#ifdef HAS_PROTOCOL_RAWHTTPSTREAM
  _pRawHTTPStreamHandler = new RawHTTPStreamAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_INBOUND_RAW_HTTP_STREAM, _pRawHTTPStreamHandler);
#endif /* HAS_PROTOCOL_RAWHTTPSTREAM */
#ifdef HAS_PROTOCOL_AVHTTPSTREAM
  _pAVHTTPStreamHandler = new AVHTTPStreamAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_INBOUND_AV_HTTP_STREAM, _pAVHTTPStreamHandler);
#endif /* HAS_PROTOCOL_AVHTTPSTREAM */

#ifdef HAS_PROTOCOL_HTTP
  _pHTTPHandler = new HTTPAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_INBOUND_HTTP, _pHTTPHandler);

  //RegisterAppProtocolHandler(PT_OUTBOUND_HTTP_FOR_RTMP, _pHTTPHandler);
  _pHTTPSHandler = new HTTPSAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_INBOUND_SSL, _pHTTPSHandler);
  //RegisterAppProtocolHandler(PT_JSON_VAR, _pHTTPSHandler);

#endif /* HAS_PROTOCOL_HTTP */
#ifdef HAS_PROTOCOL_CLI
#ifdef HAS_PROTOCOL_HTTP
  _pCLIHandler = new RestCLIAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_INBOUND_JSONRESTCLI, _pCLIHandler);

#if 1
  _pWSInHandler = new WSServerAppProtocolHandler (_configuration);
  RegisterAppProtocolHandler (PT_INBOUND_HTTP_WSCONNECTION, _pWSInHandler);
  RegisterAppProtocolHandler (PT_WEBSOCKET_SUBPROTOCOL, _pWSInHandler);
  if (!_pWSInHandler->Initialize())
    return false;

  _pWSOutHandler = new WSClientAppProtocolHandler (_configuration);
  RegisterAppProtocolHandler (PT_OUTBOUND_HTTP_WSCONNECTION, _pWSOutHandler);
  if (!_pWSOutHandler->Initialize())
    return false;
#endif

#endif /* HAS_PROTOCOL_HTTP */
#endif /* HAS_PROTOCOL_CLI */

/*#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
  //_pUXHandler = new UXDomainSocketAppProtocolHandler(_configuration);
  //RegisterAppProtocolHandler(PT_UNIX_DOMAIN_SOCKET, _pUXHandler);
  _pUDSRecordHandler = new UDSRecordAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_UDS_RECORD, _pUDSRecordHandler);
  _pUDSCommandHandler = new UDSCommandAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_UDS_COMMAND, _pUDSCommandHandler);
  _pUDSIVAHandler = new UDSIVAAppProtocolHandler(_configuration);
  RegisterAppProtocolHandler(PT_UDS_IVA, _pUDSIVAHandler);
#endif*/

  return true;

}

bool QICStreamerApplication::InitActionHandlers() {
  _pActionRouter = new ActionRouter();
  if (SystemManager::IsFactoryMode()) {
    _pActionRouter->CreateProductionActionHandlers((BaseClientApplication *)this);
  }
  else {
    _pActionRouter->CreateActionHandlers((BaseClientApplication *)this);
  }
  return true;
}

bool QICStreamerApplication::InitSystemInformation() {
  return true;
}

bool QICStreamerApplication::InitMediaStreams() {

  //----------------------------------------------------------------------------
  // Camera Steram
  //----------------------------------------------------------------------------

  // avc stream
  BaseV4L2VideoCapture *pAVCCaptureInstance = (BaseV4L2VideoCapture *)HardwareManager::GetHardwareInstance(HT_VIDEO_AVC);
#if defined(QIC1832)
  QICMotionCapture * pMotionCaptureInstance = (QICMotionCapture *)HardwareManager::GetHardwareInstance(HT_VIDEO_MOTION);
#endif
  if(pAVCCaptureInstance == NULL){
    FATAL("Unable to access avc");
    //return false;
  }
  else {
    //_pAVCStream = pAVCCaptureInstance->CreateVideoStream(GetStreamsManager(), QIC_STREAM_AVC, 0);
    if(pAVCCaptureInstance->CreateVideoStreams(GetStreamsManager(), QIC_STREAM_MUX) == 0){
    //if(_pAVCStream == NULL){
      FATAL("Unable to create AVC stream");
      //return false;
    }
#if defined(QIC1832)
    if(pMotionCaptureInstance->CreateMotionStream(GetStreamsManager(), QIC_STREAM_MOTION) == 0){
    //if(_pAVCStream == NULL){
      FATAL("Unable to create Motion stream");
      //return false;
    }
#endif

    pAVCCaptureInstance->StartCapture();
// Raylin remove to prevent crash
    SystemManager::SetNVRam(NVRAM_FIRMWARE_DSP, pAVCCaptureInstance->GetFirmwareVersion());

    //Create AVC Relay Stream
    if (!pAVCCaptureInstance->CreateRelayStream(GetStreamsManager(), QIC_STREAM_RELAYAVC)) {
      FATAL("Unable to create Relay AVC stream");
      return false;
    }
    //Get Mux Stream
    map<uint32_t, BaseStream *> inStreams;
    inStreams = GetStreamsManager()->FindByTypeByName(ST_IN_AV_MUX, QIC_STREAM_MUX, true, true);
    if (inStreams.size() == 0)
      return false;
    _pMuxStream = (AVMuxStream *) MAP_VAL(inStreams.begin());

    _pRelayMuxStream= new RelayMuxStream(GetStreamsManager(), QIC_STREAM_RELAYMUX);
     pAVCCaptureInstance->RegisterCarrier((IDeviceCarrier *)_pRelayMuxStream);
     pAVCCaptureInstance->RegisterObserver((IDeviceObserver *)_pRelayMuxStream);
  }

  // mjpg stream
  BaseV4L2VideoCapture *pMJPGCaptureInstance = (BaseV4L2VideoCapture *)HardwareManager::GetHardwareInstance(HT_VIDEO_MJPG);
  if(pMJPGCaptureInstance == NULL){
    FATAL("Unable to access mjpg");
  }
  else {
    if(pMJPGCaptureInstance->CreateVideoStreams(GetStreamsManager(), QIC_STREAM_MJPG) == 0) {
      FATAL("Unable to create MJPG stream");
      return false;
    }
    // jennifer : Why avc video stream NEED to know mjpg vidoe status? i.e why RegisterObserver
    // To get notification of stop streaming ?
    //DeviceVideoStream *pDeviceStream = pAVCCaptureInstance->GetCurrentDeviceStream();
    //pMJPGCaptureInstance->RegisterObserver((IDeviceObserver *)pDeviceStream->GetVideoStream());
    pMJPGCaptureInstance->StartCapture();
  }

  //----------------------------------------------------------------------------
  // Audio Stream
  //----------------------------------------------------------------------------

  //Microphone
  BaseAudioDevice *pMICInstance = reinterpret_cast<BaseAudioDevice *>(HardwareManager::GetHardwareInstance(HT_MIC));
  if(pMICInstance == NULL){
    FATAL("Unable to access mic");
  }
  else {
   _pWaveStream = new PCMStream(GetStreamsManager(), QIC_STREAM_WAVE);
    if (_pWaveStream) {
      pMICInstance->RegisterCarrier((IDeviceCarrier *)_pWaveStream);
      pMICInstance->RegisterObserver((IDeviceObserver *)_pWaveStream);
    }
    pMICInstance->Start();
    if (_pMuxStream) {
      pMICInstance->RegisterCarrier((IDeviceCarrier *)_pMuxStream);
      pMICInstance->RegisterObserver((IDeviceObserver *)_pMuxStream);
    }
    if (_pRelayMuxStream) {
      pMICInstance->RegisterCarrier((IDeviceCarrier *)_pRelayMuxStream);
      pMICInstance->RegisterObserver((IDeviceObserver *)_pRelayMuxStream);
    }
  }
  //Speaker
  Speaker *pSpeakerInstance =
      reinterpret_cast<Speaker *>(HardwareManager::GetHardwareInstance(HT_SPEAKER));
  if(pSpeakerInstance == NULL){
    WARN("Unable to access speaker");
  }
  else {
    pSpeakerInstance->CreateStream(GetStreamsManager());
  }
  _pSystemInfoStream = new InMSGStream(GetStreamsManager(), QIC_STREAM_SYSTEMMSG);
  return true;
}

// Raylin remove to prevent compile error
/*#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
bool QICStreamerApplication::InitUXThreads() {
  //_pUXThreadSystem = new UXThreadSystem(format("/tmp/%s", UX_SOCKET_SYSTEM));
  //_pUXThreadSystem->StartThread();

  //_pUXThreadCVR = new UXThreadCVR(format("/tmp/%s", UX_SOCKET_CVR));
  //_pUXThreadCVR->StartThread();

  //_pUXThreadInfo = new UXThreadInfo(format("/tmp/%s", UX_SOCKET_INFO));
  //_pUXThreadInfo->StartThread();
  if(!_configuration.HasKey("threads")) {
    WARN("No thread configuration");
    return true;
  }
  if(_configuration["threads"].HasKey("numOfRecordThreads")){
    ThreadManager::CreateThreadPool(THREAD_T_RECORD);
    uint32_t num = (uint32_t)_configuration["threads"]["numOfRecordThreads"];
    UDSRecordThread *pThread = NULL;
    while(num--) {
      pThread = new UDSRecordThread();
      ThreadManager::AddThreadToPool(pThread);
      pThread->StartThread();
    }
  }
  if(_configuration["threads"].HasKey("numOfCommandThreads")){
    ThreadManager::CreateThreadPool(THREAD_T_COMMAND);
    uint32_t num = (uint32_t)_configuration["threads"]["numOfCommandThreads"];
    UDSCommandThread *pThread = NULL;
    while(num--) {
      pThread = new UDSCommandThread();
      ThreadManager::AddThreadToPool(pThread);
      pThread->StartThread();
    }
  }
  if(_configuration["threads"].HasKey("numOfIVAThreads")){
    ThreadManager::CreateThreadPool(THREAD_T_IVA);
    uint32_t num = (uint32_t)_configuration["threads"]["numOfIVAThreads"];
    UDSIVAThread *pThread = NULL;
    while(num--) {
      pThread = new UDSIVAThread();
      ThreadManager::AddThreadToPool(pThread);
      pThread->StartThread();
    }
  }
  return true;
}
void QICStreamerApplication::DeleteUXThreads() {
 ThreadManager::Cleanup();
  //if (_pUXThreadSystem)
  //  delete _pUXThreadSystem;
  //if (_pUXThreadCVR)
  //  delete _pUXThreadCVR;
}

#endif*/

void QICStreamerApplication::CloseCloudService() {
}


void QICStreamerApplication::DeleteProtocolHandlers() {

#ifdef HAS_PROTOCOL_RTMP
  UnRegisterAppProtocolHandler(PT_INBOUND_RTMP);
  UnRegisterAppProtocolHandler(PT_OUTBOUND_RTMP);
  if (_pRTMPHandler != NULL) {
    delete _pRTMPHandler;
    _pRTMPHandler = NULL;
  }
#endif /* HAS_PROTOCOL_RTMP */
#ifdef HAS_PROTOCOL_LIVEFLV
  UnRegisterAppProtocolHandler(PT_INBOUND_LIVE_FLV);
  if (_pLiveFLVHandler != NULL) {
    delete _pLiveFLVHandler;
    _pLiveFLVHandler = NULL;
  }
#endif /* HAS_PROTOCOL_LIVEFLV */

#ifdef HAS_PROTOCOL_TS
  UnRegisterAppProtocolHandler(PT_INBOUND_TS);
  if (_pTSHandler != NULL) {
    delete _pTSHandler;
    _pTSHandler = NULL;
  }
#endif /* HAS_PROTOCOL_TS */

#ifdef HAS_PROTOCOL_RTP
  UnRegisterAppProtocolHandler(PT_INBOUND_RTP);
  if (_pRTPHandler != NULL) {
    delete _pRTPHandler;
    _pRTPHandler = NULL;
  }

  UnRegisterAppProtocolHandler(PT_RTSP);
  if (_pRTSPHandler != NULL) {
    delete _pRTSPHandler;
    _pRTSPHandler = NULL;
  }
#endif /* HAS_PROTOCOL_RTP */
#ifdef HAS_PROTOCOL_MMS
  UnRegisterAppProtocolHandler(PT_OUTBOUND_MMS);
  if (_pMMSHandler != NULL) {
    delete _pMMSHandler;
    _pMMSHandler = NULL;
  }
#endif /* HAS_PROTOCOL_MMS */
#ifdef HAS_PROTOCOL_RAWHTTPSTREAM
  UnRegisterAppProtocolHandler(PT_INBOUND_RAW_HTTP_STREAM);
  if (_pRawHTTPStreamHandler != NULL) {
    delete _pRawHTTPStreamHandler;
    _pRawHTTPStreamHandler = NULL;
  }
#endif /* HAS_PROTOCOL_RAWHTTPSTREAM */
#ifdef HAS_PROTOCOL_AVHTTPSTREAM
  UnRegisterAppProtocolHandler(PT_INBOUND_AV_HTTP_STREAM);
  if (_pAVHTTPStreamHandler != NULL) {
    delete _pAVHTTPStreamHandler;
    _pAVHTTPStreamHandler = NULL;
  }
#endif /* HAS_PROTOCOL_AVHTTPSTREAM */

#ifdef HAS_PROTOCOL_HTTP
  UnRegisterAppProtocolHandler(PT_INBOUND_HTTP);
  UnRegisterAppProtocolHandler(PT_OUTBOUND_HTTP);
  if (_pHTTPHandler != NULL) {
    delete _pHTTPHandler;
    _pHTTPHandler = NULL;
  }

  UnRegisterAppProtocolHandler(PT_INBOUND_SSL);
  if (_pHTTPSHandler != NULL) {
    delete _pHTTPSHandler;
    _pHTTPSHandler = NULL;
  }


#endif /* HAS_PROTOCOL_HTTP */
#ifdef HAS_PROTOCOL_CLI
  UnRegisterAppProtocolHandler(PT_INBOUND_JSONRESTCLI);
  if (_pCLIHandler != NULL) {
    delete _pCLIHandler;
    _pCLIHandler = NULL;
  }

  UnRegisterAppProtocolHandler(PT_OUTBOUND_HTTP_WSCONNECTION);
  if (_pWSOutHandler!= NULL) {
    delete _pWSOutHandler;
    _pWSOutHandler = NULL;
  }

#endif /* HAS_PROTOCOL_HTTP */

/*#ifdef HAS_PROTOCOL_UNIXDOMAIN_SOCKET
  UnRegisterAppProtocolHandler(PT_UDS_RECORD);
  if (_pUDSRecordHandler != NULL) {
    delete _pUDSRecordHandler;
    _pUDSRecordHandler = NULL;
  }
  UnRegisterAppProtocolHandler(PT_UDS_COMMAND);
  if (_pUDSCommandHandler != NULL) {
    delete _pUDSCommandHandler;
    _pUDSCommandHandler = NULL;
  }
  UnRegisterAppProtocolHandler(PT_UDS_IVA);
  if (_pUDSIVAHandler != NULL) {
    delete _pUDSIVAHandler;
    _pUDSIVAHandler = NULL;
  }



#endif*/
}

void QICStreamerApplication::DeleteActionHandlers() {
  if(_pActionRouter != NULL){
    delete _pActionRouter;
    _pActionRouter = NULL;
  }
}

void QICStreamerApplication::DeleteMediaStreams() {
  if(_pWaveStream)
    delete _pWaveStream;
  if(_pSystemInfoStream)
    delete _pSystemInfoStream;
}

bool QICStreamerApplication::OnCVRNotify(IOBuffer& event) {
  uint32_t length=GETAVAILABLEBYTESCOUNT(event);
  uint8_t *pBuf=GETIBPOINTER(event);
  uint32_t count;

  //DEBUG ("On CVR Notify:\n%s", STR(event.ToString()));
  CVRMuxOutStream* pStream=reinterpret_cast<CVRMuxOutStream*>
      (GetStreamsManager()->FindFirstByTypeByName(ST_OUT_FILE_MP4, QIC_STREAM_CVRFILE));
  if (!pStream) {
    DEBUG ("no cvr mux stream");
    return false;
  }
  for (count=0; (count+2)<=length; count+=2) {
    uint16_t msg= *((uint16_t*)(pBuf+count));
    if (EVT_CVR_ERROR(msg)) {
      DEBUG ("is error");
    }
    else if (msg==EVT_CVR_READY){
      pStream->StartRecord();
    }
  }
  event.Ignore(count);
  return true;
}

bool QICStreamerApplication::SendInfo(uint16_t subType, uint8_t* pData, uint32_t length) {

  if (_pSystemInfoStream) {
    IOBuffer msg;
    uint32_t msgtype=(subType<<16)|ENTOHS(CLOUD_MSG_INFO);
    uint64_t ext64=0;
    msg.ReadFromBuffer((uint8_t*)&msgtype, sizeof(msgtype));
    msg.ReadFromBuffer((uint8_t*)&ext64, sizeof(ext64));
    msg.ReadFromBuffer(pData, length);
    _pSystemInfoStream->FeedMSGData(GETIBPOINTER(msg), GETAVAILABLEBYTESCOUNT(msg), false);
    return true;
  }
  return false;
}

bool QICStreamerApplication::SendCmdResp(string& payload, uint64_t utid, uint32_t pId) {
  IOBuffer msg;
  uint32_t msgtype=ENTOHS(CLOUD_MSG_CMDRESP);

  //DEBUG ("SendResp:\n%s", STR(payload));
  msg.ReadFromBuffer((uint8_t*)&msgtype, sizeof(msgtype));
  msg.ReadFromBuffer((uint8_t*)&utid, sizeof(utid));
  msg.ReadFromString(payload);
  BaseHTTPWSProtocol *pProtocol=
      reinterpret_cast<BaseHTTPWSProtocol*>(ProtocolManager::GetProtocol(pId));

  if (pProtocol) {
    if (!pProtocol->EnqueueForWSOutbound((uint8_t*)GETIBPOINTER(msg),
                                         GETAVAILABLEBYTESCOUNT(msg),
                                         true, WS_OPCODE_BINARY_FRAME)) {
      WARN ("EnqueueForWSOutbound failed");
      return false;
    }
  }
  else
    WARN ("protocol deleted");
  return true;
}

bool QICStreamerApplication::SendSync(Variant& data) {

  if (_pSystemInfoStream) {
    IOBuffer msg;
    uint32_t msgtype=ENTOHS(CLOUD_MSG_SYNC);
    uint8_t reserved[8]={0};

    msg.ReadFromBuffer((uint8_t*)&msgtype, sizeof(msgtype));
    msg.ReadFromBuffer(reserved, sizeof(reserved));
    if (data.HasKey(REST_PAYLOAD_STR)) {
      msg.ReadFromString(data[REST_PAYLOAD_STR]);
    }

    _pSystemInfoStream->FeedMSGData(GETIBPOINTER(msg), GETAVAILABLEBYTESCOUNT(msg), false);
    return true;
  }
  return false;
}

bool QICStreamerApplication::SendError(const uint16_t subType, const string& description) {
  if (_pSystemInfoStream) {
    IOBuffer msg;
    Variant data;
    string binData;
    uint64_t ts=0;
    uint32_t msgtype=(ENTOHS(subType)<<16)|ENTOHS(CLOUD_MSG_ERROR);

    msg.ReadFromBuffer((uint8_t*)&msgtype, sizeof(msgtype));
    msg.ReadFromBuffer((uint8_t*)&ts, sizeof(ts));
    GETTIMESTAMP(ts);
    data["ts"]= ts;
    data["errorCode"]= subType;
    data["description"]= description;
    data.SerializeToJSON(binData);
    msg.ReadFromString(binData);
    return _pSystemInfoStream->FeedMSGData(GETIBPOINTER(msg), GETAVAILABLEBYTESCOUNT(msg), true);
  }
  return false;
}

bool QICStreamerApplication::SendEvent(uint16_t subType,
                                       uint64_t eventInfo,
                                       string payload) {
  if (_pSystemInfoStream) {
    IOBuffer data;
    uint32_t msgtype=(ENTOHS(subType)<<16)|ENTOHS(CLOUD_MSG_EVENT);
    data.ReadFromBuffer((uint8_t*)&msgtype, sizeof(msgtype));
    data.ReadFromBuffer((uint8_t*)&eventInfo, sizeof(eventInfo));
    data.ReadFromBuffer((uint8_t*)STR(payload), payload.length());
    _pSystemInfoStream->FeedMSGData(GETIBPOINTER(data), GETAVAILABLEBYTESCOUNT(data), false);
    return true;
  }
  return false;
}

bool QICStreamerApplication::OnNotifyEvent(uint16_t type, uint16_t eventType,
                                           uint64_t eventInfo, string payload) {
  BaseGPIODevice *pGPIODev=
    reinterpret_cast<BaseGPIODevice*>(HardwareManager::GetHardwareInstance(HT_GPIO));

  switch (type) {
    case CLOUD_MSG_INFO:
      return SendInfo(eventType, (uint8_t*)STR(payload), payload.length());
    case CLOUD_MSG_ERROR:
      return SendError(ERROR_CODE(eventType), ERROR_DESCRIPTION(eventType)+payload);
    case CLOUD_MSG_EVENT:
      if (pGPIODev && ((eventType&0x00ff)==EVT_IN_MJPG_CONNECTED)) {
        pGPIODev->SetLedEvent(LED_EVENT_STATUS_LIVE_STREAMING,
                              (eventType&0x0100)==0);
      }
      return SendEvent(eventType, eventInfo);
    default:
      return false;
  }
  return true;
}

bool QICStreamerApplication::OnNotifyData(uint16_t subType, uint64_t info,
                                          uint8_t *pMeta, uint32_t metaLength,
                                          uint8_t *pData, uint32_t length) {
  if (_pSystemInfoStream) {
    IOBuffer data;
    uint32_t msgtype=(ENTOHS(subType)<<16)|ENTOHS(CLOUD_MSG_DATA);
    data.ReadFromBuffer((uint8_t*)&msgtype, sizeof(msgtype));
    data.ReadFromBuffer((uint8_t*)&info, sizeof(info));
    data.ReadFromBuffer(pMeta, metaLength);
    //TODO: need to padd bytes 4 bytes alignment?
    data.ReadFromBuffer((uint8_t*)&length, sizeof(length));
    data.ReadFromBuffer(pData, length);
    _pSystemInfoStream->FeedMSGData(GETIBPOINTER(data), GETAVAILABLEBYTESCOUNT(data), true);
    return true;
  }
  return false;
}

bool QICStreamerApplication::IsAuthedSuccess(string uriString) {
  URI uri;
  string accessToken;
  string timeStamp;
  string clientId;

    return true;
  //do authentication in facotry mode
  if (SystemManager::IsFactoryMode()) {
    return true;
  }

  //check appselectorapplication too
  URI::FromString(uriString, false, uri);
  Variant params=uri.parameters();
  if (!params.HasKey(URL_CLIENT_ID)) {
    clientId=(string)SystemManager::GetNVRam("Login");
  }
  else {
    clientId=(string)params[URL_CLIENT_ID];
  }
  if (params.HasKey(URL_ACCESS_TOKEN)) {
    accessToken=(string)params[URL_ACCESS_TOKEN];
  }
  if (params.HasKey(URL_TIMESTAMP)) {
    timeStamp=(string)params[URL_TIMESTAMP];
  }

  DEBUG ("clientId:%s, accessToken:%s, timeStamp:%s",
         STR(clientId), STR(accessToken), STR(timeStamp));
  if (HTTPAuthHelper::IsAuthedSuccess(clientId, accessToken, timeStamp)){
    return true;
  }
  return false;
}

bool QICStreamerApplication::OnServerRegisterAuthed(Variant& authData) {
#define CUID "cuid"
#define PRIVATEKEY "privateKey"
#define TIMEZONE  "timeZone"
#define TIMESTAMP "timeStamp"

  if (authData.HasKey(CUID) && authData.HasKey(PRIVATEKEY)
      && authData.HasKey(TIMEZONE)) {

    if (authData[CUID]!=V_STRING || authData[PRIVATEKEY]!=V_STRING ||
        authData[TIMEZONE]!=V_STRING) {
      DEBUG ("Invalid data format");
    }
    string cuid=(string)authData[CUID];
    string privateKey=(string)authData[PRIVATEKEY];
    string timeZone=(string)authData[TIMEZONE];

    if (!SystemManager::SetCUID(cuid)) {
      return false;
    }
    if (!SystemManager::SetPrivateKey(privateKey)) {
      return false;
    }
    if (!SystemManager::SetSystemTime(timeZone)) {
      return false;
    }
    if (authData.HasKey(TIMESTAMP) && authData[TIMESTAMP]==V_INT64) {
      uint64_t ts=(uint64_t)authData[TIMESTAMP];
      SystemManager::SetSystemTime(ts/1000);
      DEBUG ("timeStamp:%llu", ts);
      WSClientAppProtocolHandler* pHandler=reinterpret_cast<WSClientAppProtocolHandler*>
        (GetProtocolHandler(PT_OUTBOUND_HTTP_WSCONNECTION));
      if (pHandler) {
        pHandler->UpdatePongTime((double)ts);
      }
    }

    DEBUG ("cuid:%s", STR(cuid));
    DEBUG ("privatekey:%s", STR(privateKey));
    DEBUG ("timeZone:%s", STR(timeZone));
  }
  else
    return false;


  system("icam_renew_ip");
  return true;
}

#if 0
void QICStreamerApplication::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                           uint32_t dataLength) {
#define CAM_AGENT "camAgent"
#define REG_AGENT "regAgent"

  string result((char*)pData, dataLength);
  Variant data;
  uint32_t start=0;

  _serverAck=true;
  if (Variant::DeserializeFromJSON(result, data, start)) {
    if (data==V_MAP) {
      if (data.HasKey(CAM_AGENT) && data[CAM_AGENT]==V_STRING) {
        _serverUri = (string)data[CAM_AGENT];
        //SystemManager::SetNVRam(NVRAM_NETWORK_CLOUD_SERVER, _serverUri);
      }
      else if (data.HasKey(REG_AGENT) && data[REG_AGENT]==V_STRING) {
        _regServerUri = (string)data[REG_AGENT];
      }
      else {
        WARN ("Unable to get server address.");
      }
    }
    else {
      WARN ("Unable to get server address.");
    }
  }
  else {
    WARN ("Unable to get server address. Invalid json format");
  }
}

void QICStreamerApplication::QueryCloudServer(string addr) {
  if (_serverAck) {
    uint64_t msgId=0;
    string cmd="curl -s -k -m5 " + addr;

    DEBUG ("cmd:%s", STR(cmd));
    msgId=UnixDomainSocketManager::ExecuteSystemCommand(cmd, GetCallbackId());
    _serverAck=(msgId==0);
  }
}

string QICStreamerApplication::GetCloudServer(bool isReg) {
  if (isReg) {
    return _regServerUri;
  }
  return _serverUri;
}
void QICStreamerApplication::UpdateCAgent(string addr) {
  _serverUri=addr;
}
#else

void QICStreamerApplication::OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                           uint32_t dataLength) {
}
#endif
