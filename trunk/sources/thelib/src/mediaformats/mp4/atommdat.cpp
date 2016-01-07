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
#include "mediaformats/mp4/atommdat.h"
#include "mediaformats/mp4/basemp4document.h"

AtomMDAT::AtomMDAT(BaseMP4Document *pDocument, uint32_t type, uint64_t size, uint64_t start)
: BaseAtom(pDocument, type, size, start) {
}

AtomMDAT::~AtomMDAT() {
}

bool AtomMDAT::Read() {
  NYI;
  return true;
}

string AtomMDAT::Hierarchy(uint32_t indent) {
	return string(4 * indent, ' ') + GetTypeString();
}

void AtomMDAT::SerializeToBuffer(IOBuffer& data, uint32_t maxFrames) {
  uint32_t start=GETAVAILABLEBYTESCOUNT(data);
  BaseAtom::SerializeToBuffer(data, maxFrames);

  _size=GETAVAILABLEBYTESCOUNT(data)-start;
  *(uint32_t*)(GETIBPOINTER(data)+start) = endianSwap32(_size);
}
#endif /* HAS_MEDIA_MP4 */
