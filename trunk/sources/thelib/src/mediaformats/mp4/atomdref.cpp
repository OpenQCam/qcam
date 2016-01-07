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
#include "mediaformats/mp4/atomdref.h"
#include "mediaformats/mp4/basemp4document.h"

AtomDREF::AtomDREF(BaseMP4Document *pDocument, uint32_t type, uint64_t size, uint64_t start)
: VersionedBoxAtom(pDocument, type, size, start) {
}

AtomDREF::~AtomDREF() {
}

bool AtomDREF::ReadData() {
	uint32_t count;
	if (!ReadUInt32(count)) {
		FATAL("Unable to read count");
		return false;
	}
	return true;
}
void AtomDREF::SetChildAtom(BaseAtom *pChild) {
  BoxAtom::SetChildAtom(pChild);
  AtomCreated(pChild);
}

bool AtomDREF::AtomCreated(BaseAtom *pAtom) {
	switch (pAtom->GetTypeNumeric()) {
		case A_URL:
			ADD_VECTOR_END(_urls, (AtomURL *) pAtom);
			return true;
		default:
		{
			FATAL("Invalid atom type: %s", STR(pAtom->GetTypeString()));
			return false;
		}
	}
}

void AtomDREF::SerializeToBuffer(IOBuffer& data, uint32_t maxFrames) {
  uint32_t start=GETAVAILABLEBYTESCOUNT(data);
  BaseAtom::SerializeToBuffer(data, maxFrames);
  data.ReadFromByte(_version);
  data.ReadFromBuffer(_flags, sizeof(_flags));
  data.ReadFromDataType<uint32_t>(endianSwap32(_urls.size()));
  _size=GETAVAILABLEBYTESCOUNT(data)-start;

  FOR_VECTOR(_subAtoms, i) {
    _subAtoms[i]->SerializeToBuffer(data, maxFrames);
    _size += _subAtoms[i]->GetSize();
  }
  *(uint32_t*)(GETIBPOINTER(data)+start) = endianSwap32(_size);
}
#endif /* HAS_MEDIA_MP4 */
