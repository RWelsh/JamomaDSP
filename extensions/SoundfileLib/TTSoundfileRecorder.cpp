/* 
 * Jamoma DSP Soundfile Recorder
 * Copyright © 2010, Tim Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTSoundfileRecorder.h"

#define thisTTClass			TTSoundfileRecorder
#define thisTTClassName		"soundfile.recorder"
#define thisTTClassTags		"audio, soundfile, record"


TT_AUDIO_CONSTRUCTOR,
mFilePath(kTTSymEmpty),
mFormat(kTTSymEmpty),
mSoundFile(NULL),
mRecord(false),
mNumChannels(0),
mNumBufferFrames(0),
mLength(0),
mLengthInSamples(0),
mTimedRecord(false)
{
	addAttribute(			FilePath,		kTypeSymbol);
	addAttribute(			Format,			kTypeSymbol);
	addAttributeWithSetter(	Record,			kTypeBoolean);
	addAttribute(			NumChannels,	kTypeUInt16);
	addAttribute(			Length,			kTypeFloat64);
	addAttributeProperty(	NumChannels,	readOnly, kTTBoolYes);

	setProcessMethod(processAudio);
}


TTSoundfileRecorder::~TTSoundfileRecorder()
{
	setAttributeValue(TT("record"), kTTBoolNo);
	if (mSoundFile)
		sf_close(mSoundFile);
}
/*TTErr TTSoundfileRecorder::setLength(const TTValue& newValue)
{
	mLength = newValue;
	return kTTErrNone;
}*/

TTErr TTSoundfileRecorder::setRecord(const TTValue& newValue)
{
	TTBoolean	newRecordState = newValue;
	TTErr		err = kTTErrNone;
	
	if (mRecord != newRecordState) {
		mRecord = newRecordState;
		if (mRecord) {			// start recording
			mLengthInSamples = mLength * sr * 0.001; // reset the Sample counter 
			if (mLengthInSamples > 0)
				mTimedRecord = true;
			else 
				mTimedRecord = false;
			mNumChannels = 0;	// set to zero so that the process method will set the num channels and trigger an open
		}
		else {					// stop recording -- close the file
			if (mSoundFile)
				sf_close(mSoundFile);
			mSoundFile = NULL;
		}
	}
	return err;
}


// "FLAC-24bit" -> SF_FORMAT_FLAC | SF_FORMAT_PCM_24

int TTSoundfileRecorder::translateFormatFromName(TTSymbolPtr name)
{
	int		format = 0;
	char	cname[64];
	char*	s;
	
	if (name)
		strncpy(cname, name->getCString(), 64);
	else
		strncpy(cname, "CAF", 64);

	s = strrchr(cname, '-'); // look for subtype
	if (s) {
		*s = 0;
		s++;
		if (s) {
			if (strstr(s, "16bit"))
				format |= SF_FORMAT_PCM_16;
			else if (strstr(s, "24bit"))
				format |= SF_FORMAT_PCM_24;
			else if (strstr(s, "32bit"))
				format |= SF_FORMAT_PCM_32;
			else
				format |= SF_FORMAT_PCM_24;
		}
	}
	else { // no subtype, set default
		format |= SF_FORMAT_PCM_24;
	}

	// now look at the primary type
	if (strstr(cname, "FLAC"))
		format |= SF_FORMAT_FLAC;
	else if (strstr(cname, "AIFF"))
		format |= SF_FORMAT_AIFF;
	else if (strstr(cname, "WAV"))
		format |= SF_FORMAT_WAV;
	else if (strstr(cname, "Matlab"))
		format |= SF_FORMAT_MAT5;		
	else
		format |= SF_FORMAT_CAF;
	
	return format;	
}


TTErr TTSoundfileRecorder::openFile()
{
	memset(&mSoundFileInfo, 0, sizeof(mSoundFileInfo));	
	mSoundFileInfo.channels = mNumChannels;
	mSoundFileInfo.format = translateFormatFromName(mFormat);
	mSoundFileInfo.samplerate = sr;
	
	mSoundFile = sf_open(mFilePath->getCString(), SFM_WRITE, &mSoundFileInfo);
	
	if (!mSoundFile) {
		return kTTErrGeneric;
	}
	return kTTErrNone;
}


TTErr TTSoundfileRecorder::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&		in = inputs->getSignal(0);
	TTUInt16			channelCount = in.getNumChannelsAsInt();
	TTUInt16			numFrames = in.getVectorSizeAsInt();
	TTBoolean			bufferNeedsResize = NO;
	TTUInt16			n;
	TTSampleValuePtr	inSample;
	sf_count_t			numSamplesWritten;

	if (!mRecord)				// not recording
		return kTTErrNone;
	
	if (!mNumChannels) {		// this is the first frame to record, we need to set up the file
		mNumChannels = channelCount;
		bufferNeedsResize = YES;
		openFile();
	}
	if (mNumBufferFrames != numFrames) {
		mNumBufferFrames = numFrames;
		bufferNeedsResize = YES;
	}
	if (bufferNeedsResize)
		mBuffer.resize(mNumBufferFrames * mNumChannels);
	
	for (TTUInt16 channel=0; channel<channelCount; channel++) {
		inSample = in.mSampleVectors[channel];
		for (n=0; n<numFrames; n++)
			mBuffer[n * channelCount + channel] = inSample[n];
	}

	numSamplesWritten = sf_write_double(mSoundFile, &mBuffer[0], numFrames*channelCount);
	if (mTimedRecord){
		mLengthInSamples = mLengthInSamples - numFrames; // decreasing the samplecounter by a vs if we want to record with a duration
		if (mLengthInSamples <= 0) //TODO: we might want to chop of the samples that were recorded too long
			return setRecord(0);
	}	
	return kTTErrNone;
}
