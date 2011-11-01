/* 
 * Jamoma DSP Soundfile Recorder
 * Copyright © 2010, Tim Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __TT_SOUNDFILERECORDER2_H__
#define __TT_SOUNDFILERECORDER2_H__

#include "TTDSP.h"
#include "libsndfile/sndfile.h"
#include "sigproc_util.h"
#include <pthread.h>


class TTSoundfileRecorder2 : public TTAudioObject {
	TTCLASS_SETUP(TTSoundfileRecorder2)

	TTSymbolPtr			mFilePath;			///< full POSIX path to the file, including file name
	TTSymbolPtr			mFormat;			///< format of the file, e.g. "WAV", "AIFF", "FLAC", "FLAC-16bit", etc.
	SNDFILE*			mSoundFile;			///< libsndfile handle for the actual file we open
	SF_INFO				mSoundFileInfo;		///< libsndfile metadata for the file we open
	TTBoolean			mRecord;			///< is actively recording the file?
	TTUInt16			mNumChannels;		///< read-only: number of channels in the open file
	//TTUInt16			mNumBufferFrames;	///< number of frames in the buffer to be read from the file at a time
	TTFloat64			mLength;			///< length of the file in ms
	TTInt32				mLengthInBlocks;	///< length of the file in blocks
	TTUInt16			mCycles;
	//TTSampleVector		mBuffer;			///< buffer of mNumBufferFrames * mNumChannels;
	TTUInt16			mBlocksPerBuffer;
	TTUInt16			mVs;
	sf_count_t			numSamplesWritten;
	//int					mPosition;			///<index that takes care of how many data has been written to the output file
	int					mThisWrite; 
	//vec64 ybuf, ywrite;
	vec32 ybuf, ywrite;
	pthread_t ith;
	pthread_t oth;  
	pthread_attr_t ioth_attr;
	
	pthread_mutex_t ith_mutex;
	pthread_cond_t ith_cond;
	pthread_mutex_t oth_mutex;
	pthread_cond_t oth_cond;
	
	int ith_state;
	int oth_state;
	
	/**	Setter */
	TTErr setRecord(const TTValue& value);
	TTErr setLength(const TTValue& value);
	TTErr setFilePath(const TTValue& value);
	
	// internal use: map human symbols to libsndfile's bitmask
	int translateFormatFromName(TTSymbolPtr name);
	
	TTErr updateSampleRate(const TTValue& oldSampleRate);

	// internal use: opens the file for recording
	TTErr openFile();
	TTErr stopRecording();
	TTSoundfileRecorder2* setup();
	
	// Block-based Audio Processing Methods
	TTErr processAudioRecording(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	TTErr processTimedAudioRecording(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	TTErr processAudioBypass(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);

	//private:
	// output buffers
	
	
	
	
	static void *writeChunkThreadEntry(void *arg);
	int writeChunk(void);
	
	
};


#endif // __TT_SOUNDFILERECORDER2_H__
