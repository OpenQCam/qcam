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
#ifndef _ATOMMDHD_H
#define _ATOMMDHD_H

#include "mediaformats/mp4/versionedatom.h"

class AtomMDHD
: public VersionedAtom {
private:
	uint64_t _creationTime;
	uint64_t _modificationTime;
	uint32_t _timeScale;
	uint64_t _duration;
	uint16_t _language;
	uint16_t _quality;

  double _firstTS;
  uint64_t _offset;
public:
	AtomMDHD(BaseMP4Document *pDocument, uint32_t type, uint64_t size, uint64_t start);
	AtomMDHD(BaseMP4Document *pDocument, uint64_t creationTime, uint32_t timeScale);
	virtual ~AtomMDHD();

	uint32_t GetTimeScale();
  void virtual SerializeToBuffer(IOBuffer& data, uint32_t maxFrames);
  void virtual UpdateToFile(AtomUpdateInfo &info, uint64_t duration); 
  uint64_t virtual UpdateAtomSizeToFile();
protected:
	virtual bool ReadData();
private:
	bool ReadDataVersion0();
	bool ReadDataVersion1();
};

#endif	/* _ATOMMDHD_H */


#endif /* HAS_MEDIA_MP4 */
