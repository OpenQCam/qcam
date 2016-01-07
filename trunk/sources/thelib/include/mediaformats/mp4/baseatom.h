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
#ifndef _BASEATOM_H
#define	_BASEATOM_H

#include "common.h"

string U32TOS(uint32_t type);

typedef struct {
  uint32_t frameIndex;
  uint32_t isKeyFrame;
  uint32_t size;
  double timeStamp;
} AtomUpdateInfo;

class BaseMP4Document;

class BaseAtom {
protected:
	uint64_t _start;
	uint64_t _size;
	uint32_t _type;

	BaseMP4Document *_pDoc;
	BaseAtom *_pParent;
public:
	BaseAtom(BaseMP4Document *pDocument, uint32_t type, uint64_t size, uint64_t start);
	virtual ~BaseAtom();

	uint64_t GetStart();
	uint64_t GetSize();
	string GetTypeString();
	uint32_t GetTypeNumeric();

	BaseMP4Document * GetDoc();

	virtual bool Read() = 0;

	virtual operator string();
	virtual bool IsIgnored();

	virtual BaseAtom * GetPath(vector<uint32_t> path);

	virtual string Hierarchy(uint32_t indent) = 0;
  virtual void SerializeToBuffer(IOBuffer &data, uint32_t maxFrames) {
    uint32_t start=GETAVAILABLEBYTESCOUNT(data);
    _start= start;
    data.ReadFromDataType<uint32_t>(endianSwap32(_size));
    data.ReadFromDataType<uint32_t>(endianSwap32(_type));
    _size=GETAVAILABLEBYTESCOUNT(data)-start;
  }

  virtual void UpdateToFile(AtomUpdateInfo &info, uint64_t duration);
  virtual uint64_t UpdateAtomSizeToFile();
	BaseAtom *GetParentAtom();
	virtual void SetParentAtom(BaseAtom *pParent);
  virtual void SetChildAtom(BaseAtom *pChild) {NYI;}
  virtual void ListSize(string  prefix) {
    DEBUG ("%s%s[%llu]", STR(prefix), STR(GetTypeString()), _size);
  }
  virtual void SignalSizeChange(int32_t val) {
    _size+=val;
    if (_pParent) 
      _pParent->SignalSizeChange(val);
  }


protected:
	bool SkipRead(bool issueWarn = true);
	uint64_t CurrentPosition();
	bool CheckBounds(uint64_t size);
	bool ReadArray(uint8_t *pBuffer, uint64_t length);
	bool ReadUInt8(uint8_t &val);
	bool ReadUInt16(uint16_t &val, bool networkOrder = true);
	bool ReadInt16(int16_t &val, bool networkOrder = true);
	bool ReadUInt24(uint32_t &val, bool networkOrder = true);
	bool ReadUInt32(uint32_t &val, bool networkOrder = true);
	bool ReadInt32(int32_t &val, bool networkOrder = true);
	bool ReadUInt64(uint64_t &val, bool networkOrder = true);
	bool ReadInt64(int64_t &val, bool networkOrder = true);
	bool SkipBytes(uint64_t count);
	bool ReadString(string &val, uint64_t size);
};

#endif	/* _BASEATOM_H */


#endif /* HAS_MEDIA_MP4 */
