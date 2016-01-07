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
#ifndef _ATOMDUMY_H
#define _ATOMDUMY_H

#include "mediaformats/mp4/baseatom.h"
#include "mediaformats/mp4/basemp4document.h"

class AtomDUMY
: public BaseAtom {
public:
	AtomDUMY(BaseMP4Document *pDocument, uint32_t bytesPerFrame, uint32_t linkType);
	virtual ~AtomDUMY();

	virtual bool Read();
	string Hierarchy(uint32_t indent);
  virtual void SerializeToBuffer(IOBuffer &data, uint32_t maxFrames);
  virtual void SignalSizeChange(int32_t val);
  virtual uint64_t UpdateAtomSizeToFile();
  virtual void UpdateToFile(AtomUpdateInfo &info, uint64_t duration) {
    if (_dumyType==A_STSS && !info.isKeyFrame) {
      return;
    }
    SignalSizeChange(-_bytesPerFrame);
  }
private:
  uint32_t _offset;
  uint32_t _bytesPerFrame;
  uint32_t _dumyType;
};

#endif	/* _ATOMDUMY_H */


#endif /* HAS_MEDIA_MP4 */
