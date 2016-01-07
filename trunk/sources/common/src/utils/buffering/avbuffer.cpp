#include "utils/buffering/avbuffer.h"

#ifdef WITH_SANITY_INPUT_BUFFER
#define SANITY_INPUT_BUFFER \
o_assert(_consumed<=_published); \
o_assert(_published<=_size);
#define SANITY_INPUT_BUFFER_INDEX \
o_assert(index >= 0); \
o_assert((_published - _consumed - index) > 0);
#else
#define SANITY_INPUT_BUFFER
#define SANITY_INPUT_BUFFER_INDEX
#endif


AVBuffer::AVBuffer() {
  _pBuffer = NULL;
  _size = 0;
  _published = 0;
  _consumed = 0;
  _timeStamp = 0;
  _isKeyframe = false;
  _minChunkSize = 0;
  SANITY_INPUT_BUFFER;
}

AVBuffer::AVBuffer(uint32_t size, double timeStamp,
                   bool isKeyframe, bool isAudio) {
  _pBuffer = NULL;
  _size = 0;
  _published = 0;
  _consumed = 0;
  _timeStamp = timeStamp;
  _minChunkSize = 0;
  _isKeyframe = isKeyframe;
  _isAudio= isAudio;
  EnsureSize(size);
}

AVBuffer::~AVBuffer() {
	SANITY_INPUT_BUFFER;
	Cleanup();
	SANITY_INPUT_BUFFER;
}


bool AVBuffer::ReadFromeBuffer(const uint8_t *pBuffer, const uint32_t size, bool isKeyframe) {
	SANITY_INPUT_BUFFER;
	if (!EnsureSize(size)) {
		SANITY_INPUT_BUFFER;
		return false;
	}
	memcpy(_pBuffer + _published, pBuffer, size);
	_published += size;
	SANITY_INPUT_BUFFER;
  _isKeyframe = isKeyframe;
	return true;
}

bool AVBuffer::IgnoreAll() {
	SANITY_INPUT_BUFFER;
	_consumed = _published;
	Recycle();
	SANITY_INPUT_BUFFER;

	return true;
}

bool AVBuffer::MoveData() {
	SANITY_INPUT_BUFFER;
	if (_published - _consumed <= _consumed) {
		memcpy(_pBuffer, _pBuffer + _consumed, _published - _consumed);
		_published = _published - _consumed;
		_consumed = 0;
	}
	SANITY_INPUT_BUFFER;

	return true;
}

bool AVBuffer::EnsureSize(uint32_t expected) {
	SANITY_INPUT_BUFFER;
	//1. Do we have enough space?
	if (_published + expected <= _size) {
		SANITY_INPUT_BUFFER;
		return true;
	}

	//2. Apparently we don't! Try to move some data
	MoveData();

	//3. Again, do we have enough space?
	if (_published + expected <= _size) {
		SANITY_INPUT_BUFFER;
		return true;
	}

	//4. Nope! So, let's get busy with making a brand new buffer...
	//First, we allocate at least 1/3 of what we have and no less than _minChunkSize
	if ((_published + expected - _size)<(_size / 3)) {
		expected = _size + _size / 3 - _published;
	}

	if (expected < _minChunkSize) {
		expected = _minChunkSize;
	}

	//5. Allocate
	uint8_t *pTempBuffer = new uint8_t[_published + expected];

	//6. Copy existing data if necessary and switch buffers
	if (_pBuffer != NULL) {
		memcpy(pTempBuffer, _pBuffer, _published);
		delete[] _pBuffer;
	}
	_pBuffer = pTempBuffer;

	//7. Update the size
	_size = _published + expected;
	SANITY_INPUT_BUFFER;

	return true;
}

void AVBuffer::Cleanup() {
	if (_pBuffer != NULL) {

		delete[] _pBuffer;
		_pBuffer = NULL;
	}
	_size = 0;
	_published = 0;
	_consumed = 0;
  _timeStamp = 0;
}

void AVBuffer::Recycle() {
	if (_consumed != _published)
		return;
	SANITY_INPUT_BUFFER;
	_consumed = 0;
	_published = 0;
	SANITY_INPUT_BUFFER;
}
