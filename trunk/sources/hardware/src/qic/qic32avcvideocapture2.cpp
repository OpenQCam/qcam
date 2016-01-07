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
 *  Author  : S.J.Hung
 *  Purpose : AVC video capture for QIC1832 with motion detection support
 * ==================================================================
 */

#ifdef QIC1832

#include "qic/qic32avcvideocapture2.h"
#include "video/devicevideostream.h"
#include <sys/time.h>

#define DBG_SEI_MD_TIMESTAMP            1
#define DBG_SEI_MD_NUM_OF_MOVING_OBJS   2
#define DBG_SEI_MD_X                    3
#define DBG_SEI_MD_Y                    4
#define DBG_SEI_MD_WIDTH                5
#define DBG_SEI_MD_HEIGHT               6
#define DBG_SEI_CODE_PAGE            0x3F

static const uint8_t guid[] = {0x19, 0xA3, 0x39, 0x57, 0x4F, 0xBD, 0x4A, 0x0D, 0xB7, 0xCF, 0xE5, 0xD2, 0xC8, 0xB2, 0xDD, 0x5D};

BaseHardwareAbstractDevice* QIC32AVCVideoCapture2::GetInstance(string devNode, int pipe_write_fd) {
    static BaseHardwareAbstractDevice* _pDeviceInstance = NULL;

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
    _pDeviceInstance = new QIC32AVCVideoCapture2(fd, pipe_write_fd);
    if(!_pDeviceInstance->InitializeDevice()){
      delete _pDeviceInstance;
      return NULL;
    }
  }
  return _pDeviceInstance;
}

QIC32AVCVideoCapture2::QIC32AVCVideoCapture2(int32_t fd, int pipe_write_fd)
  : QIC32AVCVideoCapture(fd)
  ,_pipe_write_fd(pipe_write_fd)
  , previous_num_of_moving_objects(0)
{
  hadReadStreamStatus=false;
  int j;
  for(j=0;j<16;j++) {
    myStatus[j]=0;
    myStreamMap[j]=-1;
  }
  hadAllocTemporalLayerBuffer=false;
  countTemporalLayer=0;
  temporalLayerBufferCpy=NULL;
}

QIC32AVCVideoCapture2::~QIC32AVCVideoCapture2() {
}

uint32_t QIC32AVCVideoCapture2::AnalyzeSEIMessages(uint8_t* buffer, uint32_t size, uint32_t& sei_begin_offset)
{
    uint32_t i;
    bool foundSEI = false;
    uint8_t* sei;
    uint32_t sei_size;
    md_status_t md_status;
    int top = 0;

    /*DEBUG("0x%02x 0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x 0x%02x",
         buffer[0], buffer[1], buffer[2], buffer[3],
            buffer[4], buffer[5], buffer[6], buffer[7],
            buffer[8], buffer[9], buffer[10], buffer[11],
            buffer[12], buffer[13], buffer[14], buffer[15]
            );*/

    /* search for the SEI uuid */
    for(i=0;i<size-16;i++)
    {
        if(buffer[i] == guid[0])
        {
            if(memcmp(&buffer[i], guid, sizeof(guid)) == 0)
            {
                foundSEI = true;
                sei_begin_offset = i - 7;
                sei = &buffer[i+16];
                sei_size =buffer[i-1] - 16;
                break;
            }
        }
    }
//    DEBUG("foundSEI=%d\n", foundSEI);
    if(foundSEI == false)
        return 0;

    memset(&md_status, 0, sizeof(md_status));

    /* parse key/value */
    int codepage = 0;
    for(i=0;i<sei_size;i++)
    {
        int type = sei[i];
        int len = type >> 6;
        int value = 0;

        type = type & 0x3F;

        uint8_t* ptr = &sei[i+1];
        switch(len)
        {
        case 3:
                value = *ptr++;
                i++;
                // fall through
        case 2:
                value <<=8;
                value |= *ptr++;
                i++;
                // fall through
        case 1:
                value <<=8;
                value |= *ptr++;
                i++;
                // fall through
        case 0:
                value <<=8;
                value |= *ptr++;
                i++;
                // fall through
        }

        if(type == DBG_SEI_CODE_PAGE)
        {
            codepage = value;
        }
        else if(codepage == 1)
        {
            switch(type)
            {
            case DBG_SEI_MD_TIMESTAMP:
                md_status.timestamp = value;
                break;
            case DBG_SEI_MD_NUM_OF_MOVING_OBJS:
                md_status.number_of_moving_objects = value;
                break;
            case DBG_SEI_MD_X:
                md_status.moving_objects[top].x = value;
                break;
            case DBG_SEI_MD_Y:
                md_status.moving_objects[top].y = value;
                break;
            case DBG_SEI_MD_WIDTH:
                md_status.moving_objects[top].width = value;
                break;
            case DBG_SEI_MD_HEIGHT:
                md_status.moving_objects[top].height = value;
                top++;
                break;
            default:
                FATAL("got unknown entry (%d, %d, %d)", codepage, type, value);
            }
        }
    }

    //INFO("md_status.number_of_moving_objects=%d\n", md_status.number_of_moving_objects);

    /* write md_status_t to pipe */
    if(previous_num_of_moving_objects != 0 || md_status.number_of_moving_objects != 0)
    {
      FATAL("before pipe");
        if(write(_pipe_write_fd, &md_status, sizeof(md_status)) == -1)
        {
            FATAL("write to pipe fail, %s\n", strerror(errno));
        }
        FATAL("after pipe");
    }
    previous_num_of_moving_objects = md_status.number_of_moving_objects;
    uint32_t sei_end_offset = (uint32_t)(sei - buffer) + sei_size + 1;
    return sei_end_offset;
}

bool QIC32AVCVideoCapture2::GenerateStreamMap(bool streamAvailable[], int *streamID) {
  int j;
  int stream_count=0;
  for(j=0;j<16;j++) {
    if(streamAvailable[j]==1){
      switch(stream_count){
        case 0:
          INFO("j:%d, SIMULCAST_STREAM0",j);
          streamID[j]=SIMULCAST_STREAM0;
          break;
        case 1:
          INFO("j:%d, SIMULCAST_STREAM1",j);
          streamID[j]=SIMULCAST_STREAM1;
          break;
        case 2:
          INFO("j:%d, SIMULCAST_STREAM2",j);
          streamID[j]=SIMULCAST_STREAM2;
          break;
        case 3:
          INFO("j:%d, SIMULCAST_STREAM3",j);
          streamID[j]=SIMULCAST_STREAM3;
          break;
        default:
          FATAL("stream count is more than four");
          return false;
          break;
      }
      stream_count++;
    }
  }
  return true;
}

void QIC32AVCVideoCapture2::SendDataToCarrierAndSubStream(int rawStreamID,
    int temporalID, uint8_t *pData, uint32_t dataLength, uint64_t ts, uint32_t type)
{
  int j;
  if(!hadAllocTemporalLayerBuffer) {
    for(j=0;j<4;j++){
      if(myStreamMap[rawStreamID*4+j]!=-1)
        countTemporalLayer++;
    }
    temporalLayerBufferCpy=(uint8_t **) malloc(countTemporalLayer* sizeof(uint8_t *));
    hadAllocTemporalLayerBuffer=true;
  }
  for(j=1;j<countTemporalLayer;j++){
    temporalLayerBufferCpy[j] = (uint8_t *) malloc(dataLength);
    memcpy(temporalLayerBufferCpy[j], pData, dataLength);
  }
  temporalLayerBufferCpy[0]=pData;

  int is_pframe=check_for_P_frame(pData,dataLength);
  if(!is_pframe) {
    //every channel need keyframe
    for(j=0;j<countTemporalLayer;j++) {
      SendDataToCarrier(temporalLayerBufferCpy[j], dataLength, ts,
        _deviceVideoStreams[myStreamMap[rawStreamID*4+j]]->GetCarrierId(),HW_DATA_VIDEO);
    }
  }else {
    //send p frame to substream
    for(j=0;j<countTemporalLayer-temporalID;j++) {
      SendDataToCarrier(temporalLayerBufferCpy[j], dataLength, ts,
        _deviceVideoStreams[myStreamMap[rawStreamID*4+j]]->GetCarrierId(),HW_DATA_VIDEO);
    }
  }//end of sending both of P frame and I frame

  for(j=1;j<countTemporalLayer;j++)
    free(temporalLayerBufferCpy[j]);
}

bool QIC32AVCVideoCapture2::OnReadEvent() {
//  INFO("OnReadEvent");
  if(!hadReadStreamStatus){
    hadReadStreamStatus=true;
    if(GetSubStreamEnableStatus(myStatus)) {
      INFO("get substream status success");
      int j;
      for(j=0;j<16;j++)
        INFO("i=%d, status:%d",j,myStatus[j]);
      if(!GenerateStreamMap(myStatus,myStreamMap))
        FATAL("Unable to set stream Map");
      else{
        INFO("gen stream map success");
        int j;
        for(j=0;j<16;j++)
          INFO("stream map i=%d, status:%d",j,myStreamMap[j]);
      }
    }
  }
  int rawStreamID;
  int streamID;
  int temporalID=0;
  struct v4l2_buffer buf = {0};
  uint64_t PTS = 0;

  CLEAR(*_pHeader);
  // Dequeue buffer

  if(!v4l2_dqbuf(buf)){
  INFO("v4l2_dqbuf fail");
    return false;
  }
  // Retrieve stream data and send to corresponding carrier
  if(buf.index < _numOfV4L2Buffers && buf.index >=0){

    rawStreamID = (uint16_t) IdentifySubStream((uint8_t *)_pV4L2Buffers[buf.index].start, buf.bytesused);
//INFO("before get_stream_temporal_id");

    switch(rawStreamID) {
      case SIMULCAST_STREAM0:
        rawStreamID=0;
        break;
      case SIMULCAST_STREAM1:
        rawStreamID=1;
        break;
      case SIMULCAST_STREAM2:
        rawStreamID=2;
        break;
      case SIMULCAST_STREAM3:
        rawStreamID=3;
        break;
      default:
        FATAL("undefine stream id type %d",rawStreamID);
        break;
    }
    if(myStatus[rawStreamID*4+1]!=0) {//has temporal lay setting
      temporalID = get_stream_temporal_id((uint8_t*)_pV4L2Buffers[buf.index].start, buf.bytesused);
    }
    streamID=myStreamMap[(rawStreamID*4)+temporalID];

    // This is for video content analysis
    /*
    if(SIMULCAST_STREAM0 == streamID){
      //fwrite(((uint8_t *)_pV4L2Buffers[buf.index].start), 1, buf.bytesused, fp);
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

    // Analyze SEI message
    /*uint32_t sei_begin_offset;
    int sei_end_offset = AnalyzeSEIMessages((uint8_t *)_pV4L2Buffers[buf.index].start, buf.bytesused, sei_begin_offset);
    uint8_t* data = (uint8_t *)_pV4L2Buffers[buf.index].start;
    uint32_t data_size = buf.bytesused;
    if(sei_end_offset)
    {
        memmove(data + sei_end_offset - sei_begin_offset, data, sei_begin_offset);
        data += sei_end_offset - sei_begin_offset;
        data_size -= sei_end_offset - sei_begin_offset;
    }
    if((sei_end_offset - sei_begin_offset)>buf.bytesused) {
      FATAL("SEI error");
      //return false;
    }*/
    // Split SPS and PPS from I frame
      PTS = (uint64_t)((buf.timestamp.tv_sec*1000)+(buf.timestamp.tv_usec/1000));
    if(temporalID==0 && myStatus[rawStreamID*4+1]==0) {//no temporal lay setting
      SendDataToCarrier(((uint8_t *)_pV4L2Buffers[buf.index].start), buf.bytesused, PTS, _deviceVideoStreams[streamID]->GetCarrierId(), HW_DATA_VIDEO);
    } else {//temporal layer~~~~~~~~
      SendDataToCarrierAndSubStream(rawStreamID, temporalID, ((uint8_t *)_pV4L2Buffers[buf.index].start), buf.bytesused, PTS, HW_DATA_VIDEO);
    }

    if(!v4l2_qbuf(buf)){
      FATAL("Unable to queue buffer");
      return false;
    }
  }else{
    FATAL("Out of buffer index, %d, max. %d", buf.index, _numOfV4L2Buffers);
    return false;
  }

  return true;
}

/*bool QIC32AVCVideoCapture2::StartCapture()
{
    bool ret = QIC32AVCVideoCapture::StartCapture();

<<<<<<< HEAD
    // enable debug SEI
=======
    // enable debug SEI
>>>>>>> c2fe997aeb4b2fc21258fa119d4534575d849e1c
    if(QicMmioWrite(0x6F0000A0, 1) != 0)
    {
        FATAL("Unable to enable SEI message\n");
    }

    return ret;
}*/

#endif
