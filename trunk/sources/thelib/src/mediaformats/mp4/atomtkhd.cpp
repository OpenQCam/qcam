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
#include "mediaformats/mp4/atomtkhd.h"
#include "mediaformats/mp4/basemp4document.h"
#include "mediaformats/mp4/writemp4document.h"

AtomTKHD::AtomTKHD(BaseMP4Document *pDocument, uint32_t type, uint64_t size, uint64_t start)
: VersionedAtom(pDocument, type, size, start) {
	_creationTime = 0;
	_modificationTime = 0;
	_trackId = 0;
	memset(_reserved1, 0, 4);
	_duration = 0;
	memset(_reserved2, 0, 8);
	_layer = 0;
	_alternateGroup = 0;
	_volume = 0;
	memset(_reserved3, 0, 2);
	memset(_matrixStructure, 0, 36);
	_trackWidth = 0;
	_trackHeight = 0;
}

AtomTKHD::AtomTKHD(BaseMP4Document *pDocument, uint32_t creationTime, uint32_t trackId, uint32_t trackWidth, uint32_t trackHeight)
: VersionedAtom(pDocument, A_TKHD, 0, 0) {
  uint8_t matrixStructure[36] = {
    0x00, 0x01, 0x00, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x00, 0x00, 0x00, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x00, 0x00, 0x00, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x0, 0x0, 0x0,
  };
  _creationTime = _modificationTime = creationTime;
  _trackId = trackId;
	memset(_reserved1, 0, 4);
	_duration = 0;
  _firstTS=0;
	memset(_reserved2, 0, 8);
	_layer = 0;
	_alternateGroup = 0;
	_volume = 0;
	memset(_reserved3, 0, 2);
	memcpy(_matrixStructure, matrixStructure, sizeof(matrixStructure));
	_trackWidth = trackWidth;
	_trackHeight = trackHeight;

}

AtomTKHD::~AtomTKHD() {
}

uint32_t AtomTKHD::GetTrackId() {
	return _trackId;
}

uint32_t AtomTKHD::GetWidth() {
	return _trackWidth >> 16;
}

uint32_t AtomTKHD::GetHeight() {
	return _trackHeight >> 16;
}

bool AtomTKHD::ReadData() {
	if (!ReadUInt32(_creationTime)) {
		FATAL("Unable to read creation time");
		return false;
	}

	if (!ReadUInt32(_modificationTime)) {
		FATAL("Unable to read modification time");
		return false;
	}

	if (!ReadUInt32(_trackId)) {
		FATAL("Unable to read track id");
		return false;
	}

	if (!ReadArray(_reserved1, 4)) {
		FATAL("Unable to read reserved 1");
		return false;
	}

	if (!ReadUInt32(_duration)) {
		FATAL("Unable to read duration");
		return false;
	}

	if (!ReadArray(_reserved2, 8)) {
		FATAL("Unable to read reserved 2");
		return false;
	}

	if (!ReadUInt16(_layer)) {
		FATAL("Unable to read layer");
		return false;
	}

	if (!ReadUInt16(_alternateGroup)) {
		FATAL("Unable to read alternate group");
		return false;
	}

	if (!ReadUInt16(_volume)) {
		FATAL("Unable to read volume");
		return false;
	}

	if (!ReadArray(_reserved3, 2)) {
		FATAL("Unable to read reserved 3");
		return false;
	}

	if (!ReadArray(_matrixStructure, 36)) {
		FATAL("Unable to read matrix structure");
		return false;
	}

	if (!ReadUInt32(_trackWidth)) {
		FATAL("Unable to read track width");
		return false;
	}

	if (!ReadUInt32(_trackHeight)) {
		FATAL("Unable to read track height");
		return false;
	}

	return true;
}

void AtomTKHD::SerializeToBuffer(IOBuffer& data, uint32_t maxFrames) {
  uint32_t start=GETAVAILABLEBYTESCOUNT(data);
  _flags[2]=0x1;
  VersionedAtom::SerializeToBuffer(data, maxFrames);
  if (_version == 1) {
    NYI;
  }
  else { //version==0
    data.ReadFromDataType<uint32_t>(endianSwap32(_creationTime));
    data.ReadFromDataType<uint32_t>(endianSwap32(_modificationTime));
    data.ReadFromDataType<uint32_t>(endianSwap32(_trackId));
    data.ReadFromBuffer(_reserved1, sizeof(_reserved1));
    _offset = GETAVAILABLEBYTESCOUNT(data);
    data.ReadFromDataType<uint32_t>(endianSwap32(_duration));
  }

  data.ReadFromBuffer(_reserved2, sizeof(_reserved2));
  data.ReadFromDataType<uint16_t>(endianSwap16(_layer));
  data.ReadFromDataType<uint16_t>(endianSwap16(_alternateGroup));
  data.ReadFromDataType<uint16_t>(endianSwap16(_volume));
  data.ReadFromBuffer(_reserved3, sizeof(_reserved3));
  data.ReadFromBuffer(_matrixStructure, sizeof(_matrixStructure));
  data.ReadFromDataType<uint32_t>(endianSwap32(_trackWidth));
  data.ReadFromDataType<uint32_t>(endianSwap32(_trackHeight));
  _size=GETAVAILABLEBYTESCOUNT(data)-start;
  *(uint32_t*)(GETIBPOINTER(data)+start) = endianSwap32(_size);
}

void AtomTKHD::UpdateToFile(AtomUpdateInfo &info, uint64_t duration) {
  if (!_firstTS)
    _firstTS=info.timeStamp;

  _duration = (uint64_t)(info.timeStamp-_firstTS)+duration;
}

uint64_t AtomTKHD::UpdateAtomSizeToFile() {
  BaseAtom::UpdateAtomSizeToFile();

  WriteMP4Document *pWriteDoc=reinterpret_cast<WriteMP4Document*>(_pDoc);
  pWriteDoc->SeekTo(_offset);
  pWriteDoc->WriteI32(_duration);
  return _size;
}

#endif /* HAS_MEDIA_MP4 */
