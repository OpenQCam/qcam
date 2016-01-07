/* ==================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 * (C) COPYRIGHT Quanta Computer Inc. ALL RIGHTS RESERVED
 *
 * ==================================================================
 * ------------------------------------------------------------------
 *  Date    : 2013/03/27
 *  Version : 0.2
 *  Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 *  Purpose : 0.1 Create Multimedia stream over HTTP
 *            0.2 Revised design
 *  =================================================================
 */

#ifdef HAS_PROTOCOL_AVHTTPSTREAM

#include "appprotocolhandler/avhttpstreamappprotocolhandler.h"
#include "hardwaremanager.h"
#include "storage/basestoragedevice.h"
#include "application/baseclientapplication.h"
#include "protocols/avhttpstream/avstreamdefine.h"
#include "protocols/avhttpstream/inboundavhttpstreamprotocol.h"
#include "protocols/avhttpstream/streaming/mjpg4httpstream.h"
#include "protocols/avhttpstream/streaming/snapshot4httpstream.h"
#include "protocols/avhttpstream/streaming/mp44httpstream.h"
#include "protocols/avhttpstream/streaming/infile4httpstream.h"

using namespace app_qicstreamer;


AVHTTPStreamAppProtocolHandler::AVHTTPStreamAppProtocolHandler(
                Variant &configuration)
: BaseAVHTTPStreamAppProtocolHandler(configuration),
  _pStreamsManager(0)
{
  _supportedStreamType[QIC_STREAM_AVC] = QIC_STREAM_TYPE_AVC;
  _supportedStreamType[QIC_STREAM_MJPG] = QIC_STREAM_TYPE_MJPG;
  _supportedStreamType[QIC_STREAM_SNAPSHOT] = QIC_STREAM_TYPE_JPEG;
  _supportedStreamType[QIC_STREAM_MP4FILE] = QIC_STREAM_TYPE_MP4FILE;
  _supportedStreamType[QIC_STREAM_G711] = QIC_STREAM_TYPE_G711;
  _supportedStreamType[QIC_STREAM_WAVE] = QIC_STREAM_TYPE_WAVE;
  _supportedStreamType[QIC_STREAM_AAC] = QIC_STREAM_TYPE_AAC;
  _supportedStreamType[QIC_STREAM_MP3] = QIC_STREAM_TYPE_MP3;
}



AVHTTPStreamAppProtocolHandler::~AVHTTPStreamAppProtocolHandler() {
  _supportedStreamType.clear();
}

bool AVHTTPStreamAppProtocolHandler::ParseHttpRangeToken (Variant& msg, uint64_t &offset)
{
  if (msg.HasKey("headers")) {
    Variant headers = msg["headers"];
    if (headers.HasKey("Range")) {
      vector<string> fullRangeToken;
      split ((string)headers["Range"], "=", fullRangeToken);
      if (fullRangeToken.size()>=2) {
        vector<string> valueRangeToken;
        split (fullRangeToken[1], "-", valueRangeToken);
        if (!valueRangeToken.empty())
          if (isNumeric(valueRangeToken[0]))
            offset = atoi(STR(valueRangeToken[0]));
          else return false;
        else return false;
      }
      else return false;
    }
    else return false;
  }
  else return false;

  return true;
}

bool AVHTTPStreamAppProtocolHandler::ProcessMessage(InboundAVHTTPStreamProtocol *pFrom, Variant &message) {
  string request = (string) message[QIC_STREAM_REQUEST];
  string apiVersion = (string) message[QIC_STREAM_API_VERSION];
  string streamName = (string) message[QIC_STREAM_NAME];
  Variant parameters = (Variant) message[QIC_STREAM_PARAMETERS];

  _pStreamsManager = GetApplication()->GetStreamsManager();
  //0. Check if everything is ok
  if(!IsValidRequest(request)){
    FATAL("Invalid request: %s", STR(request));
    return false;
  }
  if(!IsValidAPIVersion(apiVersion)){
    FATAL("Invalid API version: %s", STR(apiVersion));
    return false;
  }
  if(!MAP_HAS1(_supportedStreamType, streamName)){
    FATAL("Invalid stream type: %s", STR(streamName));
    return false;
  }

  BaseInStream *pInStream = NULL;
  BaseOutHTTPStream *pOutStream = NULL;
  uint64_t fileSize, offset = 0;
  string fileName, filePath="";
  string referer;
  uint32_t fps = 30;

  //Get range token, offset and put it in the response header
  Variant headers = pFrom->GetHeaders();
  ParseHttpRangeToken (headers, offset);

  // If creating instream or outstream, remeber to register stream into
  // protocol in case of memory leak
  uint64_t streamType = (uint64_t)_supportedStreamType[streamName];
  switch(streamType) {
    case QIC_STREAM_TYPE_MJPG:
      if(parameters.HasKey("r")){
        referer = (string) parameters["r"];
      }
      //TODO(TIM): Check max. streams number here and reject connect to avoid running out of resource.
      //NOTE(Tim): To avoid create multiple-mjpgstream for the same client
      if(referer.length() > 0){
        BaseOutHTTPStream *pStream = (BaseOutHTTPStream *) FindOutStream(md5(referer, true));
        if(pStream != NULL){
          BaseProtocol *pProtocol = pStream->GetProtocol();
          pProtocol->GracefullyEnqueueForDelete();
        }
      } else{
        referer = generateRandomString(8);
      }
      // check fps and set into mjpg stream object
      if(parameters.HasKey(QIC_STREAM_MJPG_FPS)){
        if(isNumeric((string)parameters[QIC_STREAM_MJPG_FPS])){
          fps = atoi(STR(parameters[QIC_STREAM_MJPG_FPS]));
        }
      }

      if(fps > 0){
        pOutStream = (BaseOutHTTPStream *) new MJPG4HTTPStream(reinterpret_cast<BaseProtocol *>(pFrom),
            _pStreamsManager, md5(referer, true));
        ((MJPG4HTTPStream*)pOutStream)->SetFrameRate(fps);
      }else{
        pOutStream = (BaseOutHTTPStream *) new Snapshot4HTTPStream(reinterpret_cast<BaseProtocol *>(pFrom),
            _pStreamsManager, md5(referer, true));
      }
      pFrom->RegisterOutStream(reinterpret_cast<BaseOutHTTPStream *>(pOutStream));
      pInStream = FindInStream(streamName);
      break;

    case QIC_STREAM_TYPE_JPEG:
      //TODO(TIM): Check max. streams number here and reject connect to avoid running out of resource.
      pOutStream = (BaseOutHTTPStream *) new Snapshot4HTTPStream(reinterpret_cast<BaseProtocol *>(pFrom),
          _pStreamsManager, md5(generateRandomString(8), true));
      pFrom->RegisterOutStream(reinterpret_cast<BaseOutHTTPStream *>(pOutStream));
      pInStream = FindInStream(QIC_STREAM_MJPG);
      break;

    case QIC_STREAM_TYPE_MP4FILE:
      if(parameters.HasKey(QIC_STREAM_MP4_FILENAME))
        fileName=(string)parameters[QIC_STREAM_MP4_FILENAME];
      else {
        WARN ("no filename parameter");
        return false;
      }

      if(fileName.length() == 0){
        FATAL("The requested file is not found");
        return false;
      }

      //already created before
      if (pFrom->IsOutStreamCreated()) {
        if (pFrom->IsRegisteredOutStream(filePath+fileName)) {
          uint32_t offset=0;
          Variant msg = pFrom->GetHeaders();
          if (msg.HasKey("headers")) {
            Variant headers = msg["headers"];
            if (headers.HasKey("Range")) {
              string range = (string)headers["Range"];
            }
          }
          pFrom->RePlayStream(offset);
          return true;
        }
        else {
          WARN ("invalid file name");
          pFrom->Send400BadRequest();
          return false;
        }
      }
      else { //outstream not created
        pInStream = CreateInFile4HTTPStream(pFrom, filePath, fileName, fileSize);
        if(pInStream == NULL){
          FATAL("Create infile stream failed");
          return false;
        }
        // register to clean up memory while destructing
        pFrom->RegisterInStream(pInStream, filePath+fileName);
        pOutStream = (BaseOutHTTPStream *) new MP44HTTPStream(reinterpret_cast<BaseProtocol *>(pFrom),
            _pStreamsManager, md5(generateRandomString(8), true), fileSize);
        //FIXME-should use outstream to find out instream  filepath+filename
        pFrom->RegisterOutStream(pOutStream);
      }
      break;

    default:
      break;
  }

  if(pInStream == NULL){
    FATAL("Unable to find instream %s", STR(streamName));
    return false;
  }

  if(pOutStream == NULL){
    FATAL("Unable to create outstream %s", STR(streamName));
    return false;
  }

  if (!pInStream->Link(reinterpret_cast<BaseOutStream *>(pOutStream))) {
    FATAL("Unable to link to the in stream");
    return false;
  } else{
    if(streamType == QIC_STREAM_TYPE_MP4FILE){
      //Send http response first
      double ats = 0.0, pts = -1.0;
      double dOffset = (double)offset;
      pInStream->SignalSeek(dOffset);
      offset = (uint64_t) dOffset;
      (reinterpret_cast<InboundAVHTTPStreamProtocol *>(pFrom))->SendHttpResponseHeader(206, fileSize, offset);
      pInStream->SignalPlay(ats, pts);
    }
  }
 return true;
}

bool AVHTTPStreamAppProtocolHandler::IsValidRequest(string request) {
  return ((request == "video") || (request == "audio"));
}

bool AVHTTPStreamAppProtocolHandler::IsValidAPIVersion(string apiVersion) {
  return (apiVersion == QIC_STREAM_CURRENT_API_VERSION);
}

BaseInStream * AVHTTPStreamAppProtocolHandler::FindInStream(string streamName) {

  map<uint32_t, BaseStream *> inStreams = _pStreamsManager->FindByTypeByName( ST_IN, streamName, true, true);

  if (inStreams.size() != 1) { // Hardware related stream should be only one
    FATAL("Stream %s not found", STR(streamName));
    return NULL;
  }
  return (BaseInStream *) MAP_VAL(inStreams.begin());

}

BaseOutHTTPStream * AVHTTPStreamAppProtocolHandler::FindOutStream(string streamName) {

  map<uint32_t, BaseStream *> outStreams = _pStreamsManager->FindByTypeByName( ST_OUT, streamName, true, true);

  if (outStreams.size() != 1) { // Hardware related stream should be only one
    FATAL("Stream %s not found", STR(streamName));
    return NULL;
  }
  return (BaseOutHTTPStream *) MAP_VAL(outStreams.begin());

}



BaseInStream* AVHTTPStreamAppProtocolHandler::CreateInFile4HTTPStream(InboundAVHTTPStreamProtocol *pProtocol, string filePath, string fileName, uint64_t &size) {

  Variant metaData;
  metaData[IN_FILE_STREAM_TYPE] = ST_IN_FILE_MP4;
  metaData[IN_FILE_STREAM_FILE_NAME] = fileName;
  metaData[IN_FILE_STREAM_FILE_DIR] = filePath;
  metaData[IN_FILE_STREAM_FILE_PATH] = filePath + fileName;
  metaData[IN_FILE_STREAM_NAME] = fileName;

  InFile4HTTPStream *pInFileStream = InFile4HTTPStream::GetInstance(pProtocol, _pStreamsManager, metaData);
  if(pInFileStream == NULL){
    FATAL("request file %s cannot open", STR(metaData[IN_FILE_STREAM_FILE_PATH]));
    return NULL;
  }
  if(!pInFileStream->Initialize(metaData, true)){
    FATAL("Unable to initialize file inbound stream");
    delete pInFileStream;
    return NULL;
  }

  //_message[QIC_STREAM_MP4_FILESIZE] = metaData[IN_FILE_STREAM_FILE_SIZE];
  size = metaData[IN_FILE_STREAM_FILE_SIZE];
  return reinterpret_cast<BaseInStream *>(pInFileStream);
}






#endif /* HAS_PROTOCOL_AVHTTPSTREAM */
