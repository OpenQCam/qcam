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
#include "mediaformats/mp4/atomftyp.h"
#include "mediaformats/mp4/basemp4document.h"

AtomFTYP::AtomFTYP(BaseMP4Document *pDocument, uint32_t type, uint64_t size, uint64_t start)
: BaseAtom(pDocument, type, size, start) {
	_majorBrand = 0;
	_minorVersion = 0;
}


AtomFTYP::AtomFTYP(BaseMP4Document *pDocument, uint32_t majorBrand, uint32_t minorVersion, vector<uint32_t> brands)
: BaseAtom(pDocument, A_FTYP, 0, 0),
  _majorBrand(majorBrand),
  _minorVersion(minorVersion),
  _compatibleBrands(brands)
{
}

AtomFTYP::~AtomFTYP() {
}

bool AtomFTYP::Read() {
	if (!ReadUInt32(_majorBrand, false)) {
		FATAL("Unable to read major brand");
		return false;
	}

	if (!ReadUInt32(_minorVersion, false)) {
		FATAL("Unable to read minor version");
		return false;
	}

	for (uint64_t i = 16; i < _size; i += 4) {
		uint32_t val = 0;
		if (!ReadUInt32(val, false)) {
			FATAL("Unable to read compatible brand");
			return false;
		}
		ADD_VECTOR_END(_compatibleBrands, val);
	}
	return true;
}

string AtomFTYP::Hierarchy(uint32_t indent) {
	return string(4 * indent, ' ') + GetTypeString();
}

void AtomFTYP::SerializeToBuffer(IOBuffer& data, uint32_t maxFrames) {
  uint32_t start=GETAVAILABLEBYTESCOUNT(data);
  BaseAtom::SerializeToBuffer(data, maxFrames);
  data.ReadFromDataType<uint32_t>(endianSwap32(_majorBrand));
  data.ReadFromDataType<uint32_t>(endianSwap32(_minorVersion));
  FOR_VECTOR(_compatibleBrands, i) {
    data.ReadFromDataType<uint32_t>(endianSwap32(_compatibleBrands[i]));
  }

  _size = GETAVAILABLEBYTESCOUNT(data)-start;
  *(uint32_t*)(GETIBPOINTER(data)+start) = endianSwap32(_size);
}

#endif /* HAS_MEDIA_MP4 */
