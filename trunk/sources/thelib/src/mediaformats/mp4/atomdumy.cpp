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
#include "mediaformats/mp4/atomdumy.h"
#include "mediaformats/mp4/writemp4document.h"

AtomDUMY::AtomDUMY(BaseMP4Document *pDocument, uint32_t bytesPerFrame, uint32_t dumyType)
: BaseAtom(pDocument, A_DUMY, 0, 0)
  ,_bytesPerFrame(bytesPerFrame)
  ,_dumyType(dumyType) {
}

AtomDUMY::~AtomDUMY() {
}

bool AtomDUMY::Read() {
	return true;
}

string AtomDUMY::Hierarchy(uint32_t indent) {
	return string(4 * indent, ' ') + "dumy";
}


void AtomDUMY::SerializeToBuffer(IOBuffer &data, uint32_t maxFrames) {
  uint32_t start=GETAVAILABLEBYTESCOUNT(data);
  uint32_t dummyDataSize= maxFrames*_bytesPerFrame;
  BaseAtom::SerializeToBuffer(data, maxFrames);
  data.ReadFromRepeat(0x00, dummyDataSize);
  _size+=dummyDataSize;
  *(uint32_t*)(GETIBPOINTER(data)+start) = endianSwap32(_size);
}

void AtomDUMY::SignalSizeChange(int32_t val) {
  BaseAtom::SignalSizeChange(val);
  _start-=(val);
}

uint64_t AtomDUMY::UpdateAtomSizeToFile() {
  WriteMP4Document *pWriteDoc = reinterpret_cast<WriteMP4Document*>(_pDoc);
  BaseAtom::UpdateAtomSizeToFile();
  pWriteDoc->WriteI32(A_DUMY);
  return _size;
}

#endif /* HAS_MEDIA_MP4 */
