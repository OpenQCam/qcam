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


#include "websocket/wsclientsubprotocolfactory.h"
#include "websocket/wsclientmjpgprotocol.h"
#include "websocket/wsclientcmdprotocol.h"
#include "websocket/wsclientavcprotocol.h"
#include "websocket/wsclientpushpcmprotocol.h"

using namespace app_qicstreamer;

WSClientSubProtocolFactory::WSClientSubProtocolFactory()
{
}

WSClientSubProtocolFactory::~WSClientSubProtocolFactory()
{
}

vector<uint64_t> WSClientSubProtocolFactory::HandledProtocols()
{
  vector<uint64_t> result;

  ADD_VECTOR_END(result, SPT_REG);
  ADD_VECTOR_END(result, SPT_CMD);
  ADD_VECTOR_END(result, SPT_MJPG);
  ADD_VECTOR_END(result, SPT_AVC);
  ADD_VECTOR_END(result, SPT_PUSHWAVE);
  return result;
}

BaseProtocol *WSClientSubProtocolFactory::SpawnProtocol(uint64_t type, Variant &parameters)
{
  BaseProtocol *pResult = NULL;
  switch (type) {
    case SPT_MJPG:
      pResult = new WSClientMJPGProtocol();
      break;
    case SPT_AVC:
      pResult = new WSClientAVCProtocol();
      break;
    case SPT_PUSHWAVE:
      pResult = new WSClientPushPCMProtocol();
      break;
    case SPT_CMD:
    case SPT_REG:
      pResult = new WSClientCMDProtocol();
      break;
    default:
      FATAL("Spawning protocol %s not yet implemented",
          STR(tagToString(type)));
      break;
  }

  return pResult;
}
