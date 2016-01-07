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

#include "websocket/wsclientcmdprotocol.h"
//thelib
#include "protocols/avhttpstream/avstreamdefine.h"
#include "protocols/http/basewebsocketappprotocolhandler.h"
#include "protocols/http/outboundhttpwsprotocol.h"
#include "streaming/baseinstream.h"
#include "streaming/streamsmanager.h"
#include "streaming/streamstypes.h"
#include "streaming/wsoutnetmsgstream.h"
#include "websocket/subprotocoltypes.h"
//applications
#include "qicstreamerapplication.h"

using namespace app_qicstreamer;

WSClientCMDProtocol::WSClientCMDProtocol()
  : _pStream(0)
{
}

WSClientCMDProtocol::~WSClientCMDProtocol()
{
  if(_pStream != NULL) {
    delete _pStream;
  }
}

bool WSClientCMDProtocol::Initialize(Variant &parameters)
{
  QICStreamerApplication *pApp = (QICStreamerApplication *) GetApplication();
  if(!pApp) {
    WARN("Unable to get application instance");
    return false;
  }

  // Find system stream and link it
  string randomOutStreamName = md5(generateRandomString(8), true);
  StreamsManager *pStreamsManager= pApp->GetStreamsManager();
  _pStream = new WSOutNetMSGStream(reinterpret_cast<BaseWSSubProtocol*>(this),
                                   pStreamsManager, randomOutStreamName);
  if(!_pStream) {
    FATAL("Unable to create wsoutnetmsgstream with stream name(%s)", STR(randomOutStreamName));
    delete _pStream;
    return false;
  }

  // Link to system stream
  BaseInStream* pInStream=reinterpret_cast<BaseInStream*>
      (pStreamsManager->FindFirstByTypeByName(ST_IN_MSG_SYSTEM, QIC_STREAM_SYSTEMMSG));

  if (!pInStream) {
    FATAL("Unable to find system stream");
    return false;
  }
  pInStream->Link(_pStream);

  INFO("create push stream %s", STR(randomOutStreamName));
  return true;
}

bool WSClientCMDProtocol::SignalInputTextFrame(IOBuffer &buffer, WebSocketDataFrame &dataFrame)
{
  if (_pFarProtocol) {
    uint32_t offset = dataFrame._payloadOffset;
    uint8_t *pBuf = (uint8_t*)(GETIBPOINTER(buffer)+offset);

    BaseWebSocketAppProtocolHandler *pHandler=
        reinterpret_cast<OutboundHTTPWSProtocol*>(_pFarProtocol)->GetProtocolHandler();

    return pHandler->ProcessWebsocketText(_pFarProtocol, pBuf, dataFrame._header.payloadLength);
  }
  return false;
}
bool WSClientCMDProtocol::SignalInputBinaryFrame(IOBuffer &buffer, WebSocketDataFrame &dataFrame)
{
  NYI;
  return true;
}
