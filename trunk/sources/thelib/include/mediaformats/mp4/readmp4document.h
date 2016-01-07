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
#ifndef _READMP4DOCUMENT_H
#define	_READMP4DOCUMENT_H

#include "common.h"
#include "mediaformats/mp4/basemp4document.h"

class ReadMP4Document
: public BaseMP4Document {

//private:
//	vector<BaseAtom *> _allAtoms;
//	vector<BaseAtom *> _topAtoms;
//	AtomFTYP *_pFTYP;
//	AtomMOOV *_pMOOV;
//	vector<AtomMOOF*> _moof;


public:
	ReadMP4Document(Variant &metadata);
	virtual ~ReadMP4Document();

	//void AddAtom(BaseAtom *pAtom);
  BaseAtom *ReadAtom(BaseAtom *pParentAtom);
protected:
	virtual bool ParseDocument();
	virtual bool BuildFrames();
	virtual Variant GetRTMPMeta();

private:
	string Hierarchy();
	AtomTRAK * GetTRAK(bool audio);
	AtomTRAF * GetTRAF(AtomMOOF *pMOOF, bool audio);
	bool BuildMOOVFrames(bool audio);
	bool BuildMOOFFrames(AtomMOOF *pMOOF, bool audio);
};

#endif	/* _MP4DOCUMENT_H */


#endif /* HAS_MEDIA_MP4 */
