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
#include "mediaformats/mp4/atomstsc.h"
#include "mediaformats/mp4/basemp4document.h"
#include "mediaformats/mp4/writemp4document.h"

AtomSTSC::AtomSTSC(BaseMP4Document *pDocument, uint32_t type, uint64_t size, uint64_t start)
: VersionedAtom(pDocument, type, size, start) {
}

AtomSTSC::AtomSTSC(BaseMP4Document *pDocument, uint32_t firstChunk, uint32_t sampleDescriptionIndex)
: VersionedAtom(pDocument, A_STSC, 0, 0) {
  _offset=0;

  STSCEntry entry;
  entry.firstChunk = firstChunk;
  entry.samplesPerChunk= 0;
  entry.sampleDescriptionIndex= sampleDescriptionIndex;

  _stscEntries.push_back(entry);
}

AtomSTSC::~AtomSTSC() {
}

vector<uint32_t> AtomSTSC::GetEntries(uint32_t totalChunksCount) {
	if ((_normalizedEntries.size() != 0) || (_stscEntries.size() == 0))
		return _normalizedEntries;

	//1. Expand the table
	vector<uint32_t> samplesPerChunk;
	for (uint32_t i = 0; i < _stscEntries.size() - 1; i++) {
		for (uint32_t j = 0; j < _stscEntries[i + 1].firstChunk - _stscEntries[i].firstChunk; j++) {
			ADD_VECTOR_END(samplesPerChunk, _stscEntries[i].samplesPerChunk);
		}
	}

	uint32_t samplesPerChunkCount = (uint32_t) samplesPerChunk.size();
	for (uint32_t i = 0; i < totalChunksCount - samplesPerChunkCount; i++) {
		ADD_VECTOR_END(samplesPerChunk,
				_stscEntries[_stscEntries.size() - 1].samplesPerChunk);
	}

	//2. build the final result based on the expanded table
	samplesPerChunkCount = (uint32_t) samplesPerChunk.size();
	for (uint32_t i = 0; i < samplesPerChunkCount; i++) {
		for (uint32_t j = 0; j < samplesPerChunk[i]; j++) {
			ADD_VECTOR_END(_normalizedEntries, i);
		}
	}

	return _normalizedEntries;
}

bool AtomSTSC::ReadData() {
	uint32_t count;
	if (!ReadUInt32(count)) {
		FATAL("Unable to read count");
		return false;
	}

	if (count == 0)
		return true;

	for (uint32_t i = 0; i < count; i++) {
		STSCEntry entry;

		if (!ReadUInt32(entry.firstChunk)) {
			FATAL("Unable to read first chunk");
			return false;
		}

		if (!ReadUInt32(entry.samplesPerChunk)) {
			FATAL("Unable to read first samples per chunk");
			return false;
		}

		if (!ReadUInt32(entry.sampleDescriptionIndex)) {
			FATAL("Unable to read first sample description index");
			return false;
		}

		ADD_VECTOR_END(_stscEntries, entry);
	}

	return true;
}

void AtomSTSC::SerializeToBuffer(IOBuffer& data, uint32_t maxFrames) {
  uint32_t start=GETAVAILABLEBYTESCOUNT(data);
  VersionedAtom::SerializeToBuffer(data, maxFrames);
  data.ReadFromDataType<uint32_t>(endianSwap32(_stscEntries.size()));
  FOR_VECTOR(_stscEntries, i) {
    data.ReadFromDataType<uint32_t>(endianSwap32(_stscEntries[i].firstChunk));
    _offset = GETAVAILABLEBYTESCOUNT(data);
    data.ReadFromDataType<uint32_t>(endianSwap32(_stscEntries[i].samplesPerChunk));
    data.ReadFromDataType<uint32_t>(endianSwap32(_stscEntries[i].sampleDescriptionIndex));
  }
  _size=GETAVAILABLEBYTESCOUNT(data)-start;
  *(uint32_t*)(GETIBPOINTER(data)+start) = endianSwap32(_size);
}

void AtomSTSC::UpdateToFile(AtomUpdateInfo &info, uint64_t duration) {
  _stscEntries[0].samplesPerChunk += 1;
}

uint64_t AtomSTSC::UpdateAtomSizeToFile() {
  BaseAtom::UpdateAtomSizeToFile();
  WriteMP4Document *pWriteDoc=reinterpret_cast<WriteMP4Document*>(_pDoc);
  pWriteDoc->SeekTo(_offset);
  pWriteDoc->WriteI32(_stscEntries[0].samplesPerChunk);
  return _size;
}

#endif /* HAS_MEDIA_MP4 */
