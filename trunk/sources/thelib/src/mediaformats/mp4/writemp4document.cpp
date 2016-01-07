/*
 *  Copyright (c) 2010,
 *  Gavriloaie Eugen-Andrei (shiretu@gmail.com)
 *
 *  This file is part of crtmpserver.
 *  crtmpserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  crtmpserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with crtmpserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAS_MEDIA_MP4
#include "mediaformats/mp4/writemp4document.h"
#include "mediaformats/mp4/atomavc1.h"
#include "mediaformats/mp4/atomavcc.h"
#include "mediaformats/mp4/atomco64.h"
#include "mediaformats/mp4/atomctts.h"
#include "mediaformats/mp4/atomdata.h"
#include "mediaformats/mp4/atomdinf.h"
#include "mediaformats/mp4/atomdref.h"
#include "mediaformats/mp4/atomesds.h"
#include "mediaformats/mp4/atomftyp.h"
#include "mediaformats/mp4/atomhdlr.h"
#include "mediaformats/mp4/atomilst.h"
#include "mediaformats/mp4/atommdhd.h"
#include "mediaformats/mp4/atommdia.h"
#include "mediaformats/mp4/atommeta.h"
#include "mediaformats/mp4/atommetafield.h"
#include "mediaformats/mp4/atommfhd.h"
#include "mediaformats/mp4/atomminf.h"
#include "mediaformats/mp4/atommoof.h"
#include "mediaformats/mp4/atommoov.h"
#include "mediaformats/mp4/atommp4a.h"
#include "mediaformats/mp4/atommvex.h"
#include "mediaformats/mp4/atommvhd.h"
#include "mediaformats/mp4/atomnull.h"
#include "mediaformats/mp4/atomsmhd.h"
#include "mediaformats/mp4/atomstbl.h"
#include "mediaformats/mp4/atomstco.h"
#include "mediaformats/mp4/atomstsc.h"
#include "mediaformats/mp4/atomstsd.h"
#include "mediaformats/mp4/atomstss.h"
#include "mediaformats/mp4/atomstsz.h"
#include "mediaformats/mp4/atomstts.h"
#include "mediaformats/mp4/atomtfhd.h"
#include "mediaformats/mp4/atomtkhd.h"
#include "mediaformats/mp4/atomtraf.h"
#include "mediaformats/mp4/atomtrak.h"
#include "mediaformats/mp4/atomtrex.h"
#include "mediaformats/mp4/atomtrun.h"
#include "mediaformats/mp4/atomudta.h"
#include "mediaformats/mp4/atomurl.h"
#include "mediaformats/mp4/atomvmhd.h"
#include "mediaformats/mp4/atomwave.h"
#include "mediaformats/mp4/baseatom.h"
#include "mediaformats/mp4/ignoredatom.h"

WriteMP4Document::WriteMP4Document(Variant &metadata)
: BaseMP4Document(metadata),
  _pFile(NULL),
  _lastTS(0)
{
}

WriteMP4Document::~WriteMP4Document() 
{
  if (_pFile) {
    if (_pFile->IsOpen()) {
      _pFile->Close();
    }
    delete _pFile;
  }
}

bool WriteMP4Document::Open(string path)
{
  if (_pFile) {
    FATAL ("file opened");
    return false;
  }

  _pFile = new File();
  bool ret =_pFile->Initialize(path, FILE_OPEN_MODE_TRUNCATE);
  _fileName = path;
  return ret;
}

bool WriteMP4Document::SerializeToFile(uint64_t offset, uint32_t maxFrames)
{
  bool ret=true;
  if (_pFile) {
    FOR_VECTOR(_topAtoms,i) {
      _topAtoms[i]->SerializeToBuffer(_data, maxFrames);
    }
    ret=_pFile->WriteBuffer(GETIBPOINTER(_data), GETAVAILABLEBYTESCOUNT(_data));
  }
  return ret;
}

void WriteMP4Document::ListAtomSize()
{
  FOR_VECTOR(_topAtoms, j) {
    _topAtoms[j]->ListSize("---");
  }
}
void WriteMP4Document::UpdateSize() {
  FOR_VECTOR(_allAtoms, j) {
    _allAtoms[j]->UpdateAtomSizeToFile();
  }
}

bool WriteMP4Document::UpdateHeader(AtomUpdateInfo& videoInfo, uint64_t duration)
{
  if (_pFile) {
    FOR_VECTOR(_allAtoms,j) {
      _allAtoms[j]->UpdateToFile(videoInfo, duration);
    }
  }
  else {
    WARN ("_pFile empty");
    return false;
  }
  return true;
}

bool WriteMP4Document::UpdateMediaData(uint8_t *pData, uint32_t size)
{
  bool ret=true;
  if (_pFile) {
    FOR_VECTOR(_topAtoms,i) {
      if (_topAtoms[i]->GetTypeNumeric()==A_MDAT) {
        ret=_pFile->SeekEnd();
        if (!ret) return ret;
        ret=_pFile->WriteBuffer(pData, size);
        if (!ret) return ret;
        uint32_t fileSize= _pFile->Size();
        ret=_pFile->SeekBegin();
        if (!ret) return ret;
        ret=_pFile->WriteI32(fileSize, true);
        if (!ret) return ret;
        _topAtoms[i]->SignalSizeChange(size);
      }
    }
  }
  return ret;
}

void WriteMP4Document::Close() {
  if (_pFile) {
    //_pFile->Flush();
    _pFile->Close();
  }
}

bool WriteMP4Document::WriteI32(uint32_t data) {
  if (_pFile)
    return _pFile->WriteI32(data, true);
  return false;
}

bool WriteMP4Document::WriteBuffer(uint8_t* pData, uint64_t size) {
  if (_pFile) 
    return _pFile->WriteBuffer(pData, size);
  return false;
}

bool WriteMP4Document::SeekTo(uint64_t position) {
  if (_pFile) 
    return _pFile->SeekTo(position);
  return false;
}


bool WriteMP4Document::SeekEnd() {
  if (_pFile) 
    return _pFile->SeekEnd();
  return false;
}

bool WriteMP4Document::Flush() {
  if (_pFile)
    return _pFile->Flush();
  return false;
}

uint64_t WriteMP4Document::Size() {
  if (_pFile)
    return _pFile->Size();
  return 0;
}

/* FIXME(recardo) ugly interface */
IOBuffer* WriteMP4Document::GetBuffer() {
  return &_data;
}

#endif /* HAS_MEDIA_MP4 */
