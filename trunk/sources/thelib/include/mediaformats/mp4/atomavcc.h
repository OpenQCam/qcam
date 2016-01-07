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
#ifndef _ATOMAVCC_H
#define _ATOMAVCC_H

#include "mediaformats/mp4/baseatom.h"

typedef struct _AVCCParameter {
	uint16_t size;
	uint8_t *pData;
} AVCCParameter;

class AtomAVCC
: public BaseAtom {
private:
	uint8_t _configurationVersion;
	uint8_t _profile;
	uint8_t _profileCompatibility;
	uint8_t _level;
	uint8_t _naluLengthSize;
	vector<AVCCParameter> _seqParameters;
	vector<AVCCParameter> _picParameters;
public:
	AtomAVCC(BaseMP4Document *pDocument, uint32_t type, uint64_t size, uint64_t start);
  AtomAVCC(BaseMP4Document *pDocument, uint8_t profile, uint8_t profilecomp, uint8_t level);
	virtual ~AtomAVCC();
	uint64_t GetExtraDataStart();
	uint64_t GetExtraDataLength();
	virtual bool Read();
	virtual string Hierarchy(uint32_t indent);

  virtual void SetSPSParam(uint16_t SPSsize, uint8_t *SPSdata);
  virtual void SetPPSParam(uint16_t PPSsize, uint8_t *PPSdata);
  virtual void SerializeToBuffer(IOBuffer& data, uint32_t maxFrames);
};

#endif	/* _ATOMAVCC_H */


#endif /* HAS_MEDIA_MP4 */
