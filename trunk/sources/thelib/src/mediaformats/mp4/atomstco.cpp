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
#include "mediaformats/mp4/atomstco.h"
#include "mediaformats/mp4/writemp4document.h"

AtomSTCO::AtomSTCO(BaseMP4Document *pDocument, uint32_t type, uint64_t size, uint64_t start)
: VersionedAtom(pDocument, type, size, start) {

}

AtomSTCO::~AtomSTCO() {
}

vector<uint64_t> AtomSTCO::GetEntries() {
	return _entries;
}

void AtomSTCO::AddEntries(uint64_t value) {
	_entries.push_back(value);
}

bool AtomSTCO::ReadData() {
	uint32_t count;

	if (!ReadUInt32(count)) {
		FATAL("Unable to read count");
		return false;
	}

	for (uint32_t i = 0; i < count; i++) {
		uint32_t offset;
		if (!ReadUInt32(offset)) {
			FATAL("Unable to read offset");
			return false;
		}
		ADD_VECTOR_END(_entries, offset);
	}
	return true;
}


void AtomSTCO::SerializeToBuffer(IOBuffer& data, uint32_t maxFrames) {
  uint32_t start=GETAVAILABLEBYTESCOUNT(data);
  VersionedAtom::SerializeToBuffer(data, maxFrames);
  data.ReadFromDataType<uint32_t>(endianSwap32(_entries.size()));

  _offset=GETAVAILABLEBYTESCOUNT(data);
  FOR_VECTOR (_entries, i) {
    data.ReadFromDataType<uint32_t>(endianSwap32(_entries[i]));
  }
  _size=GETAVAILABLEBYTESCOUNT(data)-start;
  *(uint32_t*)(GETIBPOINTER(data)+start) = endianSwap32(_size);
}

bool AtomSTCO::UpdateChunkOffset(uint32_t chunkOffset) {
  bool ret=true;
  WriteMP4Document *pWriteDoc=reinterpret_cast<WriteMP4Document*>(_pDoc);
  ret=pWriteDoc->SeekTo(_offset);
  pWriteDoc->WriteI32(chunkOffset);
  return ret;
}
#endif /* HAS_MEDIA_MP4 */
