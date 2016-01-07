// ==================================================================
// This confidential and proprietary software may be used only as
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

#ifdef HAS_PROTOCOL_CLI
#ifndef _RECORDACTIONHANDLER_H
#define _RECORDACTIONHANDLER_H

#include "actionhandler/baseactionhandler.h"
#include "protocols/avhttpstream/avstreamdefine.h"

class BaseInStream;
class BaseMP4MuxOutStream;
class CVRMuxOutStream;
class BaseAVCVideoCapture;

#define RECORD_ERROR_CODE(errorType) (EC_500_RECORD_ERROR+errorType)
enum RestErrorCodeRecord {
  EC_RECORD_NO_MANUAL_MUXSTREAM,
  EC_RECORD_NO_EVENT_MUXSTREAM,
  EC_RECORD_NO_VIDEO_STREAM,
  EC_RECORD_NO_AUDIO_STREAM,
  EC_RECORD_MANUAL_MUX_ERR,
  EC_RECORD_NOT_ENOUGH_SPACE,
  EC_RECORD_NO_STROAGE_DEVICE,
  EC_RECORD_START_FAIL,
};


class BaseAVCVideoCapture;
class CVRMuxOutStream;

using namespace std;
namespace app_qicstreamer {

  class RecordActionHandler
  : public BaseActionHandler {
    private:
      typedef RestHTTPCode (RecordActionHandler::*Action)(Variant &req, Variant &res);
      map<string, Action> _actionMap;
      BaseMP4MuxOutStream *_pMP4MuxOutStream;
      BaseAVCVideoCapture *_pAVCCaptureInstance;
      BaseInStream *_pAVCInStream;
      BaseInStream *_pPCMInStream;
      CVRMuxOutStream *_pCVRMuxStream;
//      BaseInStream *_pMP3InStream;
      uint32_t _maxFrames;

    public:
      RecordActionHandler(string apiversion, BaseClientApplication *pApplication);
      virtual ~RecordActionHandler();
      virtual bool Initialize();
      virtual bool IsValidAction(string action);
      virtual RestHTTPCode DoAction(BaseProtocol *pFrom,
                                    bool isRO,
                                    vector<string> &resource,
                                    Variant &parameters,
                                    Variant &message);
      virtual void OnCmdFinished(uint32_t msgId, uint8_t *pData,
                                 uint32_t dataLength);

    private:
      // Record relevant procedure
      // Actions
      RestHTTPCode QICStartRecord(Variant &params, Variant &message);
      RestHTTPCode QICStopRecord(Variant &params, Variant &message);
      RestHTTPCode StartRecord(Variant &params, Variant &message);
      RestHTTPCode StopRecord(Variant &params, Variant &message);
      RestHTTPCode GetStatus(Variant &params, Variant &message);
      RestHTTPCode GetFilesList(Variant &params, Variant &message);
      RestHTTPCode SetEventFileDuration(Variant &params, Variant &message);
      RestHTTPCode GetEventFileDuration(Variant &params, Variant &message);

      RestHTTPCode CreateVideoRecordThread(Variant &params, Variant &message);
      RestHTTPCode DumpAVCStream(Variant &params, Variant &message);

      bool SetParameters(Variant &params, Variant &message);
      bool ProcessDefaultSettings(Variant &config);
  };
}

#endif
#endif
