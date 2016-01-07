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
#include "mediaformats/mp4/atomavc1.h"
#include "mediaformats/mp4/basemp4document.h"
#include "mediaformats/mp4/atomavcc.h"

AtomAVC1::AtomAVC1(BaseMP4Document *pDocument, uint32_t type, uint64_t size, uint64_t start)
: VersionedBoxAtom(pDocument, type, size, start) {
	_pAVCC = NULL;
	_reserved = 0;
	_referenceIndex = 0;
	_qtVideoEncodingVersion = 0;
	_qtVideoEncodingRevisionLevel = 0;
	_qtVideoEncodingVendor = 0;
	_qtVideoTemporalQuality = 0;
	_qtVideoSpatialQuality = 0;
	_videoFramePixelSize = 0;
	_horizontalDpi = 0;
	_verticalDpi = 0;
	_qtVideoDataSize = 0;
	_videoFrameCount = 0;
	_videoEncoderNameLength = 0;
	_videoPixelDepth = 0;
	_qtVideoColorTableId = 0;
}

AtomAVC1::AtomAVC1(BaseMP4Document *pDocument, uint16_t width, uint16_t height, uint16_t frameCount, uint16_t depth) 
: VersionedBoxAtom(pDocument, A_AVC1, 0, 0) {
	_pAVCC = NULL;
	_reserved = 0;
	_referenceIndex = 1;
	_qtVideoEncodingVersion = 0;
	_qtVideoEncodingRevisionLevel = 0;
	_qtVideoEncodingVendor = 0;
	_qtVideoTemporalQuality = 0;
	_qtVideoSpatialQuality = 0;
	_videoFramePixelSize = 0;
	_horizontalDpi = 0x00480000;
	_verticalDpi = 0x00480000;
	_qtVideoDataSize = 0;
	_videoFrameCount = frameCount;
	_videoEncoderNameLength = 0;
	_videoPixelDepth = depth;
	_qtVideoColorTableId = 0xffff;
  _width = width;
  _height = height;
}

AtomAVC1::~AtomAVC1() {
}

bool AtomAVC1::ReadData() {
	if (!ReadUInt16(_reserved)) {
		FATAL("Unable to read _reserved");
		return false;
	}

	if (!ReadUInt16(_referenceIndex)) {
		FATAL("Unable to read _referenceIndex");
		return false;
	}

	if (!ReadUInt16(_qtVideoEncodingVersion)) {
		FATAL("Unable to read _qtVideoEncodingVersion");
		return false;
	}

	if (!ReadUInt16(_qtVideoEncodingRevisionLevel)) {
		FATAL("Unable to read _qtVideoEncodingRevisionLevel");
		return false;
	}

	if (!ReadUInt32(_qtVideoEncodingVendor)) {
		FATAL("Unable to read _qtVideoEncodingVendor");
		return false;
	}

	if (!ReadUInt32(_qtVideoTemporalQuality)) {
		FATAL("Unable to read _qtVideoTemporalQuality");
		return false;
	}

	if (!ReadUInt32(_qtVideoSpatialQuality)) {
		FATAL("Unable to read _qtVideoSpatialQuality");
		return false;
	}

	if (!ReadUInt32(_videoFramePixelSize)) {
		FATAL("Unable to read _videoFramePixelSize");
		return false;
	}

	if (!ReadUInt32(_horizontalDpi)) {
		FATAL("Unable to read _horizontalDpi");
		return false;
	}

	if (!ReadUInt32(_verticalDpi)) {
		FATAL("Unable to read _verticalDpi");
		return false;
	}

	if (!ReadUInt32(_qtVideoDataSize)) {
		FATAL("Unable to read _qtVideoDataSize");
		return false;
	}

	if (!ReadUInt16(_videoFrameCount)) {
		FATAL("Unable to read _videoFrameCount");
		return false;
	}

	if (!ReadUInt8(_videoEncoderNameLength)) {
		FATAL("Unable to read _videoEncoderNameLength");
		return false;
	}

	if (_videoEncoderNameLength < 31)
		_videoEncoderNameLength = 31;

	uint8_t *pTemp = new uint8_t[_videoEncoderNameLength];
	if (!ReadArray(pTemp, _videoEncoderNameLength)) {
		FATAL("Unable to read _videoEncoderNameLength");
		delete[] pTemp;
		return false;
	}
	_videoEncoderName = string((char *) pTemp, _videoEncoderNameLength);
	delete[] pTemp;

	if (!ReadUInt16(_videoPixelDepth)) {
		FATAL("Unable to read _videoPixelDepth");
		return false;
	}

	if (!ReadUInt16(_qtVideoColorTableId)) {
		FATAL("Unable to read _qtVideoColorTableId");
		return false;
	}
	if (_qtVideoColorTableId != 0xffff) {
		FATAL("_qtVideoColorTableId not supported yet");
		return false;
	}

	return true;
}

bool AtomAVC1::AtomCreated(BaseAtom *pAtom) {
	switch (pAtom->GetTypeNumeric()) {
			//TODO: What is the deal with all this order stuff!?
		case A_AVCC:
			_pAVCC = (AtomAVCC *) pAtom;
			return true;
		default:
		{
			FATAL("Invalid atom type: %s", STR(pAtom->GetTypeString()));
			return false;
		}
	}
}

void AtomAVC1::SerializeToBuffer(IOBuffer& data, uint32_t maxFrames) {
  uint32_t start=GETAVAILABLEBYTESCOUNT(data);
  BaseAtom::SerializeToBuffer(data, maxFrames);
  data.ReadFromByte(_version);
  data.ReadFromBuffer(_flags, sizeof(_flags));
 
  data.ReadFromDataType<uint16_t>(endianSwap16(_reserved));
  data.ReadFromDataType<uint16_t>(endianSwap16(_referenceIndex));
  data.ReadFromRepeat(0x00, 2); //pre-defined
  data.ReadFromRepeat(0x00, 2); //reserved
  data.ReadFromRepeat(0x00, 12); //pre-defined
  data.ReadFromDataType<uint16_t>(endianSwap16(_width));
  data.ReadFromDataType<uint16_t>(endianSwap16(_height));
  data.ReadFromDataType<uint32_t>(endianSwap32(_horizontalDpi));
  data.ReadFromDataType<uint32_t>(endianSwap32(_verticalDpi));
  data.ReadFromDataType<uint32_t>(endianSwap32(_qtVideoDataSize));
  data.ReadFromDataType<uint16_t>(endianSwap16(_videoFrameCount));
  data.ReadFromRepeat(0x00, 32); //compressor name
  data.ReadFromDataType<uint16_t>(endianSwap16(_videoPixelDepth));
  data.ReadFromDataType<uint16_t>(endianSwap16(_qtVideoColorTableId));
  _size=GETAVAILABLEBYTESCOUNT(data)-start;
  FOR_VECTOR (_subAtoms, i) {
    _subAtoms[i]->SerializeToBuffer(data, maxFrames);
    _size += _subAtoms[i]->GetSize();
  }
  *(uint32_t*)(GETIBPOINTER(data)+start) = endianSwap32(_size);
}
#endif /* HAS_MEDIA_MP4 */
