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
 *  Purpose : AVC video capture for QIC1832
 * ==================================================================
 */

#ifdef QIC1832

#include "qic/qic32avcvideocapture.h"
#include "qic/qic32avcstream.h"
#include "mux/avmuxstream.h"
//#include "video/basevideostream.h"
#include "video/devicevideostream.h"

/*
static FILE *fp;
static FILE *fp1;
static FILE *fp2;
static FILE *fp3;
 */

#define ENABLE 1
#define DISABLE 0
#define OSD_POSITION_ONE 0
#define OSD_POSITION_TWO 1
#define OSD_POSITION_THREE 2
#define OSD_POSITION_FOUR 3
/* By firmware 2106, the last three character out of 32 are out of line attribute control range
 * Setting line attribute to 32 will make clearing the last three character impossible */
#define OSD_DISPLAY_MAX_LENGTH  20
//#define OSD_DISPLAY_MAX_LENGTH  28 // Not sutible for simulcast
//jennifer
#define OSD_STRING_MAX_LENGTH (OSD_DISPLAY_MAX_LENGTH - DATE_TIME_LENGTH -1)
#define TIME_LENGTH  9
#define DATE_TIME_LENGTH  19
#define MAX_AVAILABE_DATE_FORMAT   11
#define MAX_AVAILABE_POSITION   4
#define SAMPLE_STRING "0123456789012345678901234567"
#define TIMER_PLACE_HOLDER "         "
#define SEI_SWITCH_ADDR 0x6F0000A0

BaseHardwareAbstractDevice* QIC32AVCVideoCapture::_pDeviceInstance = NULL;

StreamLayerId_t QIC32AVCVideoCapture::_deviceStreamIdMap[] = {SIMULCAST_STREAM0,SIMULCAST_STREAM1,SIMULCAST_STREAM2,SIMULCAST_STREAM3};

const char * const QIC32AVCVideoCapture::_kDateFormat[] =
{
    "%Y-%m-%d",
    "%Y/%m/%d",
    "%Y-%b-%d",
    "%Y/%b/%d",
    "%d-%m-%Y",
    "%d/%m/%Y",
    "%d-%b-%Y",
    "%d/%b/%Y",
    "%m-%d-%Y",
    "%m/%d/%Y",
    "%b.%d,%Y"
};

const uint16_t QIC32AVCVideoCapture::_kOSDPositionXCoordinate[] = {0,160,0,160};// simulcast compatible
//jennifer
//const uint16_t QIC32AVCVideoCapture::_kOSDPositionXCoordinate[] = {0,832,0,832};// 28 characters
//const uint16_t QIC32AVCVideoCapture::_kOSDPositionXCoordinate[] = {0,768,0,768};// 32 characters
const uint16_t QIC32AVCVideoCapture::_kOSDPositionYCoordinate[] = {0,0,630,630};
//const uint16_t QIC32AVCVideoCapture::_kOSDPositionYCoordinate[] = {0,0,700,700};
const uint16_t QIC32AVCVideoCapture::_kOSDPositionToLine[] = {LINE0,LINE0,LINE7,LINE7};

BaseHardwareAbstractDevice* QIC32AVCVideoCapture::GetInstance(string devNode) {
  if(_pDeviceInstance == NULL){
    int32_t fd;
    if((fd = open(STR(devNode), O_RDWR | O_NONBLOCK, 0)) < 0) {
      FATAL("Cannot open device %s", STR(devNode));
      return NULL;
    }
    //jennifer(TODO) : _deviceFD is used in standard ioctl,
    //qic_xuctrl keep its own device file descriptor.
    //Consider to refactor qic_xuctrl
    //so that device file descriptor can be under controlled through _deviceFD
    _pDeviceInstance = new QIC32AVCVideoCapture(fd);
    if(!_pDeviceInstance->InitializeDevice()){
      delete _pDeviceInstance;
      return NULL;
    }
  }
  return _pDeviceInstance;
}

QIC32AVCVideoCapture::QIC32AVCVideoCapture(int32_t fd)
  : BaseAVCVideoCapture(HT_VIDEO_AVC, fd)
  ,_defaultWidth("")
  ,_defaultHeight("")
  ,_defaultFrameRate("")
  ,_defaultBitRate("")
  ,_defaultGOP("")
  ,_lfcnt(0)
  ,_fcnt(0)
{
  /*
  fp = fopen("/tmp/qic32.dump", "w");
  fp1 = fopen("/tmp/qic32.dump1", "w");
  fp2 = fopen("/tmp/qic32.dump2", "w");
  fp3 = fopen("/tmp/qic32.dump3", "w");
   */

  // Initialize videocapturefilterscapability and videocapturefilters
  BaseV4L2VideoCapture::InitializeDevice();
  _isSetFormat = false;

  _isALSSupported = false;
  _isIRSupported = false;
  _isOSDSupported = false;
  _isPTSupported = false;
  _isSimulcastSupported = false;

  _pOSDInfo = new OSD();
  _pHeader = new QIC32Header();
  //memset(_subStreamEnableStatus,0,16);
  int j;
  for(j=0;j<16;j++)
    _subStreamEnableStatus[j]=0;
}

QIC32AVCVideoCapture::~QIC32AVCVideoCapture() {
  /*
  fclose(fp);
  fclose(fp1);
  fclose(fp2);
  fclose(fp3);
   */
  // 1. stop capturing
  StopCapture();

  // 2. Un-initialize mmap
  UnInitMmap();

  if(NULL != _pOSDInfo)
    delete _pOSDInfo;

  if(NULL != _pHeader)
    delete _pHeader;
  // 3. close device
  Close();
}

bool QIC32AVCVideoCapture::InitializeDevice() {

  _status = VCS_IDLE;

  // 1. Mappings of XU controls to V4L2 controls
  QicSetDeviceHandle(_deviceFD);

  // 2. query capability
  struct v4l2_capability V4L2Capability;
  if(!v4l2_querycap(V4L2Capability)){
    return false;
  }

  // 3. set resolution
  // The resolution here is just for initialization sake
  // Initialization for simulcast will be done later.
  // i.e after requsted , queued buffer and before capturing
#if (defined(__CL1830__))
  if(!v4l2_s_fmt(REQ_WIDTH, REQ_HEIGHT, V4L2_PIX_FMT_MJPEG)){
    FATAL("Set format failed");
    return false;
  }

  if(!InitMmap()) return false;
#endif
  INFO("Init AVC video capture done. fd = %d", _deviceFD);

  //Query the capabilities of QIC.
  //i.e. XU control for PAN/TILT, IR, ALS, OSD, Simulcast etc
  /*
   */
   unsigned char supportedNumber = 0;
   unsigned int supportedList = 0;
   if(QicGetCapabilityList(&supportedNumber, &supportedList)){
     FATAL("Get QIC capabilities failed");
     return false;
   }
   //FATAL("Supported number : %d, list : %d", supportedNumber, (uint32_t)supportedList);
   _isALSSupported = supportedList & SUPPORT_ALS;
   _isIRSupported = supportedList & SUPPORT_IR;
   _isOSDSupported = supportedList & SUPPORT_OSD;
   _isPTSupported = supportedList & SUPPORT_PT;
   _isSimulcastSupported = supportedList & SUPPORT_SIMULCAST;

   /*
   DEBUG("_isALSSupported %d", _isALSSupported);
   DEBUG("_isIRSupported %d", _isIRSupported);
   DEBUG("_isOSDSupported %d", _isOSDSupported);
   DEBUG("_isSimulcastSupported %d", _isSimulcastSupported);
   DEBUG("_isPTSupported %d", _isPTSupported);
   FATAL("_isPTSupported %d", _isPTSupported);
   if(_isPTSupported){
     InitializePTTagList();
    if(!v4l2_query_ctrl(V4L2_CID_PAN_ABSOLUTE, &_pan))
      return false;
    //INFO("Pan Max : %d", _pan.max);
    //INFO("Pan Min : %d", _pan.min);

    if(!v4l2_query_ctrl(V4L2_CID_TILT_ABSOLUTE, &_tilt))
      return false;
    //INFO("Tilt Max : %d", _tilt.max);
    //INFO("Tilt Min : %d", _tilt.min);

   }

    */
  if(!GetDeviceVideoConstraints()) return false;

  _isSetFormat = true;
  INFO("Init AVC video capture done. fd = %d", _deviceFD);
  return true;
}

bool QIC32AVCVideoCapture::GetDeviceVideoConstraints() {
  Resolution resolution;
  unsigned char count;
  EncoderCapability_t capability;
  VideoConstraint videoConstraint;
  uint32_t key;

  if(QicEncoderGetNumberOfCapabilities(&count)){
    FATAL("Getting number of supported resolution failed");
    return false;
  }
  //INFO("number of supported resolution :   %d  ", count);
  for(int i=0; i<count; i++){
    if(QicEncoderGetStreamCaps(i, &capability)){
      FATAL("Getting stream capability failed");
      return false;
    }
    videoConstraint.maxBitrate = capability.uiMaxBitrate;
    videoConstraint.minBitrate = capability.uiMinBitrate;
    videoConstraint.maxFPS = capability.ucMaxFramerate;
    videoConstraint.minFPS = capability.ucMinFramerate;
    key = ((capability.usWidth << 16) | capability.usHeight);
    _deviceVideoCapabilities[key] = videoConstraint;
    //FATAL("Resolution : %dx%d, Bitrate max : %d, min : %d, Framerate max : %d, min : %d", capability.usWidth, capability.usHeight, capability.uiMaxBitrate, capability.uiMinBitrate, capability.ucMaxFramerate, capability.ucMinFramerate);
  }
  return true;
}

bool QIC32AVCVideoCapture::OnReadEvent() {
  //FATAL("QIC32AVCVideoCapture::OnReadEvent");
  int streamID;
  int temporalID;
  struct v4l2_buffer buf = {0};
  uint64_t PTS = 0;

  CLEAR(*_pHeader);
  // Dequeue buffer
  if(!v4l2_dqbuf(buf)){
    return false;
  }
  // Retrieve stream data and send to corresponding carrier
  if(buf.index < _reqBufferCount && buf.index >=0){

    //IdentifySubStream((uint8_t *)_pV4L2Buffers[buf.index].start, buf.bytesused);
    streamID = (uint16_t) IdentifySubStream((uint8_t *)_pV4L2Buffers[buf.index].start, buf.bytesused);
    temporalID = get_stream_temporal_id((uint8_t*)_pV4L2Buffers[buf.index].start, buf.bytesused);
    // This is for video content analysis
    /*
    if(SIMULCAST_STREAM0 == streamID){
      fwrite(((uint8_t *)_pV4L2Buffers[buf.index].start), 1, buf.bytesused, fp);
    }
    if(SIMULCAST_STREAM1 == streamID){
      fwrite(((uint8_t *)_pV4L2Buffers[buf.index].start), 1, buf.bytesused, fp1);
    }
    if(SIMULCAST_STREAM2 == streamID){
      fwrite(((uint8_t *)_pV4L2Buffers[buf.index].start), 1, buf.bytesused, fp2);
    }
    if(SIMULCAST_STREAM3 == streamID){
      fwrite(((uint8_t *)_pV4L2Buffers[buf.index].start), 1, buf.bytesused, fp3);
    }
     */
    PTS = (uint64_t)((buf.timestamp.tv_sec*1000)+(buf.timestamp.tv_usec/1000));
    SendDataToCarrier(((uint8_t *)_pV4L2Buffers[buf.index].start), buf.bytesused, PTS, _deviceVideoStreams[streamID]->GetCarrierId(), HW_DATA_VIDEO);
#if 0
    // Move this part to BaseInStream
    // Split SPS and PPS from I frame
    if(!ParseFrame((uint8_t *)_pV4L2Buffers[buf.index].start, buf.bytesused, _pHeader)){
      //WARN("Invalid Frame");
    }else{
      if(_pHeader != NULL){
        _fcnt++;

        _pHeader->PTS = (unsigned long)((buf.timestamp.tv_sec*1000)+(buf.timestamp.tv_usec/1000));

        if(_pHeader->IsKeyFrame){
          // Send SPS and PPS first
          SendDataToCarrier(((uint8_t *)_pV4L2Buffers[buf.index].start), _pHeader->FrameOffset, _pHeader->PTS, _deviceVideoStreams[streamID]->GetCarrierId(), HW_DATA_VIDEO);
          // Send I frame
          SendDataToCarrier(((uint8_t *)_pV4L2Buffers[buf.index].start)+_pHeader->FrameOffset, _pHeader->FrameLength, _pHeader->PTS,  _deviceVideoStreams[streamID]->GetCarrierId(), HW_DATA_VIDEO);
        //FATAL("naluType@qic32avcvideocapture : %d",((uint8_t *)_pV4L2Buffers[buf.index].start)[4]);

        }
        else{
          // Send P frame
          SendDataToCarrier(((uint8_t *)_pV4L2Buffers[buf.index].start), _pHeader->FrameLength, _pHeader->PTS,  _deviceVideoStreams[streamID]->GetCarrierId(), HW_DATA_VIDEO);
          /*
          // Send P frame
          SendDataToCarrier(((uint8_t *)_pV4L2Buffers[buf.index].start), _pHeader->FrameLength, _pHeader->PTS,  _deviceVideoStreams[streamID]->GetCarrierId());
           */
        }
        //FATAL(" QIC32AVCVideoCapture::OnReadEvent,   _deviceVideoStreams[streamID]->GetCarrierId() : %d",  _deviceVideoStreams[streamID]->GetCarrierId());
      } // Header not null
      //FATAL("Sent data to stream %d", streamID);
    }
#endif

    if(!v4l2_qbuf(buf)){
      FATAL("Unable to queue buffer");
      return false;
    }
  }else{
    FATAL("Out of buffer index, %d, max. %d", buf.index, _reqBufferCount);
    return false;
  }

  return true;
}

uint8_t QIC32AVCVideoCapture::CreateVideoStreams(StreamsManager *pSM, string streamName) {
  uint16_t numOfStreams = AVAIL_SUB_STREAMS;
  uint16_t i = 0;

  while(i < numOfStreams){
    // 1. create video stream for application
    // video stream is created here but deleting in BaseV4L2VideoCapture because video stream is relevant to hardware device
    // The stream name assigned to avc stream is the key for streaming video through the internet. e.g rtsp
     /*
     BaseVideoStream *pStream = (BaseVideoStream*) new QIC32AVCStream(pSM, format("%s%d", STR(streamName), i));
      **/
    BaseVideoStream *pStream;
    if(i == 0){
      AVMuxStream * _avMuxStream = new AVMuxStream(pSM, streamName);
      //_avMuxStream->EnableHLS();
      pStream = _avMuxStream;

      //pStream = (BaseVideoStream*) new QIC32AVCStream(pSM, streamName);
    } else{
      pStream = (BaseVideoStream*) new AVMuxStream(pSM, format("%s%d", STR(streamName), i));
      //pStream = (BaseVideoStream*) new QIC32AVCStream(pSM, format("%s%d", STR(streamName), i));
    }

    // 2. register video stream as carrier for receiving data
    RegisterCarrier((IDeviceCarrier *)pStream);
    // 3. register video stream as observer for obtaining notification
    RegisterObserver((IDeviceObserver *)pStream);
    // 4. create device stream as video source
    DeviceVideoStream *pDeviceStream = new DeviceVideoStream(pStream);
    // 5. set device video constraints
    // CAUTION : since device video constraints is a static member,
    // the content should be assigned ONLY ONCE
    // (i.e Need NOT to assign it again in MJPGCaptureDevice)
    if(i==0){
      pDeviceStream->GetCapability()->SetDeviceVideoConstraint(&_deviceVideoCapabilities);
    }
    // 6. Initialize video capability
    if(!pDeviceStream->GetCapability()->InitVideoCapability(i,true)) return false;
    // 7. bind video stream and device stream together
    _deviceVideoStreams.insert(pair<uint16_t, DeviceVideoStream*>((uint16_t)_deviceStreamIdMap[i], pDeviceStream));

    i++;
  }
  // 7. last device stream as default current stream
  // jennifer : stream name of the first stream has no trailing digit currently
  //if(BaseV4L2VideoCapture::SelectDeviceStream(streamName + "0")){
  if(BaseV4L2VideoCapture::SelectDeviceStream(streamName)){
    // return number of streams
    return i;
  }else{
    return 0;
  }
}

bool QIC32AVCVideoCapture::InitSimulcast() {
  // Set codec as simulcast
  if(QicEuExSetSelectCodec(CODEC_H264_SIMULCAST)){
  // This is for ONE streamer layer only
  //if(QicEuExSetSelectCodec(CODEC_H264)){
    FATAL("Selecting codec for simulcast FAIL");
    return false;
  }
  // Enable SEI
  if(QicMmioWrite(SEI_SWITCH_ADDR, 1)){
    //FATAL("Enabling SEI FAIL");
    return false;
  }
  CheckIfSEIEnabled();
  // Customize video configuration according to NVRAM
  if(!CustomizeVideoSettings()){
    FATAL("Customizing video setting failed");
    return false;
  }

  return true;
}

bool QIC32AVCVideoCapture::CustomizeVideoSettings(){
  string avc("Device_Video_AVC");
  string width("_Width");
  string height("_Height");
  string temporal_layer("_Temporal_Layer");
  string bitrate("_Bitrate");
  string fps("_FPS");
  string gop("_GOP");
  string enable_stream("_Enable");
  string tempString;
  uint32_t i = 0;
  uint32_t substreamCount=0;
  uint32_t widthNo;
  VideoConstraint avcConstraint;
  Resolution resolutionConstraint[AVAIL_RESOLUTION_SET];
  MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->GetAVCVideoConstraints(&avcConstraint);
  MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->GetResolutionConstraints(resolutionConstraint);

  //FATAL("Current device video stream bitrate, max : %d, min : %d", avcConstraint.maxBitrate, avcConstraint.minBitrate);
  //FATAL("Current device video stream fps, max : %d, min : %d", avcConstraint.maxFPS, avcConstraint.minFPS);
  //FATAL("Current device video stream gop, max : %d, min : %d", avcConstraint.maxGOP, avcConstraint.minGOP);
  //FATAL("Current device video stream height : %d", resolutionConstraint[0].height);
  //FATAL("Current device video stream width : %d", resolutionConstraint[0].width);
  // For each device video stream, get the default configuration from NVRAM
  //while(i<1){
  while(i<AVAIL_SUB_STREAMS){
    // Select device stream for controlling
    if(!SelectDeviceStream(_deviceVideoStreams[(uint16_t)_deviceStreamIdMap[i]]->GetVideoStreamName())){
      FATAL("Setting sub stream id to %d FAIL", _deviceStreamIdMap[i]);
      return false;
    }
    //Templayer layer
    tempString.clear();
    if(i==0){
      tempString = avc + temporal_layer;
    }else{
      tempString = avc + numToString<uint32_t>(i,0) + temporal_layer;
    }
    string temp = SystemManager::GetNVRam(tempString);
    if((temp != "") && isNumeric(temp)){
      //INFO("stream %d temporal_layer is %s\n", i, temp.c_str());
      int ret=0;
      int temp_num = stringToNum<uint32_t>(temp);
      if(temp_num!=0) {
        int j;
        for(j=0;j<temp_num;j++){
          //INFO("set substream according to temporal_layer setting");
          _subStreamEnableStatus[i*4+j]=1;
          substreamCount++;
        }
        ret = QicEuExSetTsvc(temp_num);
        if(ret)
          INFO("set temporal_layer failed");
      }
    }

    // AVC Width
    tempString.clear();
    if(i==0){
      tempString = avc + width;
    }else{
      tempString = avc + numToString<uint32_t>(i,0) + width;
    }
    temp = SystemManager::GetNVRam(tempString);
    if((temp != "") && isNumeric(temp)){
      // Temporarily keep stream width for resolution setting
       widthNo = stringToNum<uint32_t>(temp);
    }
    else{
      if(!SetResolution(resolutionConstraint[0].width, resolutionConstraint[0].height))  return false;
      INFO("Setting about AVC resolution in NVRAM is empty or invalid! Set to default resolution.");
    }


    // AVC Height
    tempString.clear();
    if(i==0){
      tempString = avc + height;
    }else{
      tempString = avc + numToString<uint32_t>(i,0) + height;
    }
    temp = SystemManager::GetNVRam(tempString);
    if((temp != "") && isNumeric(temp)){
      if(!SetResolution(widthNo, stringToNum<uint32_t>(temp)))  return false;
    }

    // AVC Bitrate
    tempString.clear();
    if(i==0){
      tempString = avc + bitrate;
    }else{
      tempString = avc + numToString<uint32_t>(i,0) + bitrate;
    }
    temp = SystemManager::GetNVRam(tempString);
    if((temp != "") && isNumeric(temp)){
      if(!SetBitRate(stringToNum<uint32_t>(temp)))  return false;
    }
    else{
      if(!SetBitRate(avcConstraint.maxBitrate))  return false;
      INFO("Setting about bitrate in NVRAM is empty or invalid! Set to default bitrate.");
    }


    // AVC Framerate
    tempString.clear();
    if(i==0){
      tempString = avc + fps;
    }else{
      tempString = avc + numToString<uint32_t>(i,0) + fps;
    }
    temp = SystemManager::GetNVRam(tempString);
    if((temp != "") && isNumeric(temp)){
      if(!SetFrameRate(stringToNum<uint32_t>(temp)))  return false;
    }
    else{
      if(!SetFrameRate(avcConstraint.maxFPS))  return false;
      INFO("Setting about frame rate in NVRAM is empty or invalid! Set to default frame rate.");
    }

    // AVC GOP
    tempString.clear();
    if(i==0){
      tempString = avc + gop;
    }else{
      tempString = avc + numToString<uint32_t>(i,0) + gop;
    }
    temp = SystemManager::GetNVRam(tempString);
    if((temp != "") && isNumeric(temp)){
      if(!SetGOP(stringToNum<uint32_t>(temp)))  return false;
    }
    else{
      if(!SetGOP(avcConstraint.minGOP))  return false;
      INFO("Setting about GOP in NVRAM is empty or invalid! Set to default GOP.");
    }

    //Enable stream or not
    tempString.clear();
    if(i==0){
      tempString = avc + enable_stream;
    }else{
      tempString = avc + numToString<uint32_t>(i,0) + enable_stream;
    }
    temp = SystemManager::GetNVRam(tempString);
    if((temp != "") && isNumeric(temp)){
      INFO("AVC %d Enable value is %s\n", i, temp.c_str());
      uint32_t Enable_num = stringToNum<uint32_t>(temp);
      if(Enable_num==1 && substreamCount<=4){
        if(QicEuSetStartOrStopLayer(LAYER_START)){
          FATAL("Start sub stream failed");
        }else{
          _subStreamEnableStatus[i*4]=1;
        }
      }else{
        if(QicEuSetStartOrStopLayer(LAYER_STOP)){
          FATAL("Start sub stream failed");
        }else{
          _subStreamEnableStatus[i*4]=0;
        }
      }
    }else{
      FATAL("no stream enable parametet\n");
      if(QicEuSetStartOrStopLayer(LAYER_STOP)){
        FATAL("Start sub stream failed");
      }else{
        _subStreamEnableStatus[i*4]=0;
      }
    }
    // Below is for debug sake
    //_deviceVideoStreams[(uint16_t)_deviceStreamIdMap[i]]->PrintCurrentConfig();
    i++;
  }//while

  return true;
}

bool QIC32AVCVideoCapture::SelectDeviceStream(string streamName) {
  if(!BaseV4L2VideoCapture::SelectDeviceStream(streamName)){
    return false;
  }
  //FATAL("CURRENT device stream id : %d",MAP_KEY(_pCurrentDeviceStream.begin()));
  if(QicEuSetSelectLayer((StreamLayerId_t)MAP_KEY(_pCurrentDeviceStream.begin()))){
    FATAL("Setting simulcast layer FAIL.");
    return false;
  }
  //FATAL("QicEuSetSelectLayer");

  if("avcstream" == streamName){
    SelectDefaultSettingFields("");
  }
  else if("avcstream1" == streamName){
    SelectDefaultSettingFields("1");
  }
  else if("avcstream2" == streamName){
    SelectDefaultSettingFields("2");
  }
  else if("avcstream3" == streamName){
    SelectDefaultSettingFields("3");
  }
  return true;
}

void QIC32AVCVideoCapture::SelectDefaultSettingFields(string streamNumber) {
  string avc("Device_Video_AVC");
  string width("_Width");
  string height("_Height");
  string bitrate("_Bitrate");
  string fps("_FPS");
  string gop("_GOP");

  _defaultWidth.clear();
  _defaultWidth.append(avc);
  _defaultWidth.append(streamNumber);
  _defaultWidth.append(width);

  _defaultHeight.clear();
  _defaultHeight.append(avc);
  _defaultHeight.append(streamNumber);
  _defaultHeight.append(height);

  _defaultFrameRate.clear();
  _defaultFrameRate.append(avc);
  _defaultFrameRate.append(streamNumber);
  _defaultFrameRate.append(fps);

  _defaultBitRate.clear();
  _defaultBitRate.append(avc);
  _defaultBitRate.append(streamNumber);
  _defaultBitRate.append(bitrate);

  _defaultGOP.clear();
  _defaultGOP.append(avc);
  _defaultGOP.append(streamNumber);
  _defaultGOP.append(gop);
}

StreamLayerId_t QIC32AVCVideoCapture::IdentifySubStream(uint8_t *pData, uint32_t pDataLength){
  uint8_t pps_id;
  uint32_t i;
  /* search for slice header */
  for(i=0; i < (pDataLength-9); i++){
    if(pData[i+0] == 0x00 &&
    pData[i+1] == 0x00 &&
    pData[i+2] == 0x00 &&
    pData[i+3] == 0x01 &&
    ((pData[i+4]&0x1F) == 0x05 || (pData[i+4]&0x1F) == 0x01))
    {
      /* decode pps id from slice header */
      pps_id = DecodePPSId(&pData[i+5]);
      break;
    }
  }
  return _deviceStreamIdMap[pps_id];
}

uint32_t QIC32AVCVideoCapture::DecodePPSId(uint8_t *pData){
  unsigned int buffer;
  int leading_zero;

  buffer = (pData[0]<<24) | (pData[1]<<16) | (pData[2]<<8) | (pData[3]<<0);
  /* skip first_mb_in_slice */
  leading_zero = 0;
  while(!(buffer & 0x80000000)){
    leading_zero++;
    buffer <<=1;
  }
  buffer <<= (1+leading_zero);
  /* skip slice_type */
  leading_zero = 0;
  while(!(buffer & 0x80000000)){
    leading_zero++;
    buffer <<=1;
  }
  buffer <<= (1+leading_zero);
  /* extract pps_id */
  leading_zero = 0;
  while(!(buffer & 0x80000000)){
    leading_zero++;
    buffer <<=1;
  }
  buffer <<= 1;
  if(leading_zero == 0)
    buffer = 0;
  else
    buffer >>= (32-leading_zero);

  return (1<<leading_zero) + buffer - 1;
}

#if 0
/*This whole block are used to parse AVC frame,
 */
bool QIC32AVCVideoCapture::ParseFrame(uint8_t *pData, uint32_t dataLength, QIC32Header* &header) {
  uint8_t naluType = 0;
  uint32_t nextStartCodeOffset = 0;

  if( (nextStartCodeOffset =
        FindStartCode(pData, dataLength, naluType)) != 0 ){
    FATAL("First start code. pData: %p, dataLength : %d, naluType : %d",pData,dataLength,naluType);
    switch(naluType){
      case 0x6:// SEI
        return ParsePFrame(pData, dataLength, header, nextStartCodeOffset);
      case 0x5:// Key frame
      case 0x1:// P frame
        //WARN("p frame");
        // Replace start code w/ nalu length, where 4 is the length of start code
        *((uint32_t *)pData) = EHTONL(dataLength-4);
        // Record current frame length
        header->FrameLength = dataLength;
        //FATAL("P frame length : %d", dataLength-4);
        _frameCount++;
        break;
      case 0x7:// SPS
        return ParseKeyFrame(pData, dataLength, header, nextStartCodeOffset);
      default:
        WARN("Undefined nalu type %d", naluType);
        return false;
        break;
    } // switch
  } else
    return false;

  return true;
}

uint32_t QIC32AVCVideoCapture::FindStartCode(uint8_t *pData, uint32_t dataLength, uint8_t &type) {
  uint8_t state=0;
  uint32_t offset = 0;

  static const uint8_t fsm[4][6] = {
    {1, 2, 3, 3, 0, 0},
    {0, 0, 0, 4, 0, 0},
    {0, 0, 0, 0, 5, 0},
    {0, 0, 0, 0, 0, 0}
  };

  if(dataLength < 6) return 0;

  while(offset < dataLength){
    switch(pData[offset++]){
    case 0:
      state = fsm[0][state];
      break;
    case 1:
      state = fsm[1][state];
      break;
    case 9:
      state = fsm[2][state];
      break;
    default:
      state = fsm[3][state];
      break;
    }

    switch(state){
      case 4: // found start code
        type  = pData[offset] & 0x1F;
        return offset;
        break;
      default:
        continue;
        break;
    }
  }

  return 0;
}

bool QIC32AVCVideoCapture::ParsePFrame(uint8_t *pData, uint32_t dataLength, QIC32Header* &header, uint32_t nextStartCodeOffset) {
  uint8_t naluType = 0;
  uint8_t *marker;
  uint32_t processedByte=0;
  //FATAL("About to parse p frame with SEI");
  //FATAL("nextStartCodeOffset for SPS: %d", nextStartCodeOffset);
  // Shift to SEI start code
  marker = pData + nextStartCodeOffset;
  processedByte += nextStartCodeOffset;
  //INFO("processedByte  : %d", processedByte);
  // Find next start code for key frame
  if( (nextStartCodeOffset =
        FindStartCode(marker, dataLength-processedByte, naluType)) == 0){
        //FindStartCode(marker, dataLength-nextStartCodeOffset-4, naluType)) != 0){
    FATAL("No start code found for P frame");
    return false;
  }
  //FATAL("P frame start code. marker: %p, nextStartCodeOffset of P frame : %d, ramain data length : %d, naluType : %d", marker, nextStartCodeOffset, dataLength-processedByte, naluType);
  if(1 != naluType){
    FATAL("Invalid Frame. P frame is expected");
    return false;
  }

  // Shift to P frame start code
  marker += nextStartCodeOffset;
  processedByte += nextStartCodeOffset;
  // Backward 5 byte for RTP header usage and procceded 4 byte start code
  header->FrameOffset = processedByte - 9;
  // TODO : remove it. This is temporarily set to make video streaming
  //header->FrameOffset = processedByte - 4;
  // Replace start code w/ nalu length, where 4 is the length of start code
  *((uint32_t *)marker-4) = EHTONL(dataLength-processedByte);
  // Record current frame length
  header->FrameLength = dataLength - processedByte + 4;
  //FATAL("P frame length : %d", dataLength-4);
  _frameCount++;
  return true;
}
/*
 */

bool QIC32AVCVideoCapture::ParseKeyFrame(uint8_t *pData, uint32_t dataLength, QIC32Header* &header, uint32_t nextStartCodeOffset) {
  uint8_t naluType = 0;
  uint8_t *marker;
  uint32_t processedByte=0;

  //FATAL("nextStartCodeOffset for SPS: %d", nextStartCodeOffset);
  // Shift to SPS start code
  marker = pData + nextStartCodeOffset;
  processedByte += nextStartCodeOffset;
  // Find next start code for PPS
  if( (nextStartCodeOffset =
        FindStartCode(marker, dataLength-processedByte, naluType)) == 0){
    FATAL("No start code found for PPS");
    return false;
  }
  //FATAL("Second start code. marker: %p, nextStartCodeOffset of PPS : %d, ramain data length : %d, naluType : %d", marker, nextStartCodeOffset, dataLength-processedByte, naluType);
  if(8 != naluType){
    FATAL("Invalid Frame. PPS is expected");
    return false;
  }

  // Fill SPS length
  *(marker-1) = nextStartCodeOffset - 4;
  header->SPSLength = *(marker-1);
  //header->SPSLength = nextStartCodeOffset - 4;

  // Shift to PPS start code
  marker += nextStartCodeOffset;
  processedByte += nextStartCodeOffset;
  // Find next start code for SEI
  if( (nextStartCodeOffset =
        FindStartCode(marker, dataLength-processedByte, naluType)) == 0){
    FATAL("No start code found for SEI");
    return false;
  }
  //FATAL("Third start code. marker: %p, nextStartCodeOffset of SEI : %d, ramain data length : %d, naluType : %d", marker, nextStartCodeOffset, dataLength-processedByte, naluType);
  if(6 != naluType){
    //FATAL("Invalid Frame. SEI is expected");
    return false;
  }
  /*TODO: remove below. It is for debug sake
  if(5 != naluType){
    FATAL("Invalid Frame. Key frame is expected");
    return false;
  }
   */

  // Fill PPS length
  *(marker-1) = nextStartCodeOffset - 4;
  header->PPSLength = *(marker-1);
  //header->PPSLength = nextStartCodeOffset - 4;

  // Shift to SEI start code
  marker += nextStartCodeOffset;
  processedByte += nextStartCodeOffset;
  //INFO("processedByte  : %d", processedByte);
  // Find next start code for key frame
  if( (nextStartCodeOffset =
        FindStartCode(marker, dataLength-processedByte, naluType)) == 0){
        //FindStartCode(marker, dataLength-nextStartCodeOffset-4, naluType)) != 0){
    FATAL("No start code found for key frame");
    return false;
  }
  //FATAL("Fourth start code. marker: %p, nextStartCodeOffset of keyframe : %d, ramain data length : %d, naluType : %d", marker, nextStartCodeOffset, dataLength-processedByte, naluType);
  if(5 != naluType){
    FATAL("Invalid Frame. Key frame is expected");
    return false;
  }
  /*
   */
  // SEI is NOT required in next processing step.
  // ONLY its memmory space will be reused. Need NOT to process SEI

  // Shift to key frame start code
  marker += nextStartCodeOffset;
  processedByte += nextStartCodeOffset;
  header->IsKeyFrame = true;
  // Backward 5 byte for RTP header usage and procceded 4 byte start code,
  // and this case checking naul type of I frame will NOT get the right type
  header->FrameOffset = processedByte - 9;
  // TODO : remove it. This is temporarily set to make video streaming
  //header->FrameOffset = processedByte - 4;
  //header->FrameOffset = (marker - pData) - 4;
  //INFO("processedByte - 9 : %d, (marker - pData) - 4 : %d ", processedByte - 9, (marker - pData) - 4 );
  // Fill key frame length, removing 4 bytes of start code
  *((uint32_t *)(marker-4)) = EHTONL(dataLength - processedByte);
  //*((uint32_t *)(marker-4)) = EHTONL(dataLength - header->FrameOffset - 4);
  // NEED to pass start code(which is replaced by key frame length now) and data to instream
  header->FrameLength = dataLength - processedByte + 4;
  //header->FrameLength = dataLength - header->FrameOffset;
  //FATAL("I frame length : %d",header->FrameLength);
  //FATAL("Key frame! Frame count so far : %d", _frameCount);
  _frameCount++;
  return true;
}
#endif

bool QIC32AVCVideoCapture::StartCapture() {
  INFO("Start video capture");
  if(_status == VCS_ON){
    INFO("video was already started");
    return true;
  }

  if(_deviceFD < 0){
    FATAL("Device is not open yet");
    return false;
  }

  _fcnt=_lfcnt=0;
#if ((defined(__x86__)) || (defined(__MT7620__)))
  if(!v4l2_s_fmt(REQ_WIDTH, REQ_HEIGHT, V4L2_PIX_FMT_MJPEG)){
    FATAL("Unable to set format");
    return false;
  }

  if(!InitMmap()) return false;

#elif (defined(__CL1830__))
  if(_isSetFormat){
    _isSetFormat = false;
    INFO("Already allocated for embedded system. Doing nothing here.");
  }
  else{
    if(!v4l2_s_fmt(REQ_WIDTH, REQ_HEIGHT, V4L2_PIX_FMT_MJPEG)){
      FATAL("Unable to set format");
      return false;
    }
  }
#endif

  //re-queue buffer
  struct v4l2_buffer buf;
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  for(uint32_t i=0; i<_reqBufferCount; ++i){
    buf.index = i;
    if(!v4l2_qbuf(buf)){
      FATAL("Unable to queue buffers");
      return false;
    }
  }

  // This cannot be done before finishing queue buffers
  if(!InitSimulcast()) return false;


  //jennifer(TODO) : Since OSD timer is ON in default
  //, turn it OFF until the user asked to do so
  if(!v4l2_streamon()){
    FATAL("Unable to turn on stream");
    return false;
  }else{
    ENABLE_DEVICE_READ_DATA;
    SetStatus(VCS_ON);
  }

  INFO("Start video capture end");
  return true;
}

bool QIC32AVCVideoCapture::StopCapture() {
  INFO("Stop video capture start");
  if(_status != VCS_ON){
    INFO("video was already stopped");
    return true;
  }
  if(!v4l2_streamoff()){
    FATAL("Unable to turn off stream");
    return false;
  }
  DISABLE_DEVICE_READ_DATA;

#if !(defined(__CL1830__))
  if(!UnInitMmap()) return false;
#endif

  SetStatus(VCS_OFF);

  INFO("Stop video capture end");
  return true;
}

bool QIC32AVCVideoCapture::VerifyResolution(uint16_t width, uint16_t height){
  return MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->VerifyResolution(width, height);
}

bool QIC32AVCVideoCapture::VerifyBitrate(uint32_t bitrate){
  return MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->VerifyBitrate(bitrate);
}

bool QIC32AVCVideoCapture::VerifyFPS(uint32_t fps){
  return MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->VerifyFPS(fps);
}

bool QIC32AVCVideoCapture::VerifyGOP(uint32_t gop){
  return MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->VerifyGOP(gop);
}

bool QIC32AVCVideoCapture::SetResolution(uint32_t width, uint32_t height) {
  DeviceVideoStream *pDeviceStream = MAP_VAL(_pCurrentDeviceStream.begin());
  VideoConfig *pVideoConfig = pDeviceStream->GetConfig();

  if(pVideoConfig->width == width && pVideoConfig->height == height ){
    INFO("Resolution %dx%d is the same", width, height);
    return true;
  }

  if(!pDeviceStream->GetCapability()->VerifyResolution(width, height)){
    return false;
  }

  if(QicEuSetVideoResolution(width,height)){
    FATAL("Setting simulcast resolution FAIL.");
      return false;
    }

  pVideoConfig->width = width;
  pVideoConfig->height = height;

  NotifyObservers(VCM_RESOLUTION_CHANGED, format("resolution changeed (%dx%d)", width, height));
  return true;
}

bool QIC32AVCVideoCapture::SetDefaultResolution(string width, string height) {
  if(!MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->VerifyResolution((int32_t)atoi(STR(width)), (int32_t)atoi(STR(height))))
    return false;

  if(!SystemManager::SetNVRam(_defaultWidth, width, false))
    return false;

  if(!SystemManager::SetNVRam(_defaultHeight, height, false))
    return false;

  return true;
}

Variant QIC32AVCVideoCapture::GetDefaultResolution() {
  Variant resolution;
  resolution["width"] = stringToNum<uint32_t>(SystemManager::GetNVRam(_defaultWidth));
  resolution["height"] = stringToNum<uint32_t>(SystemManager::GetNVRam(_defaultHeight));
  return resolution;
}

bool QIC32AVCVideoCapture::SetFrameRate(uint32_t fps) {
  DeviceVideoStream *pDeviceStream = MAP_VAL(_pCurrentDeviceStream.begin());
  VideoConfig *pVideoConfig = pDeviceStream->GetConfig();

  if(pVideoConfig->fps == fps){
    INFO("Frame rate  %d is the same", fps);
    return true;
  }

  if(!pDeviceStream->GetCapability()->VerifyFPS(fps)){
    return false;
  }

  if(QicEuSetMinimumFrameInterval(_fpsToFrameInterval[fps])){
    FATAL("Setting frameinterval FIL.");
    return false;
  }

  pVideoConfig->fps = fps;

  NotifyObservers(VCM_FPS_CHANGED, format("Frame rate changed to %d", fps));
  return true;
}

uint32_t QIC32AVCVideoCapture::GetFrameRate() {
  return MAP_VAL(_pCurrentDeviceStream.begin())->GetConfig()->fps;
}

bool QIC32AVCVideoCapture::SetDefaultFrameRate(string fps) {
  if(!MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->VerifyFPS((int32_t)atoi(STR(fps))))
    return false;

  if(!SystemManager::SetNVRam(_defaultFrameRate, fps, false))
    return false;

  return true;
}

string QIC32AVCVideoCapture::GetDefaultFrameRate() {
  return SystemManager::GetNVRam(_defaultFrameRate);
}

bool QIC32AVCVideoCapture::SetBitRate(uint32_t bitrate) {
  DeviceVideoStream *pDeviceStream = MAP_VAL(_pCurrentDeviceStream.begin());
  VideoConfig *pVideoConfig = pDeviceStream->GetConfig();

  if(pVideoConfig->bitrate == bitrate){
    INFO("Bitrate  %d is the same", bitrate);
    return true;
  }

  if(!pDeviceStream->GetCapability()->VerifyBitrate(bitrate)){
    return false;
  }

  if(QicEuSetAverageBitrateControl(bitrate)){
    FATAL("Setting simulcast bitrate FAIL.");
    return false;
  }

  pVideoConfig->bitrate = bitrate;

  return true;
}

uint32_t QIC32AVCVideoCapture::GetBitRate() {
  return MAP_VAL(_pCurrentDeviceStream.begin())->GetConfig()->bitrate;
}

bool QIC32AVCVideoCapture::SetDefaultBitRate(string bitrate) {
  if(!MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->VerifyBitrate((int32_t)atoi(STR(bitrate))))
    return false;

  if(!SystemManager::SetNVRam(_defaultBitRate, bitrate, false))
    return false;

  return true;
}

string QIC32AVCVideoCapture::GetDefaultBitRate() {
  return SystemManager::GetNVRam(_defaultBitRate);
}

bool QIC32AVCVideoCapture::SetGOP(uint32_t gop) {
  DeviceVideoStream *pDeviceStream = MAP_VAL(_pCurrentDeviceStream.begin());
  VideoConfig *pVideoConfig = pDeviceStream->GetConfig();

  if(pVideoConfig->gop == gop){
    INFO("GOP  %d is the same", gop);
    return true;
  }

  if(!pDeviceStream->GetCapability()->VerifyGOP(gop)){
    return false;
  }

  uint32_t currentFPS = MAP_VAL(_pCurrentDeviceStream.begin())->GetConfig()->fps;
  /*
  FATAL("GOP interval :%d",currentFPS);
  FATAL("GOP interval :%d",_fpsToFrameIntervalForGOP[currentFPS]);
  FATAL("GOP interval :%d",gop);
  FATAL("GOP interval :%d",gop * _fpsToFrameIntervalForGOP[currentFPS]);
   */
  if(QicEuSetSynchronizationAndLongTermReferenceFrame(1, gop * _fpsToFrameIntervalForGOP[currentFPS], 0)){
    FATAL("Setting GOP FAIL.");
    return false;
  }

  pVideoConfig->gop = gop;

  return true;
}

uint32_t QIC32AVCVideoCapture::GetGOP() {
  return MAP_VAL(_pCurrentDeviceStream.begin())->GetConfig()->gop;
}

bool QIC32AVCVideoCapture::SetDefaultGOP(string gop) {
  if(!MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->VerifyGOP((int32_t)atoi(STR(gop))))
    return false;

  if(!SystemManager::SetNVRam(_defaultGOP, gop, false))
    return false;

  return true;
}

string QIC32AVCVideoCapture::GetDefaultGOP() {
  return SystemManager::GetNVRam(_defaultGOP);
}

Variant QIC32AVCVideoCapture::GetConstraints(){
  return MAP_VAL(_pCurrentDeviceStream.begin())->GetCapability()->GetConstraints();
}

bool QIC32AVCVideoCapture::GetKeyFrame() {
  //INFO("About to gen key frame");
  uint8_t i;

  //for(i=0; i<1; i++){
  //jennifer(FIXME) : Requesting key frame for all sub streams
  // will cause setting OSD line attribute fail.
  // Wait firmware to resolve this
  for(i=0; i<AVAIL_SUB_STREAMS; i++){
    if(QicEuSetSelectLayer(_deviceStreamIdMap[i])){
      FATAL("Setting simulcast layer FAIL.");
      return false;
    }

  uint32_t currentFPS = MAP_VAL(_pCurrentDeviceStream.begin())->GetConfig()->fps;
  uint32_t gop = MAP_VAL(_pCurrentDeviceStream.begin())->GetConfig()->gop;
  //FATAL("Before generating GOP : %d",MAP_VAL(_pCurrentDeviceStream.begin())->GetConfig()->gop);
    if(QicEuSetSynchronizationAndLongTermReferenceFrame(1, gop * _fpsToFrameIntervalForGOP[currentFPS], 0)){
      FATAL("Getting keyframe FAIL.");
      return false;
    }
  }//for
  return true;
}

string QIC32AVCVideoCapture::GetFirmwareVersion() {
  char data[QIC_FIRMWARE_DATA_SIZE];
  char firmwareVersion[5];

  int ret = QicFlashRead (QIC_FIRMWARE_ADDRESS, (unsigned char*)data, QIC_FIRMWARE_DATA_SIZE, QIC1822_FLASH_MAX_SIZE);
  if(ret)
    FATAL("Get Firmware Version Error, (%d)%s", errno, strerror(errno));

  if (!strncmp(&data[QIC_REVISION_CHARACTER_POSITION], "REV", 3))
    strncpy((char *)&(firmwareVersion), &data[QIC_REVISION_NUMBER_POSITION], 5);
  else
    strncpy((char *)&(firmwareVersion), "FFFF\0", 5);

  string s(firmwareVersion);

  return s;
}

uint32_t QIC32AVCVideoCapture::GetNumOfProfiles() {
  /*
   * jennifer(TODO) : implement this
  int32_t num;
  QicChangeFD(_deviceFD);
   if(QicEncoderGetNumberOfProfiles(&num) < 0){
    FATAL("Get num of profiles failed");
    return -1;
  }
  return num;
   */
  return 0;
}

bool QIC32AVCVideoCapture::GetProfile(int idx, int *level, int *profile, int *constraint_flags) {
  /*
   * jennifer(TODO) : implement this
  QicChangeFD(_deviceFD);
  if(QicEncoderGetProfile(idx, level, profile, constraint_flags) < 0){
    FATAL("Get profile failed");
    return false;
  }
   */
  return true;
}

bool QIC32AVCVideoCapture::SetProfileLevel(int level, int profile, int constraint_flags) {
  /*
   * jennifer(TODO) : implement this
  bool restart = false;

  if(IsStarted()){
    if(!StopCapture()) return false;
    restart = true;
  }

  QicChangeFD(_deviceFD);
  if( QicEncoderSetProfileAndLevel(level, profile, constraint_flags) < 0){
    FATAL("Set profile level failed");
    return false;
  }
  if(restart)
    if(StartCapture()) return false;

  NotifyObservers(VCM_PROFILE_CHANGED, "set profile level done");
   */
  return true;
}

bool QIC32AVCVideoCapture::GetProfileLevel(int *level, int *profile, int *constraint_flags) {
  /*
   * jennifer(TODO) : implement this
  QicChangeFD(_deviceFD);
  if( QicEncoderGetProfileAndLevel(level, profile, constraint_flags) < 0){
    FATAL("Get profile level failed");
    return false;
  }
   */
  return true;
}

// HZ1 doesn't support
bool QIC32AVCVideoCapture::SetPanTilt(int16_t pan, int16_t tilt) {
  /*
   * jennifer(TODO) : implement this
  int16_t gotPan;
  int16_t gotTilt;
  if(10 <pan||-10> pan)
  {
    FATAL("Pan out of range!!");
    return -1;
  }

  if(10 <tilt||-10> tilt)
  {
    FATAL("Tilt out of range!!");
    return -1;
  }
  if(pan<0)
  {
    pan=(~pan)+0x8001;
  }
  if(tilt<0)
  {
    tilt=(~tilt)+0x8001;
  }
  QicChangeFD(_deviceFD);
  if(1 == QicSetPanTilt(pan,tilt)){
    FATAL("Setting Pan and Tilt failed!");
    return -1;
  }
  QicGetPanTilt(&gotPan, &gotTilt);
  _pVideoConfig->pan = gotPan;
  _pVideoConfig->tilt = gotTilt;
  //FATAL("In QICHWCamera Pan : %hd\tTilt : %hd", gotPan, gotTilt);
   */
  return 0;
}

void QIC32AVCVideoCapture::GetPanTilt(int16_t &pan, int16_t &tilt) {
  /*
   * jennifer(TODO) : implement this
  pan = _pVideoConfig->pan;
  tilt = _pVideoConfig->tilt;
   */
}

// Enable/disable OSD function before stream on
bool QIC32AVCVideoCapture::IsOSDSupported(){
  unsigned char OSDStatus;
  if(!QicOsdStatusGet(&OSDStatus)){
    FATAL("Getting OSD status failed");
    return false;
  }
  //jennifer : remove below if stable
  //FATAL("OSD Status : %d", OSDStatus);
  return (OSD_ENABLE == OSDStatus);
  //return (OSD_ENABLE == OSDStatus)? 1 : 0;
}

bool QIC32AVCVideoCapture::SetOSDStatus(unsigned char status){
  unsigned char OSDStatus;

  if(!QicOsdStatusGet(&OSDStatus)){
    FATAL("Getting OSD status failed");
    return false;
  }
  //jennifer : remove below if stable
  //FATAL("Before set, OSD Status : %d", OSDStatus);

  if(QicOsdStatusSet(OSD_ENABLE))
    FATAL("Setting OSD status failed");

  //jennifer : remove below if stable
  //if(!QicOsdStatusGet(&OSDStatus)){
  //  FATAL("Getting OSD status failed");
  //  return false;
  //}
  //FATAL("After set, OSD Status : %d", OSDStatus);
  return true;
}

void QIC32AVCVideoCapture::ResetOSDInfo(){
  UpdateOSDInformation(0,0,string(""),-1,-1,-1);
}

Variant QIC32AVCVideoCapture::GetOSDInformation(){
  Variant OSDInfo;
  Variant camera;
  Variant time;

  camera["enabled"] = _pOSDInfo->isNameEnabled;
  if(_pOSDInfo->isNameEnabled){
    camera["name"] = _pOSDInfo->name;
    camera["position"] = _pOSDInfo->namePosition;
  }

  time["enabled"] = _pOSDInfo->isTimeEnabled;
  if(_pOSDInfo->isTimeEnabled){
    time["format"] = _pOSDInfo->dateFormatIndex;
    time["position"] = _pOSDInfo->datePosition;
  }

  OSDInfo["camera"] = camera;
  OSDInfo["time"] = time;

  return OSDInfo;
}

void QIC32AVCVideoCapture::UpdateOSDInformation(bool isNameEnabled, bool isTimeEnabled
                          , string name, uint8_t namePosition
                          , uint8_t dateFormatIndex, uint8_t datePosition){
  _pOSDInfo->isNameEnabled = isNameEnabled;
  _pOSDInfo->isTimeEnabled = isTimeEnabled;
  _pOSDInfo->name = name;
  _pOSDInfo->namePosition = namePosition;
  _pOSDInfo->dateFormatIndex = dateFormatIndex;
  _pOSDInfo->datePosition = datePosition;
  INFO("Name is enabled ? %d\tName value : %s\tName position : %d\t",_pOSDInfo->isNameEnabled, STR(_pOSDInfo->name), _pOSDInfo->namePosition);
  INFO("Date is enabled ? %d\tDate format : %d\tDate position : %d\t",_pOSDInfo->isTimeEnabled, _pOSDInfo->dateFormatIndex, _pOSDInfo->datePosition);
}

uint8_t QIC32AVCVideoCapture::GetAvailablePosition(uint8_t position){
  return (position + 2) % MAX_AVAILABE_POSITION;
}

bool QIC32AVCVideoCapture::IsValidOSDPosition(int8_t position){
  return ((position >= -1) && (position < MAX_AVAILABE_POSITION));
  //jennifer : remove below if stable
  //return (position < MAX_AVAILABE_POSITION)? 1 : 0;
}

bool QIC32AVCVideoCapture::IsPositionCompatible(uint8_t namePosition,uint8_t datePosition){
  uint8_t sum = namePosition + datePosition;
  if(1 == sum) return false;
  if(5 == sum) return false;
  return true;
}

bool QIC32AVCVideoCapture::IsValidOSDDateFormat(int8_t dateFormatIndex){
  return ((dateFormatIndex >= -1) && (dateFormatIndex < MAX_AVAILABE_DATE_FORMAT));
  //jennifer : remove below if stable
  //return (position < MAX_AVAILABE_POSITION)? 1 : 0;
}

bool QIC32AVCVideoCapture::IsValidOSDName(uint8_t nameLength){
  return (nameLength <= OSD_CHAR_MAX);
  //jennifer : remove below if stable
  //return (nameLength <= OSD_CHAR_MAX)? 1 : 0;
}

bool QIC32AVCVideoCapture::SetOSDString(string content, unsigned char startIndex
                                        , int position, OsdCharAttr_t *charAttribute){
  unsigned char str[28] = {};

  // Get content to display on screen
  strcpy(reinterpret_cast<char*>(str), STR(content));

  // Set string
  if(QicOsdLineStringSet(_kOSDPositionToLine[position], startIndex, str, (*charAttribute))){
    FATAL("Setting OSD line attribute failed");
    return false;
  }

  //jennifer : remove below if stable
  //QicOsdLineStringGet(_kOSDPositionToLine[position], startIndex, str, charAttribute);
  //FATAL("After set : %s",str);
  //FATAL("\nupdated : %d\nfg_color : %d\nbg_color_on :%d\ncolor_mode : %d", charAttribute->updated, charAttribute->fg_color, charAttribute->bg_color_on, charAttribute->color_mode);
  return true;
}

bool QIC32AVCVideoCapture::SetOSDTime(unsigned char lineNumber, unsigned char enable, int position){
  OsdTimer_t timer;
  string hour, minute, second;

  // Get current time
  hour = timeToDate("%H");
  minute = timeToDate("%M");
  second = timeToDate("%S");

  // Specify time position
  timer.timer_pos = position;
  timer.initial_hour = stringToNum<uint16_t>(hour);
  timer.initial_min = stringToNum<uint16_t>(minute);
  timer.initial_sec = stringToNum<uint16_t>(second);

  if(QicOsdTimerSet(lineNumber,enable,timer))
    FATAL("Setting timer failed");

  //jennifer : remove below if stable
  //FATAL("SetOSDTime to %d at position : %d Done. Value : %s:%s:%s", enable, position, STR(hour), STR(minute), STR(second));
  return true;
}

bool QIC32AVCVideoCapture::DeterminePosition(bool isNameEnabled, bool isTimeEnabled
                                  , int8_t &namePosition, int8_t &datePosition
                                  , uint8_t nameLength){
  if(isNameEnabled){
    if(isTimeEnabled){
      if(-1 == datePosition){
        if(-1 == namePosition){
          if(nameLength < OSD_STRING_MAX_LENGTH){
            // Position for both time and name are NOT provided
            // Just assign to the first available position
            namePosition = OSD_POSITION_ONE;
            datePosition = OSD_POSITION_ONE;
          }
          else{
            // There is no enough space to hold both time and name
            // Just stamp them in different position
            datePosition = OSD_POSITION_ONE;
            namePosition = GetAvailablePosition(datePosition);
          }
        }// name position is NOT provided
        else{
          if(nameLength < OSD_STRING_MAX_LENGTH){
            // Time position is NOT provided
            // , and the specific position can hold all the string
            // Stamp time at the same position as name
            datePosition = namePosition;
          }
          else{
            // Time position is NOT provided
            // , and the specific position can NOT hold all the string
            // Get another available position for time
            datePosition = GetAvailablePosition(namePosition);
          }
        }// name position is provided
      }// time position is NOT provided
      else{
        if(-1 == namePosition){
          if(nameLength < OSD_STRING_MAX_LENGTH){
            // Name position is NOT provided
            // , and the specific position can hold all the string
            // Stamp name at the same position as time
            namePosition = datePosition;
          }
          else{
            // Name position is NOT provided
            // , and the specific position can NOT hold all the string
            // Get another available position for name
            namePosition = GetAvailablePosition(datePosition);
          }
        }// name position is NOT provided
        else{
          if(!IsPositionCompatible(namePosition, datePosition)){
            FATAL("Name and time can not be at the opposite site of the same line");
            return false;
          }
          // retain the specific position
        }// name position is provided
      }// time position is provided
    }// enable time
    else{
      if(-1 == namePosition){
        namePosition = OSD_POSITION_ONE;
      }
    }// disable time
  }// enable name
  else{
    if(isTimeEnabled){
      if(-1 == datePosition){
        datePosition = OSD_POSITION_ONE;
      }
    }// enable time
  }// disable name

  return true;
}

bool QIC32AVCVideoCapture::SetOSDContent(uint8_t position, unsigned char startIndex, string content){
  unsigned char str[28] = {};
  OsdLineAttr_t  *lineAttribute = new OsdLineAttr_t();
  OsdCharAttr_t  *charAttribute = new OsdCharAttr_t();

  // 1. Set line attribute
  lineAttribute->char_count = OSD_DISPLAY_MAX_LENGTH;
  lineAttribute->x_start = _kOSDPositionXCoordinate[position];
  lineAttribute->y_start =  _kOSDPositionYCoordinate[position];
  lineAttribute->enabled = 1;
  lineAttribute->char_x_repeat = 0;
  lineAttribute->char_y_repeat = 0;
  lineAttribute->spacing = 0;

  if(QicOsdLineAttrSet(_kOSDPositionToLine[position], (*lineAttribute))){
    FATAL("Setting line attribute failed");
    //jennifer : remove below if stable
    //FATAL("OSD content %s to positionã€€%d at start index %d", STR(content), position, startIndex);
    return false;
  }

  // 2. Set line string
  // Set Color attribute
  charAttribute->updated = 1;
  charAttribute->bg_color_on = 1;
  charAttribute->color_mode = HALF;
  charAttribute->fg_color = BLUE;
  strcpy(reinterpret_cast<char*>(str), STR(content));
  FATAL("Content to be set is : %s", str);
  // Set string
  if(QicOsdLineStringSet(_kOSDPositionToLine[position], startIndex, str, (*charAttribute))){
    FATAL("Setting OSD line attribute failed");
    return false;
  }

  delete lineAttribute;
  delete charAttribute;
  //jennifer : remove below if stable
  //FATAL("Setting OSD content %s to positionã€€%d at start index %d done", STR(content), position, startIndex);
  return true;
}

bool QIC32AVCVideoCapture::ClearAllOSD(){
  OsdCharAttr_t  *charAttribute = new OsdCharAttr_t();
  unsigned char startIndex = 0;
  // Disable timer
  if(!SetOSDTime(LINE0, DISABLE, OSD_DISPLAY_MAX_LENGTH - 1)){
    FATAL("Disable timeer for line 0 failed");
    return false;
  }
  if(!SetOSDTime(LINE7, DISABLE, OSD_DISPLAY_MAX_LENGTH - 1)){
    FATAL("Disable timeer for line 7 failed");
    return false;
  }

  // Set Color attribute
  charAttribute->updated = 1;
  charAttribute->bg_color_on = 0;
  charAttribute->color_mode = NORMAL;
  charAttribute->fg_color = BLUE;

  // Clear background
  if(!SetOSDString(SAMPLE_STRING, startIndex, OSD_POSITION_ONE, charAttribute)){
    FATAL("Setting string for line 0 failed");
    return false;
  }
  if(!SetOSDString(SAMPLE_STRING, startIndex, OSD_POSITION_THREE, charAttribute)){
    FATAL("Setting string for line 7 failed");
    return false;
  }
  //jennifer : remove below if stable
  //FATAL("\nupdated : %d\nfg_color : %d\nbg_color_on :%d\ncolor_mode : %d", charAttribute->updated, charAttribute->fg_color, charAttribute->bg_color_on, charAttribute->color_mode);
  // Clear string
  if(QicOsdLineClear(LINE0))    return false;
  if(QicOsdLineClear(LINE7))    return false;

  delete charAttribute;
  return true;
}

bool QIC32AVCVideoCapture::SetOSD(bool isNameEnabled, bool isTimeEnabled
                                  , string name, int8_t namePosition
                                  , int8_t dateFormatIndex, int8_t datePosition){
  unsigned char startIndex;
  unsigned char timePosition;
  uint8_t nameLength, dateLength;
  string date;

  //FATAL("Name length befor setting to default : %d", name.length());
  if(-1 == dateFormatIndex)
    dateFormatIndex = 1;
  if(0 == name.length())
    name = "iCamera";

  nameLength = name.length();

  date = timeToDate(_kDateFormat[dateFormatIndex]);
  dateLength = date.length();
  //jennifer : remove below if stable
  //FATAL("Name : %s", STR(name));
  //FATAL("date format : %d", dateFormatIndex);
  if(!ClearAllOSD()){
    FATAL("Clear OSD failed");
    return false;
  }

  DeterminePosition(isNameEnabled, isTimeEnabled, namePosition, datePosition, nameLength);
  if(isNameEnabled){
    if(isTimeEnabled){
      if(namePosition == datePosition){

        if(nameLength > OSD_STRING_MAX_LENGTH){
          name = name.substr(0,OSD_STRING_MAX_LENGTH);
          nameLength = name.length();
          FATAL("Trancated name : %s, and nameLength : %d",STR(name), nameLength);
        }
        // 1. Set OSD content
        if((OSD_POSITION_ONE == namePosition) || (OSD_POSITION_THREE == namePosition)){
          startIndex = 0;
          timePosition = nameLength + dateLength + TIME_LENGTH;
          //timePosition = nameLength + DATE_TIME_LENGTH + TIME_LENGTH;
          //timePosition = nameLength + DATE_TIME_LENGTH + 1;
        }
        if((OSD_POSITION_TWO == namePosition) || (OSD_POSITION_FOUR == namePosition)){
          startIndex = OSD_DISPLAY_MAX_LENGTH - (nameLength + 1 + dateLength + TIME_LENGTH);
          timePosition = OSD_DISPLAY_MAX_LENGTH - 1;
        }
        //jennifer : remove below if stable
        //FATAL("startIndex : %d, timePosition : %d", startIndex, timePosition);
        if(!SetOSDContent(namePosition, startIndex, name + " " + date + TIMER_PLACE_HOLDER)){
          FATAL("Setting OSD content failed");
          return false;
        }
        // 2. Set OSD timer
        /*
        if(!SetOSDTime(_kOSDPositionToLine[namePosition], 1, timePosition)){
          FATAL("Setting OSD timer failed");
          return false;
        }
         */
        UpdateOSDInformation(isNameEnabled, isTimeEnabled, name, namePosition, dateFormatIndex, datePosition);
      }// name and date are at the same position
      else{
        // 1.1. Set OSD content for name
        if((OSD_POSITION_ONE == namePosition) || (OSD_POSITION_THREE == namePosition))
          startIndex = 0;
        if((OSD_POSITION_TWO == namePosition) || (OSD_POSITION_FOUR == namePosition))
          startIndex = OSD_DISPLAY_MAX_LENGTH - nameLength;
        if(!SetOSDContent(namePosition, startIndex, name)){
          FATAL("Setting OSD content failed");
          return false;
        }
        // 1.2. Set OSD content for date
        if((OSD_POSITION_ONE == namePosition) || (OSD_POSITION_THREE == namePosition)){
          startIndex = 0;
          timePosition = dateLength + TIME_LENGTH - 1;
        }
        if((OSD_POSITION_TWO == namePosition) || (OSD_POSITION_FOUR == namePosition)){
          startIndex = OSD_DISPLAY_MAX_LENGTH - dateLength - TIME_LENGTH;
          timePosition = OSD_DISPLAY_MAX_LENGTH - 1;
        }
        if(!SetOSDContent(datePosition, startIndex, date + TIMER_PLACE_HOLDER)){
          FATAL("Setting OSD content failed");
          return false;
        }
        // 2. Set OSD timer
        if(!SetOSDTime(_kOSDPositionToLine[datePosition], 1, timePosition)){
          FATAL("Setting OSD timer failed");
          return false;
        }
        UpdateOSDInformation(isNameEnabled, isTimeEnabled, name, namePosition, dateFormatIndex, datePosition);
      }// name and date are NOT at the same position
    }// enable time
    else{
      // 1. Set OSD content for name
      if((OSD_POSITION_ONE == namePosition) || (OSD_POSITION_THREE == namePosition))
        startIndex = 0;
      if((OSD_POSITION_TWO == namePosition) || (OSD_POSITION_FOUR == namePosition))
        startIndex = OSD_DISPLAY_MAX_LENGTH - nameLength;

      if(!SetOSDContent(namePosition, startIndex, name)){
        FATAL("Setting OSD content failed");
        return false;
      }
      UpdateOSDInformation(isNameEnabled, isTimeEnabled, name, namePosition, dateFormatIndex, datePosition);
    }// disable time
  }// enable name
  else{
    if(isTimeEnabled){
      // 1. Set OSD content for date
      if((OSD_POSITION_ONE == datePosition) || (OSD_POSITION_THREE == datePosition)){
        startIndex = 0;
        timePosition = dateLength + TIME_LENGTH - 1;
      }
      if((OSD_POSITION_TWO == datePosition) || (OSD_POSITION_FOUR == datePosition)){
        startIndex = OSD_DISPLAY_MAX_LENGTH - dateLength - TIME_LENGTH;
        timePosition = OSD_DISPLAY_MAX_LENGTH - 1;
      }
      if(!SetOSDContent(datePosition, startIndex, date + TIMER_PLACE_HOLDER)){
        FATAL("Setting OSD content failed");
        return false;
      }
      // 2. Set OSD timer
      if(!SetOSDTime(_kOSDPositionToLine[datePosition], 1, timePosition)){
        FATAL("Setting OSD timer failed");
        return false;
      }
      UpdateOSDInformation(isNameEnabled, isTimeEnabled, name, namePosition, dateFormatIndex, datePosition);
    }// enable time
  }// disable name
  return true;
}

void QIC32AVCVideoCapture::CheckStatus(double ts) {
  if (_lfcnt==_fcnt && _fcnt>500 ) {
    //HardwareManager::SetStatus(EC_VIDEO_AVC);
  }
  else {
    HardwareManager::ClearStatus(EC_VIDEO_AVC);
    _lfcnt=_fcnt;
  }
}

// TODO(jennifer) : complete below
bool QIC32AVCVideoCapture::InitializePTTagList(){
  string tag("Device_PT_Tag");
  string pan("_Pan");
  string tilt("_Tilt");
  string tempstring, tagNameKey, tagName;
  // Since PT tags are indexed from 1 in NVRAM
  for(int i=1; i<MAX_TAG+1; i++){
    tagNameKey.clear();
    tagNameKey = tag + numToString<uint32_t>(i,0);
    //FATAL("Tag namejkey : %s", STR(tagNameKey));
    tagName = SystemManager::GetNVRam(tagNameKey);
    if(tagName.empty()){
      _PTTagSpaceAvailability[i] = true;
    }
    else{
      PTPositionInfo positionInfo;

      // pan
      tempstring.clear();
      tempstring += tagNameKey + pan;
      //FATAL("Pan key : %s", STR(tempstring));

      tempstring = SystemManager::GetNVRam(tempstring);

      if((!tempstring.empty()) && isNumeric(tempstring)){
         positionInfo.position.pan = stringToNum<signed short>(tempstring);
      }
      else{
        FATAL("Invalid pan! It is empty or not a numeric");
        _PTTagSpaceAvailability[i] = true;
        return false;
      }

      // tilt
      tempstring.clear();
      tempstring += tagNameKey + tilt;
      //FATAL("Tilt key : %s", STR(tempstring));

      tempstring = SystemManager::GetNVRam(tempstring);

      if((!tempstring.empty()) && isNumeric(tempstring)){
         positionInfo.position.tilt = stringToNum<signed short>(tempstring);
      }
      else{
        FATAL("Invalid tilt! It is empty or not a numeric");
        _PTTagSpaceAvailability[i] = true;
        return false;
      }
      _PTTagSpaceAvailability[i] = false;
      positionInfo.nvramIndex = i;
      _PTTagList[tagName] = positionInfo;
    }
    //INFO("Tag name : %s", STR(tagName));
    //INFO("Saved in NVRAM index : %d", _PTTagList[tagName].nvramIndex);
    //INFO("With pan : %d, tilt : %d", _PTTagList[tagName].position.pan, _PTTagList[tagName].position.tilt);
  }// for
  return true;
}

bool QIC32AVCVideoCapture::DeleteTagInfo(string tagName, uint8_t nvramIndex){
  _PTTagList.erase(tagName);
  //INFO("Freeing nvram space with index : %d", nvramIndex);
  _PTTagSpaceAvailability[nvramIndex] = true;
  return true;
}

bool QIC32AVCVideoCapture::SetNVRamForPTTag(uint8_t nvramIndex, string tagName, string pan, string tilt){
  string tag("Device_PT_Tag");
  string panKey("_Pan");
  string tiltKey("_Tilt");
  string tempstring, tagNameKey;

  //INFO("About to set NVRAM index : %d with tag name : %s, pan : %s and tilt : %s", nvramIndex, STR(tagName), STR(pan), STR(tilt));
  tagNameKey = tag + numToString<uint32_t>(nvramIndex,0);
  //tagNameKey = tag + numToString<uint8_t>(nvramIndex,0);
  //FATAL("NVRAM Field, tag name : %s", STR(tagNameKey));
  if(!SystemManager::SetNVRam(tagNameKey, tagName, false))
    return false;

  tempstring = tagNameKey + panKey;
  //FATAL("NVRAM Field, pan : %s", STR(tempstring));
  if(!SystemManager::SetNVRam(tempstring, pan, false))
    return false;

  tempstring = tagNameKey + tiltKey;
  //FATAL("NVRAM Field, tilt : %s", STR(tempstring));
  if(!SystemManager::SetNVRam(tempstring, tilt, false))
    return false;

  return true;
}

bool QIC32AVCVideoCapture::ResetMotorPosition(){
  AdvMotorCtl_t motorControl;
  motorControl.TotalPositionNumbers = 0;
  motorControl.StartElementIndex = 1;
  if(QicSetAdvMotorControl(ADV_MOTOR_RESET, motorControl.TotalPositionNumbers, motorControl) < 0){
    FATAL("Clearing motor position failed");
    return false;
  }
  return true;
}
bool QIC32AVCVideoCapture::IsPTSupported(){
  NYI;
  return false;
  //INFO("Is PT Supported : %d", _isPTSupported);
  //return _isPTSupported;
}

bool QIC32AVCVideoCapture::CheckPanTiltBoundary(int16_t pan, int16_t tilt, int8_t& isLeftBoundary, int8_t& isRightBoundary, int8_t& isTopBoundary, int8_t& isBottomBoundary){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::IsPTTagExist(string tagName){
  NYI;
  return true;
}
bool QIC32AVCVideoCapture::GetAvailableTagSpace(uint8_t& index){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::GetCurrentPTPosition(Variant& PTPosition){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::RotatePT(int16_t horizontalStep, int16_t verticalStep){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::RotatePTToPosition(int16_t pan, int16_t tilt){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::RotatePTToTag(string tagName){
  NYI;
  return true;
}

Variant QIC32AVCVideoCapture::GetPTTagList(){
  Variant temp;
  NYI;
  return temp;
}

bool QIC32AVCVideoCapture::TagCurrentPTPosition(string tagName){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::EditTag(string oldTagName, string newTagName){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::RenameTag(string oldTagName, string newTagName){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::DeleteTag(string tagName){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::ClearPTTagList(){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::NavigatePT(){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::NavigatePTTo(queue<string>& tagList){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::StopPTRotate(){
  NYI;
  return true;
}

bool QIC32AVCVideoCapture::IsIRSupported(){
  return _isIRSupported;
}

bool QIC32AVCVideoCapture::SetIRLEDStatus(unsigned char mode){
  //FATAL("Set IR LED status start");
  if(QicSetIRControl(mode)){
    FATAL("Setting IR LED failed");
    return false;
  }
  return true;
}

bool QIC32AVCVideoCapture::GetIRLEDStatus(bool& status, string& IRLEDMode){
  //FATAL("GetIRLEDStatus start");
  unsigned char irStatus;
  unsigned char alsStatus;
  if(QicGetIRStatus(&irStatus, &alsStatus)){
    FATAL("Getting IR LED status failed");
    return false;
  }
  //FATAL("IR LED status : %d", irStatus);
  if(irStatus == 1)
    status = true;
  else if(irStatus == 0)
    status = false;

  if(alsStatus == 1)
    IRLEDMode = "auto";
  else if(alsStatus == 0)
    IRLEDMode =  "manual";

  //FATAL("IR LED status : %d, mode : %s", irStatus , STR(IRLEDMode));
  //FATAL("GetIRLEDStatus end");
  return true;
}

bool QIC32AVCVideoCapture::IsALSSupported(){
  //INFO("Is ALS Supported : %d", _isALSSupported);
  return _isALSSupported;
}

bool QIC32AVCVideoCapture::GetALS(unsigned short& als){
  //FATAL("GetALS start");
  if(QicGetALS(&als)){
    FATAL("Getting IR LED failed");
    return false;
  }
  //FATAL("GetALS end");
  return true;
}

bool QIC32AVCVideoCapture::GetAEInfo(unsigned short& autoExposure){
  //FATAL("GetAEInfo start");
  if(QicGetSensorAEInfo(&autoExposure)){
    FATAL("Getting Y-value failed");
    return false;
  }
  //FATAL("GetAEInfo end");
  return true;
}

bool QIC32AVCVideoCapture::IsSimulcastSupported(){
  //INFO("Is Simulcast Supported : %d", _isSimulcastSupported);
  return _isSimulcastSupported;
}

bool QIC32AVCVideoCapture::ReadMMIO(unsigned int address, unsigned int *value){
  //FATAL("Target address to be read is : %x", address);
  if(0 != QicMmioRead(address, value)){
    FATAL("Reading MMIO failed");
    return false;
  }
  //FATAL("Value at address %x is %d", address, *value);
  return true;
}

bool QIC32AVCVideoCapture::GetSubStreamEnableStatus(bool *status) {
  int j;
  for(j=0;j<16;j++)
    status[j]=_subStreamEnableStatus[j];
  return true;
}

void QIC32AVCVideoCapture::PrintDeviceVideoConstraints() {
  map<uint32_t,VideoConstraint>::iterator videoConstraintIterator;
  VideoConstraint videoConstraint;
  Resolution resolution;

  for(videoConstraintIterator=_deviceVideoCapabilities.begin(); videoConstraintIterator!=_deviceVideoCapabilities.end(); videoConstraintIterator++){
    videoConstraint = MAP_VAL(videoConstraintIterator);
    resolution.height = MAP_KEY(videoConstraintIterator) & 0x0000ffff;
    resolution.width = MAP_KEY(videoConstraintIterator) >> 16;
    INFO("resolution : %dx%d", resolution.width, resolution.height);
    INFO("Bitrate, max : %d, min : %d", videoConstraint.maxBitrate , videoConstraint.minBitrate);
    INFO("FPS, max : %d, min : %d", videoConstraint.maxFPS, videoConstraint.minFPS);
  }
}
void QIC32AVCVideoCapture::CheckIfSEIEnabled() {
  unsigned int value = 0;
  if(QicMmioRead(SEI_SWITCH_ADDR, &value)){
    FATAL("Reading data from address %d failed", SEI_SWITCH_ADDR);
  }
  FATAL("Data value from address %d : %d ", SEI_SWITCH_ADDR, value);
}
#endif
