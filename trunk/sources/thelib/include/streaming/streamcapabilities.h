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


#ifndef _STRAMCAPABILITIES_H
#define	_STRAMCAPABILITIES_H

#include "streaming/codectypes.h"

struct _VIDEO_AVC {
	uint8_t *_pSPS;
	uint16_t _spsLength;
	uint8_t *_pPPS;
	uint16_t _ppsLength;
	uint32_t _rate;
	Variant _SPSInfo;
	Variant _PPSInfo;
	uint32_t _width;
	uint32_t _height;
	uint32_t _widthOverride;
	uint32_t _heightOverride;

	DLLEXP _VIDEO_AVC();
	DLLEXP virtual ~_VIDEO_AVC();
	bool Init(const uint8_t *pSPS, uint32_t spsLength, const uint8_t *pPPS, uint32_t ppsLength);
	void Clear();

	bool Serialize(IOBuffer & dest);
	static bool Deserialize(IOBuffer &src, _VIDEO_AVC & dest);
	DLLEXP operator string();
};

struct _AUDIO_AAC {
	uint8_t *_pAAC;
	uint32_t _aacLength;
	uint8_t _audioObjectType;
	uint8_t _sampleRateIndex;
	uint32_t _sampleRate;
	uint8_t _channelConfigurationIndex;

	uint8_t _numOfChannels;
  uint8_t _bitsPerChannels;
  uint32_t _frameLength;
  uint32_t _encodeBitRate;
  double   _encodeDurationTime;

	DLLEXP _AUDIO_AAC();
	DLLEXP virtual ~_AUDIO_AAC();
	bool Init(const uint8_t *pBuffer, uint32_t length);
	bool InitAACCapability(uint32_t sampleRate, uint8_t numOfChannels, uint8_t bitsPerChannels, uint32_t frameLength,
				uint32_t encodeBitRate, double encodeDurationTime);
	void Clear();
	DLLEXP string GetRTSPFmtpConfig();

	bool Serialize(IOBuffer & dest);
	static bool Deserialize(IOBuffer &src, _AUDIO_AAC & dest);
	operator string();
};

struct _AUDIO_PCM {
	uint32_t _sampleRate;
  uint32_t _numOfChannels;

	DLLEXP _AUDIO_PCM();
	DLLEXP virtual ~_AUDIO_PCM();
	void Clear();
};

class DLLEXP StreamCapabilities {
public:
	uint64_t videoCodecId;
	uint64_t audioCodecId;
	_VIDEO_AVC avc;
	_AUDIO_AAC aac;
	_AUDIO_PCM pcm;
	uint32_t bandwidthHint;

public:
	StreamCapabilities();
	virtual ~StreamCapabilities();

	bool InitAudioAAC(const uint8_t *pBuffer, uint32_t length);
	bool InitAudioADTS();
	bool InitAudioMP3();
	bool InitAudioG711();
	bool InitAudioPCM(uint32_t sampleRate, uint32_t numOfChannels);
	bool InitVideoMJPG();
	bool InitVideoH264(const uint8_t *pSPS, uint32_t spsLength, const uint8_t *pPPS,
			uint32_t ppsLength);

	void ClearVideo();
	void ClearAudio();
	void Clear();

	bool Serialize(IOBuffer &dest);
	static bool Deserialize(string seekFilePath, StreamCapabilities &capabilities);
	static bool Deserialize(IOBuffer &src, StreamCapabilities &capabilities);
};

#endif	/* _STRAMCAPABILITIES_H */

