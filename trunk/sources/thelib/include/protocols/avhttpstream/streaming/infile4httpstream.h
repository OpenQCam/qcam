// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc. 
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    : 
// // Version : 
// // Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// // ------------------------------------------------------------------
// // Purpose : 
// // ======================================================================
//

#ifndef _INFILE4HTTPSTREAM_H
#define	_INFILE4HTTPSTREAM_H

#include "streaming/baseinstream.h"
#include "protocols/timer/basetimerprotocol.h"

#ifdef HAS_MMAP
#define FileClass MmapFile
#else
#define FileClass File
#endif

#define IN_FILE_STREAM_NAME       "ifssname"
#define IN_FILE_STREAM_FILE_NAME  "ifsname"
#define IN_FILE_STREAM_FILE_SIZE  "ifssize"
#define IN_FILE_STREAM_FILE_DIR   "ifsdir"
#define IN_FILE_STREAM_FILE_PATH  "ifspath"
#define IN_FILE_STREAM_TYPE       "ifstype"
#define IN_FILE_STREAM_DURATION   "ifsduration"
#define IN_FILE_STREAM_STATE      "ifsstate"
#define IN_FILE_STREAM_CURSOR     "ifscursor"
#define IN_FILE_STREAM_WINDOW     "ifswindow"

class InboundAVHTTPStreamProtocol;
class StreamsManager;
/*!
	@class InFile4HTTPStream
	@brief
 */
class DLLEXP InFile4HTTPStream
: public BaseInStream {
private:

	class InFileStreamTimer
	: public BaseTimerProtocol {
	private:
		InFile4HTTPStream *_pInFileStream;
	public:
		InFileStreamTimer(InFile4HTTPStream *pInFileStream);
		virtual ~InFileStreamTimer();
		void ResetStream();
		virtual bool TimePeriodElapsed();
	};
	friend class InFileStreamTimer;
#ifndef HAS_MMAP
	static map<string, pair<uint32_t, File *> > _fileCache;
#endif /* HAS_MMAP */

	InFileStreamTimer *_pTimer;

	FileClass *_pFile;

	//timing info
	uint32_t _feedPeriod;
	uint32_t _totalSentTime;
	time_t _startFeedingTime;

	//buffering info
	uint32_t _bufferWindow;
	IOBuffer _buffer;

	//current state info
	uint8_t _streamingState;

	//stream capabilities
	StreamCapabilities _streamCapabilities;

public:
	InFile4HTTPStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager,
			uint64_t type, string name);
	virtual ~InFile4HTTPStream();

	static InFile4HTTPStream *GetInstance(InboundAVHTTPStreamProtocol *pRTMPProtocol,
			StreamsManager *pStreamsManager, Variant &metadata);

	virtual bool FeedData(uint8_t *pData, uint32_t dataLength,
			uint32_t processedLength, uint32_t totalLength,
			double absoluteTimestamp, bool isAudio);

	virtual bool IsCompatibleWithType(uint64_t type);

	virtual void SignalOutStreamAttached(BaseOutStream *pOutStream);
	virtual void SignalOutStreamDetached(BaseOutStream *pOutStream);



	/*!
	  @brief Returns the stream capabilities. Specifically, codec and codec related info
	 */
	virtual StreamCapabilities * GetCapabilities();

	/*!
		@brief Extracts the complete metadata from partial metadata
		@param metaData - the partial metadata containing at least the media file name
	 */
	bool ResolveCompleteMetadata(Variant &metaData);

	/*!
		@brief This will initialize the stream internally.
		@param clientSideBufferLength - the client side buffer length expressed in seconds
	 */
	virtual bool Initialize(Variant &metadata, bool hasTimer);

	/*!
		@brief Called when a play command was issued
		@param absoluteTimestamp - the timestamp where we want to seek before start the feeding process
	 */
	virtual bool SignalPlay(double &absoluteTimestamp, double &length);

	/*!
		@brief Called when a pasue command was issued
	 */
	virtual bool SignalPause();

	/*!
		@brief Called when a resume command was issued
	 */
	virtual bool SignalResume();

	/*!
		@brief Called when a seek command was issued
		@param absoluteTimestamp
	 */
	virtual bool SignalSeek(double &absoluteTimestamp);

	/*!
		@brief Called when a stop command was issued
	 */
	virtual bool SignalStop();

	/*!
		@brief This is called by the framework. The networking layer signaled the availability for sending data
	 */
	virtual void ReadyForSend();

public:
	/*!
		@brief This is the function that will actually do the feeding.
		@discussion It is called by the framework and it must deliver one frame at a time to all subscribers
	 */

	virtual bool Feed();
private:
	/*!
		@brief GetFile function will open a file and will cache it if is a regular file.
		@discussion If the file is mmap based file, it will NOT cache it
		ReleaseFile will do the opposite: It will close the file if the references
		count will reach 0. This always happens in case of mmap file
	 */
#ifdef HAS_MMAP
	static MmapFile* GetFile(string filePath, uint32_t windowSize);
	static void ReleaseFile(MmapFile *pFile);
#else
	static bool IsFileOpened(string filePath);
	static File* GetFile(string filePath, uint32_t windowSize);
	static void ReleaseFile(File *pFile);
#endif /* HAS_MMAP */

};

#endif	/* _INFILE4HTTPSTREAM_H */

