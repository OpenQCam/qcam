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
#include "mediaformats/mp4/atommvhd.h"
#include "mediaformats/mp4/basemp4document.h"
#include "mediaformats/mp4/writemp4document.h"

AtomMVHD::AtomMVHD(BaseMP4Document *pDocument, uint32_t type, uint64_t size, uint64_t start)
: VersionedAtom(pDocument, type, size, start) {
	_creationTime = 0;
	_modificationTime = 0;
	_timeScale = 0;
	_duration = 0;
	_preferredRate = 0;
	_preferredVolume = 0;
	memset(_reserved, 0, 10);
	memset(_matrixStructure, 0, 36);
	_previewTime = 0;
	_previewDuration = 0;
	_posterTime = 0;
	_selectionTime = 0;
	_selectionDuration = 0;
	_currentTime = 0;
	_nextTrakId = 0;
}

AtomMVHD::AtomMVHD(BaseMP4Document *pDocument, uint64_t creationTime, uint32_t timeScale, uint32_t nextTrakId)
: VersionedAtom(pDocument, A_MVHD, 0, 0) {
	uint32_t matrixStructure[9] = {0x10000, 0, 0,
                                 0, 0x10000, 0,
                                 0, 0, 0x40000000};

	_creationTime = _modificationTime = creationTime;
	_timeScale = timeScale;
	_duration = 0;
  _firstTS = 0;
	_preferredRate = 0x00010000;
	_preferredVolume = 0x0100;
	memset(_reserved, 0, 10);
  memcpy(_matrixStructure, matrixStructure, sizeof(matrixStructure));
	_previewTime = 0;
	_previewDuration = 0;
	_posterTime = 0;
	_selectionTime = 0;
	_selectionDuration = 0;
	_currentTime = 0;
	_nextTrakId = nextTrakId;
}

AtomMVHD::~AtomMVHD() {
}

bool AtomMVHD::ReadData() {
	if (!ReadUInt32(_creationTime)) {
		FATAL("Unable to read creation time");
		return false;
	}

	if (!ReadUInt32(_modificationTime)) {
		FATAL("Unable to read modification time");
		return false;
	}

	if (!ReadUInt32(_timeScale)) {
		FATAL("Unable to read time scale");
		return false;
	}

	if (!ReadUInt32(_duration)) {
		FATAL("Unable to read duration");
		return false;
	}

	if (!ReadUInt32(_preferredRate)) {
		FATAL("Unable to read preferred rate");
		return false;
	}

	if (!ReadUInt16(_preferredVolume)) {
		FATAL("Unable to read preferred volume");
		return false;
	}

	if (!ReadArray(_reserved, 10)) {
		FATAL("Unable to read reserved");
		return false;
	}

	if (!ReadArray((uint8_t *) _matrixStructure, 36)) {
		FATAL("Unable to read matrix structure");
		return false;
	}

	if (!ReadUInt32(_previewTime)) {
		FATAL("Unable to read preview time");
		return false;
	}

	if (!ReadUInt32(_previewDuration)) {
		FATAL("Unable to read preview duration");
		return false;
	}

	if (!ReadUInt32(_posterTime)) {
		FATAL("Unable to read poster time");
		return false;
	}

	if (!ReadUInt32(_selectionTime)) {
		FATAL("Unable to read selection time");
		return false;
	}

	if (!ReadUInt32(_selectionDuration)) {
		FATAL("Unable to read selection duration");
		return false;
	}

	if (!ReadUInt32(_currentTime)) {
		FATAL("Unable to read current time");
		return false;
	}

	if (!ReadUInt32(_nextTrakId)) {
		FATAL("Unable to read next track ID");
		return false;
	}

	return true;
}

void AtomMVHD::SerializeToBuffer(IOBuffer& data, uint32_t maxFrames) {
  uint32_t start=GETAVAILABLEBYTESCOUNT(data);
  VersionedAtom::SerializeToBuffer(data, maxFrames);
  data.ReadFromDataType<uint32_t>(endianSwap32(_creationTime));
  data.ReadFromDataType<uint32_t>(endianSwap32(_modificationTime));
  data.ReadFromDataType<uint32_t>(endianSwap32(_timeScale));
  _offset=GETAVAILABLEBYTESCOUNT(data);
  data.ReadFromDataType<uint32_t>(endianSwap32(_duration));
  data.ReadFromDataType<uint32_t>(endianSwap32(_preferredRate));
  data.ReadFromDataType<uint16_t>(endianSwap16(_preferredVolume));
  data.ReadFromBuffer(_reserved, sizeof(_reserved));
  for (uint32_t i=0; i<9; i++) 
    data.ReadFromDataType<uint32_t>(endianSwap32(_matrixStructure[i]));
  for (uint32_t i=0; i<6; i++) {
    data.ReadFromRepeat(0x00, 4);
  }
  data.ReadFromDataType<uint32_t>(endianSwap32(_nextTrakId));
  _size=GETAVAILABLEBYTESCOUNT(data)-start;

  *(uint32_t*)(GETIBPOINTER(data)+start) = endianSwap32(_size);
}

void AtomMVHD::UpdateToFile(AtomUpdateInfo &info, uint64_t duration) {
  if (!_firstTS) {
    _firstTS=info.timeStamp;
  }
  _duration = (uint64_t)(info.timeStamp-_firstTS)+duration;
}

uint64_t AtomMVHD::UpdateAtomSizeToFile() {
  BaseAtom::UpdateAtomSizeToFile();

  WriteMP4Document *pWriteDoc=reinterpret_cast<WriteMP4Document*>(_pDoc);
  pWriteDoc->SeekTo(_offset);
  pWriteDoc->WriteI32(_duration);
  return _size;
}

#endif /* HAS_MEDIA_MP4 */
