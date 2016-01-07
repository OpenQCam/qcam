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
#include <fstream>
#include "websocket/wsserverfileprotocol.h"
//thelib
#include "protocols/avhttpstream/avstreamdefine.h"
#include "protocols/http/basewebsocketappprotocolhandler.h"
#include "protocols/http/inboundhttpwsprotocol.h"
#include "streaming/baseinstream.h"
#include "streaming/streamsmanager.h"
#include "streaming/streamstypes.h"
#include "streaming/wsoutnetmjpgstream.h"
#include "websocket/subprotocoltypes.h"
//hardware
#include "system/systemmanager.h"
#include "system/nvramdefine.h"
//applications
#include "qicstreamerapplication.h"


using namespace app_qicstreamer;

WSServerFileProtocol::WSServerFileProtocol()
{
}

WSServerFileProtocol::~WSServerFileProtocol()
{
}

string WSServerFileProtocol::GetSha1Sum()
{
  FILE *fp;
  char buf[128];
  string output;
  string cmd="sha1sum " + _filename + "| cut -c 1-40";

  fp = popen(cmd.c_str(), "r");
  if (fp == NULL) {
    DEBUG("Failed to run command\n");
    return "";
  }
  //Read the output a line at a time - output it.
  while (fgets(buf, sizeof(buf)-1, fp) != NULL) {
    output = output + buf;
  }
  pclose(fp);

  printf ("output:%s", output.c_str());
  return output;
}

bool WSServerFileProtocol::Initialize(Variant &parameters)
{
  _directory=SystemManager::GetNVRam(NVRAM_STORAGE_MOUNT_PATH);
  DEBUG ("initialize");
  return true;
}

bool WSServerFileProtocol::SignalInputTextFrame(IOBuffer &buffer, WebSocketDataFrame &dataFrame)
{
#define FILENAME "filename"
#define CHKSUM   "chksum"
  //Save the filename
  uint32_t offset = dataFrame._payloadOffset;
  uint8_t *pBuf = (uint8_t*)(GETIBPOINTER(buffer)+offset);
  string raw = string((char *) pBuf, dataFrame._header.payloadLength);
  Variant result;
  uint32_t start=0;

  if (!Variant::DeserializeFromJSON(raw, result, start)) {
    DEBUG ("invalid json");
    return false;
  }

  if (result.HasKey(FILENAME) && result[FILENAME]==V_STRING) {
    _filename=_directory+"/"+(string)result[FILENAME];
    DEBUG ("filename:%s", STR(_filename));
  }
  else {
    DEBUG ("invalid filename");
    return false;
  }
  if (result.HasKey(CHKSUM) && result[CHKSUM]==V_STRING) {
    _chksum=(string)result[CHKSUM]+"\n";
  }
  return true;
}

bool WSServerFileProtocol::SignalInputBinaryFrame(IOBuffer &buffer, WebSocketDataFrame &dataFrame)
{
  uint32_t offset = dataFrame._payloadOffset;
  uint8_t *pBuf = (uint8_t*)(GETIBPOINTER(buffer)+offset);
  if (_filename.empty()) {
    DEBUG ("no image filename specified");
    return false;
  }

  ofstream imgFile;
  imgFile.open(STR(_filename), ios::binary);
  imgFile.write((char*)pBuf, GETAVAILABLEBYTESCOUNT(buffer)-offset);
  imgFile.close();

  string chksum=GetSha1Sum();
  if (chksum.compare(_chksum)!=0) {
    string rmCmd="rm -rf " + _filename;
    DEBUG ("invalid chksum");
    system(STR(rmCmd));
    return false;
  }
  return true;
}
