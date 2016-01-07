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
#include "mediaformats/mp4/basemp4document.h"
#include "mediaformats/mp4/atomavc1.h"
#include "mediaformats/mp4/atomavcc.h"
#include "mediaformats/mp4/atomco64.h"
#include "mediaformats/mp4/atomctts.h"
#include "mediaformats/mp4/atomdata.h"
#include "mediaformats/mp4/atomdinf.h"
#include "mediaformats/mp4/atomdref.h"
#include "mediaformats/mp4/atomesds.h"
#include "mediaformats/mp4/atomftyp.h"
#include "mediaformats/mp4/atomhdlr.h"
#include "mediaformats/mp4/atomilst.h"
#include "mediaformats/mp4/atommdhd.h"
#include "mediaformats/mp4/atommdia.h"
#include "mediaformats/mp4/atommeta.h"
#include "mediaformats/mp4/atommetafield.h"
#include "mediaformats/mp4/atommfhd.h"
#include "mediaformats/mp4/atomminf.h"
#include "mediaformats/mp4/atommoof.h"
#include "mediaformats/mp4/atommoov.h"
#include "mediaformats/mp4/atommp4a.h"
#include "mediaformats/mp4/atommvex.h"
#include "mediaformats/mp4/atommvhd.h"
#include "mediaformats/mp4/atomnull.h"
#include "mediaformats/mp4/atomsmhd.h"
#include "mediaformats/mp4/atomstbl.h"
#include "mediaformats/mp4/atomstco.h"
#include "mediaformats/mp4/atomstsc.h"
#include "mediaformats/mp4/atomstsd.h"
#include "mediaformats/mp4/atomstss.h"
#include "mediaformats/mp4/atomstsz.h"
#include "mediaformats/mp4/atomstts.h"
#include "mediaformats/mp4/atomtfhd.h"
#include "mediaformats/mp4/atomtkhd.h"
#include "mediaformats/mp4/atomtraf.h"
#include "mediaformats/mp4/atomtrak.h"
#include "mediaformats/mp4/atomtrex.h"
#include "mediaformats/mp4/atomtrun.h"
#include "mediaformats/mp4/atomudta.h"
#include "mediaformats/mp4/atomurl.h"
#include "mediaformats/mp4/atomvmhd.h"
#include "mediaformats/mp4/atomwave.h"
#include "mediaformats/mp4/baseatom.h"
#include "mediaformats/mp4/ignoredatom.h"
#include "mediaformats/mp4/atomdumy.h"

//TODO: See how the things are impemented inside mp4v2. Good source
//for looking at the avcC atom format for exampl

BaseMP4Document::BaseMP4Document(Variant &metadata)
: BaseMediaDocument(metadata) {
	_pFTYP = NULL;
	_pMOOV = NULL;
}

BaseMP4Document::~BaseMP4Document() {

	FOR_VECTOR_ITERATOR(BaseAtom *, _allAtoms, i) {
		delete VECTOR_VAL(i);
	}
	_allAtoms.clear();
  _topAtoms.clear();
  _dumyAtoms.clear();
}

void BaseMP4Document::AddAtom(BaseAtom *pAtom, bool isTopAtom) {
	ADD_VECTOR_END(_allAtoms, pAtom);
  if (isTopAtom) {
	  ADD_VECTOR_END(_topAtoms, pAtom);
  }
  if (pAtom->GetTypeNumeric()==A_DUMY) {
	  ADD_VECTOR_END(_dumyAtoms, pAtom);
  }
}

BaseAtom* BaseMP4Document::ReadAtom(BaseAtom *pParentAtom) {
  NYI;
  return NULL;
}

#if 0
bool BaseMP4Document::ParseDocument() {

	if (!_mediaFile.SeekBegin()) {
		FATAL("Unable to seek to the beginning of file");
		return false;
	}


	while (!_mediaFile.IsEOF()) {
		uint64_t currentPos = _mediaFile.Cursor();
		if (currentPos == _mediaFile.Size()) {
			//FINEST("\n%s", STR(Hierarchy()));
			return true;
		}
		BaseAtom *pAtom = ReadAtom(NULL);
		if (pAtom == NULL) {
			FATAL("Unable to read atom");
			return false;
		}
		if (!pAtom->IsIgnored()) {
			switch (pAtom->GetTypeNumeric()) {
				case A_FTYP:
					_pFTYP = (AtomFTYP *) pAtom;
					break;
				case A_MOOV:
					_pMOOV = (AtomMOOV *) pAtom;
					break;
				case A_MOOF:
					ADD_VECTOR_END(_moof, (AtomMOOF *) pAtom);
					break;
				default:
				{
					FATAL("Invalid atom %s", STR(pAtom->GetTypeString()));
					return false;
				}
			}
		}
		ADD_VECTOR_END(_topAtoms, pAtom);
	}

	return true;
}

bool BaseMP4Document::BuildFrames() {
	_frames.clear();

	AtomTRAK *pTrack;
	AtomAVCC *pAVCC = NULL;
	if ((pTrack = GetTRAK(false)) != NULL) {
		pAVCC = (AtomAVCC *) pTrack->GetPath(6, A_MDIA, A_MINF,
				A_STBL, A_STSD, A_AVC1, A_AVCC);
	}

	AtomESDS *pESDS = NULL;
	if ((pTrack = GetTRAK(true)) != NULL) {
		pESDS = (AtomESDS *) pTrack->GetPath(6, A_MDIA, A_MINF,
				A_STBL, A_STSD, A_MP4A, A_ESDS);
		if (pESDS == NULL) {
			pESDS = (AtomESDS *) pTrack->GetPath(7, A_MDIA, A_MINF,
					A_STBL, A_STSD, A_MP4A, A_WAVE, A_ESDS);
		}
	}

	if (pESDS != NULL) {
		//Build audio frames
		if (!BuildMOOVFrames(true)) {
			FATAL("Unable to build audio frames from MOOV");
			return false;
		}
	}

	if (pAVCC != NULL) {
		//Build video frames
		if (!BuildMOOVFrames(false)) {
			FATAL("Unable to build video frames from MOOV");
			return false;
		}
	}

	for (uint32_t i = 0; i < _moof.size(); i++) {
		if (!BuildMOOFFrames(_moof[i], true)) {
			FATAL("Unable to build audio frames from MOOF");
			return false;
		}
		if (!BuildMOOFFrames(_moof[i], false)) {
			FATAL("Unable to build video frames from MOOF");
			return false;
		}
	}

	//sort the frames
	sort(_frames.begin(), _frames.end(), CompareFrames);

	//add binary audio header
	MediaFrame audioHeader = {0, 0, 0, 0, 0, 0, 0, 0};
	if (pESDS != NULL) {
		audioHeader.type = MEDIAFRAME_TYPE_AUDIO;
		audioHeader.isBinaryHeader = true;
		audioHeader.isKeyFrame = true;
		audioHeader.length = pESDS->GetExtraDataLength();
		audioHeader.absoluteTime = 0;
		audioHeader.start = pESDS->GetExtraDataStart();
		audioHeader.deltaTime = 0;
		audioHeader.compositionOffset = 0;
		IOBuffer raw;
		if (!_mediaFile.SeekTo(audioHeader.start)) {
			FATAL("Unable to seek into media file");
			return false;
		}
		if (!raw.ReadFromFs(_mediaFile, (uint32_t) audioHeader.length)) {
			FATAL("Unable to read from media file");
			return false;
		}
		if (!_streamCapabilities.InitAudioAAC(GETIBPOINTER(raw),
				GETAVAILABLEBYTESCOUNT(raw))) {
			FATAL("Unable to initialize AAC codec");
			return false;
		}
		//		FINEST("Start: %"PRIu64" (%"PRIx64"); Length: %"PRIu64" (%"PRIx64");",
		//				audioHeader.start, audioHeader.start, audioHeader.length,
		//				audioHeader.length);
	} else {
		if ((pTrack = GetTRAK(true)) != NULL) {
			BaseAtom *pMP3 = pTrack->GetPath(5, A_MDIA, A_MINF,
					A_STBL, A_STSD, A_MP3);
			if (pMP3 != NULL) {
				_streamCapabilities.audioCodecId = CODEC_AUDIO_MP3;
			}
		}
	}

	//add binary video header
	MediaFrame videoHeader = {0, 0, 0, 0, 0, 0, 0, 0};
	if (pAVCC != NULL) {
		videoHeader.type = MEDIAFRAME_TYPE_VIDEO;
		videoHeader.isBinaryHeader = true;
		videoHeader.isKeyFrame = true;
		videoHeader.length = pAVCC->GetExtraDataLength();
		videoHeader.absoluteTime = 0;
		videoHeader.start = pAVCC->GetExtraDataStart();
		videoHeader.deltaTime = 0;
		videoHeader.compositionOffset = 0;
		IOBuffer raw;
		if (!_mediaFile.SeekTo(videoHeader.start)) {
			FATAL("Unable to seek into media file");
			return false;
		}
		if (!raw.ReadFromFs(_mediaFile, (uint32_t) videoHeader.length)) {
			FATAL("Unable to read from media file");
			return false;
		}
		if (GETAVAILABLEBYTESCOUNT(raw) < 8) {
			FATAL("Invalid AVC codec bytes");
			return false;
		}
		uint32_t spsLength = ENTOHSP(GETIBPOINTER(raw) + 6);
		if (GETAVAILABLEBYTESCOUNT(raw) < 8 + spsLength + 1 + 2) {
			FATAL("Invalid AVC codec bytes");
			return false;
		}
		uint32_t ppsLength = ENTOHSP(GETIBPOINTER(raw) + 8 + spsLength + 1);
		if (GETAVAILABLEBYTESCOUNT(raw) < 8 + spsLength + 3 + ppsLength) {
			FATAL("Invalid AVC codec bytes");
			return false;
		}
		if (!_streamCapabilities.InitVideoH264(
				GETIBPOINTER(raw) + 8,
				spsLength,
				GETIBPOINTER(raw) + 8 + spsLength + 3,
				ppsLength)) {
			FATAL("Unable to initialize AVC codec");
			return false;
		}
	}

	if (pESDS != NULL) {
		ADD_VECTOR_BEGIN(_frames, audioHeader);
	} else {
		WARN("No sound track found");
	}

	if (pAVCC != NULL) {
		ADD_VECTOR_BEGIN(_frames, videoHeader);
	} else {
		WARN("No video track found");
	}

	return true;
}

bool BaseMP4Document::BuildMOOVFrames(bool audio) {
	//1. Get the track
	AtomTRAK *pTrack = GetTRAK(audio);
	if (pTrack == NULL) {
		FATAL("no %s track", audio ? "Audio" : "Video");
		return false;
	}

	//2. Get the atom containing sample sizes
	AtomSTSZ *pSTSZ = (AtomSTSZ *) pTrack->GetPath(4, A_MDIA, A_MINF, A_STBL,
			A_STSZ);
	if (pSTSZ == NULL) {
		FATAL("no STSZ");
		return false;
	}

	//3. Get the atom containing the chuncks offests
	AtomCO64 *pCO64 = NULL;
	AtomSTCO *pSTCO = (AtomSTCO *) pTrack->GetPath(4, A_MDIA, A_MINF, A_STBL,
			A_STCO);
	if (pSTCO == NULL) {
		pCO64 = (AtomCO64 *) pTrack->GetPath(4, A_MDIA, A_MINF, A_STBL, A_CO64);
		if (pCO64 == NULL) {
			FATAL("no CO64");
			return false;
		}
	}

	//4. Get the atom containing the distribution of samples per corresponding
	//chunks
	AtomSTSC *pSTSC = (AtomSTSC *) pTrack->GetPath(4, A_MDIA, A_MINF, A_STBL,
			A_STSC);
	if (pSTSC == NULL) {
		FATAL("no STSC");
		return false;
	}

	//5. Get the atom containing the delta time of each sample
	AtomSTTS *pSTTS = (AtomSTTS *) pTrack->GetPath(4, A_MDIA, A_MINF, A_STBL,
			A_STTS);
	if (pSTTS == NULL) {
		FATAL("no STTS");
		return false;
	}

	//6. Get the atom containing the time scale of each delta time
	AtomMDHD *pMDHD = (AtomMDHD *) pTrack->GetPath(2, A_MDIA, A_MDHD);
	if (pMDHD == NULL) {
		FATAL("no MDHD");
		return false;
	}

	//7. Get the table containing the samples marked as key frames. It can be null
	//It can be null
	AtomSTSS *pSTSS = (AtomSTSS *) pTrack->GetPath(4, A_MDIA, A_MINF, A_STBL,
			A_STSS);

	//8. Get the composition timestamps
	AtomCTTS *pCTSS = (AtomCTTS *) pTrack->GetPath(4, A_MDIA, A_MINF, A_STBL,
			A_CTTS);

	//8. Extract normalized information from the atoms retrived above
	vector<uint64_t> sampleSize = pSTSZ->GetEntries();
	if (audio)
		_audioSamplesCount = (uint32_t) sampleSize.size();
	else
		_videoSamplesCount = (uint32_t) sampleSize.size();
	vector<uint32_t> sampleDeltaTime = pSTTS->GetEntries();
	vector<uint64_t> chunckOffsets;
	if (pSTCO != NULL)
		chunckOffsets = pSTCO->GetEntries();
	else
		chunckOffsets = pCO64->GetEntries();
	vector<uint32_t> sample2Chunk = pSTSC->GetEntries((uint32_t) chunckOffsets.size());
	vector<uint32_t> keyFrames;
	if (pSTSS != NULL)
		keyFrames = pSTSS->GetEntries();
	vector<int32_t> compositionOffsets;
	if (pCTSS != NULL) {
		compositionOffsets = pCTSS->GetEntries();
		if (sampleSize.size() != compositionOffsets.size()) {
			WARN("composition offsets count != samples count; compositionOffsets: %"PRIz"u; sampleSize.size: %"PRIz"u",
					compositionOffsets.size(),
					sampleSize.size());
			for (uint32_t i = (uint32_t) compositionOffsets.size(); i < (uint32_t) sampleSize.size(); i++)
				ADD_VECTOR_END(compositionOffsets, 0);
			WARN("composition offsets padded with 0. Now size is %"PRIz"u",
					compositionOffsets.size());
		}
	}
	INFO("audio: %d; keyFrames: %"PRIz"u; frames: %"PRIz"u; compositionOffsets: %"PRIz"u",
			audio, keyFrames.size(), sampleSize.size(), compositionOffsets.size());

	uint32_t timeScale = pMDHD->GetTimeScale();
	uint64_t totalTime = 0;
	uint32_t localOffset = 0;
	uint32_t startIndex = (uint32_t) _frames.size();

	for (uint32_t i = 0; i < sampleSize.size(); i++) {
		MediaFrame frame = {0, 0, 0, 0, 0, 0, 0, 0};
		frame.start = chunckOffsets[sample2Chunk[i]] + localOffset;
		if (pCTSS != NULL) {
			double doubleVal = ((double) compositionOffsets[i] / (double) timeScale)*(double) 1000.00;
			frame.compositionOffset = (int32_t) doubleVal;
		} else {
			frame.compositionOffset = 0;
		}
		if (i <= sampleSize.size() - 2) {
			//not the last frame
			uint32_t currentChunck = sample2Chunk[i];
			uint32_t nextChunck = sample2Chunk[i + 1];


			if (currentChunck == nextChunck) {
				//not changing the chunk
				localOffset += (uint32_t) sampleSize[i];
			} else {
				//changing the chunck
				localOffset = 0;
			}

		} else {
			// we are near end, so we are on the same chunck
			localOffset += (uint32_t) sampleSize[i];
		}

		frame.length = sampleSize[i];
		frame.type = audio ? MEDIAFRAME_TYPE_AUDIO : MEDIAFRAME_TYPE_VIDEO;
		if (pSTSS == NULL) {
			//no kayframe info. All frames are considered key frames
			frame.isKeyFrame = true;
		} else {
			//default it to false, will set it later
			frame.isKeyFrame = false;
		}
		//frame.isKeyFrame = true;
		frame.deltaTime = ((double) sampleDeltaTime[i] / (double) timeScale)*(double) 1000.00;
		frame.absoluteTime = ((double) totalTime / (double) timeScale)*(double) 1000.00;
		frame.isBinaryHeader = false;
		totalTime += sampleDeltaTime[i];

		ADD_VECTOR_END(_frames, frame);
	}

	FOR_VECTOR(keyFrames, i) {
		_frames[startIndex + keyFrames[i] - 1].isKeyFrame = true;
	}

	return true;
}

bool BaseMP4Document::BuildMOOFFrames(AtomMOOF *pMOOF, bool audio) {
	AtomTRAF *pTraf = GetTRAF(pMOOF, audio);
	if (pTraf == NULL) {
		WARN("No %s fragmented track found", audio ? "audio" : "video");
		return true;
	}

	AtomTFHD *pTfhd = (AtomTFHD *) pTraf->GetPath(1, A_TFHD);
	if (pTfhd == NULL) {
		FATAL("Invalid track. No TFHD atom");
		return false;
	}

	AtomTRAK *pTrack = GetTRAK(audio);
	if (pTrack == NULL) {
		FATAL("no %s track", audio ? "Audio" : "Video");
		return false;
	}

	AtomMDHD *pMDHD = (AtomMDHD *) pTrack->GetPath(2, A_MDIA, A_MDHD);
	if (pMDHD == NULL) {
		FATAL("no MDHD");
		return false;
	}

	uint32_t timeScale = pMDHD->GetTimeScale();
	uint64_t totalTime = 0;
	int64_t absoluteOffset = pTfhd->GetBaseDataOffset();

	uint32_t runSize = 0;
	vector<AtomTRUN *> &runs = pTraf->GetRuns();
	for (uint32_t runId = 0; runId < runs.size(); runId++) {
		AtomTRUN *pRun = runs[runId];
		vector<TRUNSample *> &samples = pRun->GetSamples();
		runSize = 0;
		for (uint32_t sampleId = 0; sampleId < samples.size(); sampleId++) {
			TRUNSample *pSample = samples[sampleId];
			MediaFrame frame = {0, 0, 0, 0, 0, 0, 0, 0};

			frame.start = absoluteOffset + pRun->GetDataOffset() + runSize;
			if (pSample->compositionTimeOffset != 0) {
				double doubleVal = ((double) pSample->compositionTimeOffset / (double) timeScale)*(double) 1000.00;
				frame.compositionOffset = (int32_t) doubleVal;
			} else {
				frame.compositionOffset = 0;
			}

			if (!audio) {
				frame.isKeyFrame = ((pSample->flags & 0x00010000) == 0);
			} else {
				frame.isKeyFrame = false;
			}
			frame.length = pSample->size;
			frame.type = audio ? MEDIAFRAME_TYPE_AUDIO : MEDIAFRAME_TYPE_VIDEO;
			frame.deltaTime = ((double) pSample->duration / (double) timeScale)*(double) 1000.00;
			frame.absoluteTime = ((double) totalTime / (double) timeScale)*(double) 1000.00;
			frame.isBinaryHeader = false;
			totalTime += pSample->duration;

			ADD_VECTOR_END(_frames, frame);
			runSize += pSample->size;
		}
	}

	return true;
}

Variant BaseMP4Document::GetRTMPMeta() {
	Variant result;

	AtomTRAK *pVideoTrack = GetTRAK(false);
	if (pVideoTrack != NULL) {
		AtomTKHD *pTKHD = (AtomTKHD *) pVideoTrack->GetPath(1, A_TKHD);
		if (pTKHD != NULL) {
			result["width"] = pTKHD->GetWidth();
			result["height"] = pTKHD->GetHeight();
		}
	}

	if (_pMOOV != NULL) {
		AtomILST *pILST = (AtomILST *) _pMOOV->GetPath(3, A_UDTA, A_META, A_ILST);

		if (pILST != NULL) {
			result["tags"] = pILST->GetVariant();
		} else {
			WARN("No ilst atom present");
		}
	}

	return result;
}

string BaseMP4Document::Hierarchy() {
	string result = "";
	for (uint32_t i = 0; i < _topAtoms.size(); i++) {
		result += _topAtoms[i]->Hierarchy(1);
		if (i != _topAtoms.size() - 1)
			result += "\n";
	}
	return result;
}

AtomTRAK * BaseMP4Document::GetTRAK(bool audio) {
	if (_pMOOV == NULL) {
		FATAL("Unable to find moov");
		return NULL;
	}
	vector<AtomTRAK *> tracks = _pMOOV->GetTracks();
	if (tracks.size() == 0) {
		FATAL("No tracks defined");
		return NULL;
	}
	for (uint32_t i = 0; i < tracks.size(); i++) {
		AtomHDLR *pHDLR = (AtomHDLR *) tracks[i]->GetPath(2, A_MDIA, A_HDLR);
		if (audio && (pHDLR->GetComponentSubType() == A_SOUN))
			return tracks[i];
		if ((!audio) && (pHDLR->GetComponentSubType() == A_VIDE))
			return tracks[i];
	}
	return NULL;
}

AtomTRAF * BaseMP4Document::GetTRAF(AtomMOOF *pMOOF, bool audio) {
	AtomTRAK *pTrak = GetTRAK(audio);
	if (pTrak == NULL) {
		FATAL("No track found");
		return NULL;
	}
	uint32_t trackId = pTrak->GetId();
	if (trackId == 0) {
		FATAL("No track found");
		return NULL;
	}

	map<uint32_t, AtomTRAF *> &trafs = pMOOF->GetTrafs();
	if (!MAP_HAS1(trafs, trackId)) {
		FATAL("No track found");
		return NULL;
	}

	return trafs[trackId];
}

#endif

#endif /* HAS_MEDIA_MP4 */
