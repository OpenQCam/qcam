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
#include "mediaformats/mp4/versionedboxatom.h"

VersionedBoxAtom::VersionedBoxAtom(BaseMP4Document *pDocument, uint32_t type,
		uint64_t size, uint64_t start)
: BoxAtom(pDocument, type, size, start) {
	_version = 0;
	memset(_flags, 0, 3);
}

VersionedBoxAtom::~VersionedBoxAtom() {
}

bool VersionedBoxAtom::Read() {
	if (!ReadUInt8(_version)) {
		FATAL("Unable to read version");
		return false;
	}

	if (!ReadArray(_flags, 3)) {
		FATAL("Unable to read flags");
		return false;
	}

	if (!ReadData()) {
		FATAL("Unable to read data");
		return false;
	}

	return BoxAtom::Read();
}

void VersionedBoxAtom::SerializeToBuffer(IOBuffer& data, uint32_t maxFrames) {
  uint32_t start=GETAVAILABLEBYTESCOUNT(data);
  BaseAtom::SerializeToBuffer(data, maxFrames);
  data.ReadFromByte(_version);
  data.ReadFromBuffer(_flags, sizeof(_flags));
  _size=GETAVAILABLEBYTESCOUNT(data)-start;
  FOR_VECTOR (_subAtoms, i) {
    _subAtoms[i]->SerializeToBuffer(data, maxFrames);
    DEBUG ("%s[%s] size:%llu", STR(GetTypeString()), STR(_subAtoms[i]->GetTypeString()), _subAtoms[i]->GetSize());
    _size += _subAtoms[i]->GetSize();
  }
  *(uint32_t*)(GETIBPOINTER(data)+start) = endianSwap32(_size);
  DEBUG ("%s size:%llu", STR(GetTypeString()), _size);
}


#endif /* HAS_MEDIA_MP4 */
