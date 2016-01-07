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
#ifndef _WRITEMP4DOCUMENT_H
#define	_WRITEMP4DOCUMENT_H

#include "common.h"
#include "mediaformats/mp4/baseatom.h"
#include "mediaformats/mp4/basemp4document.h"

class File;
class WriteMP4Document
: public BaseMP4Document {

private:
  File *_pFile;
  double _lastTS;
  IOBuffer _data; 
  string _fileName;

public:
	WriteMP4Document(Variant &metadata);
	virtual ~WriteMP4Document();

  virtual bool Open(string name);
  virtual bool SerializeToFile(uint64_t offset, uint32_t maxFrames);
  virtual bool UpdateHeader(AtomUpdateInfo& info, uint64_t duration);
  virtual void UpdateSize();
  virtual bool UpdateMediaData(uint8_t *pData, uint32_t size);
  virtual void Close();
  string FileName() {return _fileName;}

  bool WriteI32(uint32_t data);
  bool SeekTo(uint64_t position);
  bool SeekEnd();
  bool WriteBuffer(uint8_t* pData, uint64_t size);
  void ListAtomSize();
  uint64_t Size();
  bool Flush();
  IOBuffer* GetBuffer();
};

#endif	/* _MP4DOCUMENT_H */


#endif /* HAS_MEDIA_MP4 */
