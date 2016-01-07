/*
 * Copyright (c) 2013, Three Ocean (to@bcloud.us). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "websocket/wsservermjpgprotocol.h"
//thelib
#include "protocols/avhttpstream/avstreamdefine.h"
#include "protocols/http/basewebsocketappprotocolhandler.h"
#include "protocols/http/inboundhttpwsprotocol.h"
#include "streaming/baseinstream.h"
#include "streaming/streamsmanager.h"
#include "streaming/streamstypes.h"
#include "streaming/wsoutnetmjpgstream.h"
#include "websocket/subprotocoltypes.h"
//applications
#include "qicstreamerapplication.h"


using namespace app_qicstreamer;

WSServerMJPGProtocol::WSServerMJPGProtocol()
  : _pStream(0)
{
}

WSServerMJPGProtocol::~WSServerMJPGProtocol()
{
  if(_pStream != NULL) {
    delete _pStream;
  }
}

bool WSServerMJPGProtocol::IsValidDevice(Variant &parameters)
{
  return true;
}
bool WSServerMJPGProtocol::Initialize(Variant &parameters)
{
  QICStreamerApplication *pApp = (QICStreamerApplication *) GetApplication();
  if(!pApp) {
    WARN("Unable to get application instance");
    return false;
  }

  // Find system stream and link it
  string randomOutStreamName = md5(generateRandomString(8), true);
  StreamsManager *pStreamsManager= pApp->GetStreamsManager();
  _pStream = new WSOutNetMJPGStream(reinterpret_cast<BaseWSSubProtocol*>(this),
                                    pStreamsManager, randomOutStreamName);
  if(!_pStream) {
    FATAL("Unable to create wsoutnetmsgstream with stream name(%s)", STR(randomOutStreamName));
    delete _pStream;
    return false;
  }

  // Link to system stream
  BaseInStream* pInStream=reinterpret_cast<BaseInStream*>
      (pStreamsManager->FindFirstByTypeByName(ST_IN_CAM_MJPG, QIC_STREAM_MJPG));

  if (!pInStream) {
    FATAL("Unable to find system stream");
    return false;
  }
  pInStream->Link(_pStream);

  INFO("create push stream %s", STR(randomOutStreamName));
  return true;
}

//bool WSServerMJPGProtocol::TimePeriodElapsed()
//{
//  if(_pRedis->IsPublisherWaiting(_pStream->GetName())) {
//    _pRedis->DecreaseWaitingCounter(_pStream->GetName());
//    //INFO("yes, client(%s) is waiting", STR(_cameraId));
//    return true;
//  }
//  //IOHandlerManager::EnqueueForDelete(_pTimer);
//  BaseProtocol *pProtocol = GetFarProtocol();
//  if(pProtocol != NULL) {
//    //INFO("no, delete connection %d", pProtocol->GetId());
//    INFO("Camera(%s) connection(%d) has been idle for a while, delete it", STR(_cameraId), GetId());
//    pProtocol->EnqueueForDelete();
//    return true;
//  }
//  WARN("Cannot handle time event");
//  return false;
//}

bool WSServerMJPGProtocol::SignalInputTextFrame(IOBuffer &buffer, WebSocketDataFrame &dataFrame)
{
  NYI;
  return true;
}
bool WSServerMJPGProtocol::SignalInputBinaryFrame(IOBuffer &buffer, WebSocketDataFrame &dataFrame)
{
  NYI;
  return true;
}
