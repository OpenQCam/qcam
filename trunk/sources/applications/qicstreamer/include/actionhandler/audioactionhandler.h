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

#ifndef _AUDIOACTIONHANDLER_H
#define _AUDIOACTIONHANDLER_H

#include "actionhandler/baseactionhandler.h"
#include "interface/deviceinterface.h"

class BaseAudioCaptureDevice;
class Speaker;
class WSOutNetAVStream;
class BaseOutNetStream;

namespace app_qicstreamer {

  class AudioActionHandler
  : public BaseActionHandler{
    private:
      typedef RestHTTPCode (AudioActionHandler::*Action)(Variant &req, Variant &res);
      typedef map<string, Action> ActionMap;
      typedef map<bool, ActionMap*> CollectionMap;
      map<string, CollectionMap* > _actionMaps;

      uint64_t _streamType;
      string _filePath;
      BaseAudioCaptureDevice *_pMICInstance;
      Speaker *_pSpeakerInstance;

      //data used do async command
      string _uri;
      string _tid;

    public:
      virtual void OnCmdFinished(uint32_t msgId,
                                 uint8_t *pData, uint32_t dataLength);

      AudioActionHandler(string apiversion, BaseClientApplication *pApplication);
      virtual ~AudioActionHandler();
      virtual bool Initialize();
      virtual bool IsValidAction(map<string, CollectionMap*> *map,
                                 string collection,
                                 string id,
                                 bool isReadyOnly);
      virtual bool IsValidAction(string action);
      virtual RestHTTPCode DoAction(BaseProtocol *pFrom,
                                    bool isRO,
                                    vector<string> &resource,
                                    Variant &parameters,
                                    Variant &message);
    private:
      // common
      // Audio Capture Actions
      RestHTTPCode GetMicStream(Variant &parameters, Variant &message);
      RestHTTPCode PostMicStream(Variant &parameters, Variant &message);
      //RestHTTPCode GetMicDevice(Variant &parameters, Variant &message);
      //RestHTTPCode PostMicDevice(Variant &parameters, Variant &message);
      RestHTTPCode GetMicSampleRate(Variant &parameters, Variant &message);
      RestHTTPCode PostMicSampleRate(Variant &parameters, Variant &message);

      //Audio Playback Actions
      //RestHTTPCode GetSpeakerDevice(Variant &parameters, Variant &message);
      RestHTTPCode PostSpeakerDevice(Variant &parameters, Variant &message);
      RestHTTPCode GetSpeakerStream(Variant &parameters, Variant &message);
      RestHTTPCode GetFileList(Variant &parameters, Variant &message);
      RestHTTPCode PostSpeakerStream(Variant &parameters, Variant &message);
      RestHTTPCode PlayFile(Variant &parameters, Variant &message);
      RestHTTPCode PostSpeakerVolume(Variant &parameters, Variant &message);
      RestHTTPCode GetSpeakerVolume(Variant &parameters, Variant &message);
  };

}

#endif
