/* 
 * Jamoma DSP Soundfile Recorder
 * Copyright © 2010, Nils Peters with a lot of help by Eric Battenberg
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTSoundfileRecorder2.h"

#define thisTTClass			TTSoundfileRecorder2
#define thisTTClassName		"soundfile.recorder.2"
#define thisTTClassTags		"audio, soundfile, record"


TT_AUDIO_CONSTRUCTOR,
mFilePath(kTTSymEmpty),
mFormat(kTTSymEmpty),
mSoundFile(NULL),
mRecord(false),
mNumChannels(0),
mLength(0),
mLengthInBlocks(0),
mCycles(0),
mBlocksPerBuffer(128),
mVs(64)
{   addAttribute( BlocksPerBuffer, kTypeUInt16);
	addAttributeWithSetter(	FilePath,		kTypeSymbol);
	addAttribute(			Format,			kTypeSymbol);
	addAttributeWithSetter(	Record,			kTypeBoolean);
	addAttribute(			NumChannels,	kTypeUInt16);
		addAttributeProperty(	NumChannels,	readOnly, kTTBoolYes);
	addAttributeWithSetter(	Length,			kTypeFloat64);
		addAttributeProperty(Length,			range,			TTValue(0, 60000));
		addAttributeProperty(Length,			rangeChecking,	TT("clip"));
	setProcessMethod(processAudioBypass);
}


TTSoundfileRecorder2::~TTSoundfileRecorder2() 
{
	//setAttributeValue(TT("record"), kTTBoolNo);
	if (mSoundFile)
		stopRecording();   
    pthread_attr_destroy(&ioth_attr);
}

TTErr TTSoundfileRecorder2::stopRecording()
{
	{// stop recording -- close the file
		if (mSoundFile){
			pthread_mutex_lock(&oth_mutex);
			oth_state = -1;
			pthread_cond_signal(&oth_cond);
			pthread_mutex_unlock(&oth_mutex);
			
			sf_close(mSoundFile);
			mSoundFile = NULL;			

			//free_vec64(&ybuf);
			//free_vec64(&ywrite);
			free_vec32(&ybuf);
			free_vec32(&ywrite);
				
			//mPosition = 0;
			pthread_mutex_destroy(&oth_mutex);
			pthread_cond_destroy(&oth_cond);
		}
	}
	return kTTErrNone;
}


TTErr TTSoundfileRecorder2::setRecord(const TTValue& newValue)
{
	TTBoolean	newRecordState = newValue;
	TTErr		err = kTTErrNone;	
	
	if (newRecordState == 1){
		if(!mSoundFile)
			setup();
		if (mLength <= 0)			
			setProcessMethod(processAudioRecording);
		else 
			setProcessMethod(processTimedAudioRecording);
		}
	else 
		setProcessMethod(processAudioBypass);
		
	if (mRecord != newRecordState) {
		mRecord = newRecordState;
		if (mRecord) {			// start recording
			mLengthInBlocks = ceil(((mLength * (double)sr * 0.001)) / (double)mVs); // reset the Sample counter 
			mCycles = 0;			
		} 
		else 
			stopRecording();

	}
	return err;
}

TTErr TTSoundfileRecorder2::setLength(const TTValue& newValue)
{	TTFloat64	newLength = newValue;
	if (newLength != mLength){
		mLength = newLength;
		//mNumBufferFrames = 0; //hack to force a resize of mBuffer in the process method 
	}
	return kTTErrNone;
}


// "FLAC-24bit" -> SF_FORMAT_FLAC | SF_FORMAT_PCM_24
// something to consider when you want to write large amount of data as fast as possible: http://www.mega-nerd.com/libsndfile/FAQ.html#Q006
int TTSoundfileRecorder2::translateFormatFromName(TTSymbolPtr name)
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
		format |= SF_FORMAT_MAT4;		
	else
		format |= SF_FORMAT_CAF;
	
	return format;	
}


TTErr TTSoundfileRecorder2::setFilePath(const TTValue& newValue)
{
	mFilePath = newValue;
	return kTTErrNone;
}


TTErr TTSoundfileRecorder2::openFile()
{
	memset(&mSoundFileInfo, 0, sizeof(mSoundFileInfo));	
	mSoundFileInfo.channels = mNumChannels;
	mSoundFileInfo.format = translateFormatFromName(mFormat);
	mSoundFileInfo.samplerate = sr;
	
	mSoundFile = sf_open(mFilePath->getCString(), SFM_WRITE, &mSoundFileInfo);
	
	if (!mSoundFile) {
		TTLogError("TTSoundfileRecorder2::openFile(): Can't create soundfile.\n");
		return kTTErrGeneric;
	}
	return kTTErrNone;
}

TTSoundfileRecorder2* TTSoundfileRecorder2::setup()
{
    // setup io thread priority 
    struct sched_param ioth_param;
    pthread_attr_init(&ioth_attr);
    pthread_attr_setdetachstate(&ioth_attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&ioth_attr, SCHED_FIFO);
    ioth_param.sched_priority = sched_get_priority_min(SCHED_FIFO)+1;
    pthread_attr_setschedparam(&ioth_attr, &ioth_param);
    pthread_attr_setinheritsched(&ioth_attr, PTHREAD_EXPLICIT_SCHED);
	
  	openFile();	
	//mPosition = 0;
    	
	mThisWrite = mBlocksPerBuffer*mVs;
	TTUInt64 size = mThisWrite*mNumChannels;
	ybuf = create_vec32(size);
	ywrite = create_vec32(size);
	//ybuf = create_vec64(size);
	//ywrite = create_vec64(size);
	
	
    pthread_mutex_init(&oth_mutex,NULL);
    pthread_cond_init(&oth_cond,NULL);
		
    oth_state = 0;
		
    int err = pthread_create(&oth, &ioth_attr, writeChunkThreadEntry, (void*)this);
	if ( err != 0){
		TTLogError("pthread_create error: %d \n",err); 
		if (err == EPERM)
			TTLogError("EPERM\n");
		exit(-1);
	}
	return this;
}



void* TTSoundfileRecorder2::writeChunkThreadEntry(void *pThis)
{
    TTSoundfileRecorder2 *io = (TTSoundfileRecorder2*)pThis;
	
    while(1){ //loop until oth_state == -1
		pthread_mutex_lock(&io->oth_mutex);
        while(io->oth_state == 0 || io->oth_state == 1)
			pthread_cond_wait(&io->oth_cond,&io->oth_mutex);
        if(io->oth_state == -1)
            break;
		io->writeChunk();		
        io->oth_state = 1; //tell main thread that computation is done
        pthread_cond_signal(&io->oth_cond);
        pthread_mutex_unlock(&io->oth_mutex);
    }
    pthread_exit(NULL);
	
}


int TTSoundfileRecorder2::writeChunk(void) 
{ // thread to read audio from file into xbuf
    if (sf_writef_float(mSoundFile, ywrite.data, mThisWrite) != mThisWrite){
    //if (sf_writef_double(mSoundFile, ywrite.data, mThisWrite) != mThisWrite){
		char errorStr[256];
		sf_error_str(mSoundFile, errorStr, 256);
		TTLogError("TTSoundfileRecorder2::writeChunk() Error while writing file: %s \n",errorStr);
	}	
    //mPosition += mThisWrite;	
	//TTLogMessage("Wrote %u frames to disk \n",thisWrite);    
    return 0;
}




TTErr TTSoundfileRecorder2::processAudioRecording(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{	
	//TTAudioSignal&		out = outputs->getSignal(0);
	TTAudioSignal&		in  = inputs->getSignal(0);
	//TTSampleValuePtr	outSample, inSample;
	TTUInt16			channelCount = in.getNumChannelsAsInt();
	TTUInt16			vs = in.getVectorSizeAsInt();
	TTUInt16			n, channel;
	//TTUInt64			progress = mCycles*vs*channelCount;
	TTUInt64			offset = mCycles*vs*channelCount;
	
	
	mVs = vs;
	mNumChannels = channelCount;
	
	for (n=0; n<vs; n++){
		for (channel=0; channel<channelCount; channel++) {
			ybuf.data[offset + channel] = in.mSampleVectors[channel][n]; //sending audio to recording buffer
			//out.mSampleVectors[channel][n] = in.mSampleVectors[channel][n];
			// memcpy(outSample, inSample, sizeof(TTSampleValue) * vs);
		}
		offset += channelCount;
	} 
	
	if (++mCycles >= mBlocksPerBuffer){		
		pthread_mutex_lock(&oth_mutex);
		while (oth_state == 2){ //thread still running but mutex acquired
			pthread_cond_wait(&oth_cond,&oth_mutex);
			TTLogError("WARNING TTSoundfileRecorder::processAudioRecording() output write started late \n");
		}
		if (oth_state == 1) //join thread
			 oth_state = 0;
				
		if (oth_state == 0)
		{
			vec32 temp_vec = ybuf;
			ybuf = ywrite;
			ywrite = temp_vec;
			mCycles = 0;
			//start thread
			oth_state = 2;  //2 is "run" signal
			pthread_cond_signal(&oth_cond);
			pthread_mutex_unlock(&oth_mutex);			
		}
	}	
	return kTTErrNone;
}


TTErr TTSoundfileRecorder2::processTimedAudioRecording(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{	
	//TTAudioSignal&		out = outputs->getSignal(0);
	TTAudioSignal&		in  = inputs->getSignal(0);
	//TTSampleValuePtr	outSample, inSample;
	TTUInt16			channelCount = in.getNumChannelsAsInt();
	TTUInt16			vs = in.getVectorSizeAsInt();
	TTUInt16			n, channel;
	TTUInt64			offset = mCycles*vs*channelCount;
	
	for (n=0; n<vs; n++){
		for (channel=0; channel<channelCount; channel++) {
			ybuf.data[offset + channel] = in.mSampleVectors[channel][n]; //sending audio to recording buffer
			// memcpy(outSample, inSample, sizeof(TTSampleValue) * vs);
			//out.mSampleVectors[channel][n] = in.mSampleVectors[channel][n];
		}
		offset += channelCount;
	}    
	
	if (++mCycles >= mBlocksPerBuffer){	
		pthread_mutex_lock(&oth_mutex);
		while (oth_state == 2){ //thread still running but mutex acquired
			pthread_cond_wait(&oth_cond,&oth_mutex);
			TTLogError("WARNING TTSoundfileRecorder::processAudioRecording() output write started late \n");
		}
		if (oth_state == 1) //join thread
			oth_state = 0;
		
		if (oth_state == 0)
		{	
			mCycles = 0;
			vec32 temp_vec = ybuf;
			ybuf = ywrite;
			ywrite = temp_vec;
			
			//start thread
			oth_state = 2;  //2 is "run" signal
			pthread_cond_signal(&oth_cond);
			pthread_mutex_unlock(&oth_mutex);			
		}
	}
	mLengthInBlocks--; // decreasing the samplecounter 
	if (mLengthInBlocks < 0){ //time to stop the recording
		mVs = vs;
		mNumChannels = channelCount;
		pthread_mutex_lock(&oth_mutex);
		while (oth_state == 2){ //thread still running but mutex acquired
			pthread_cond_wait(&oth_cond,&oth_mutex);
			TTLogError("WARNING error while writing the tail of the audio file \n");
		}
		if (oth_state == 1) //join thread
			oth_state = 0;
		
		if (oth_state == 0){
		    mThisWrite = (mCycles-1)*mVs; //the last few blocks to write to disk TODO: we might want to chop of the samples that were recorded too long
			vec32 temp_vec = ybuf;
			ybuf = ywrite;
			ywrite = temp_vec; //probably a ywrite = ybuf; would be enough here
			
			//start thread
			oth_state = 2;  //2 is "run" signal
			pthread_cond_signal(&oth_cond);
			pthread_mutex_unlock(&oth_mutex);		
		}
		
		pthread_mutex_lock(&oth_mutex);
		while (oth_state == 2) //thread still running but mutex acquired
			pthread_cond_wait(&oth_cond,&oth_mutex);
		pthread_mutex_unlock(&oth_mutex);
		return setRecord(0);
	} 
	else return kTTErrNone;
}


TTErr TTSoundfileRecorder2::processAudioBypass(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{	
	TTAudioSignal&		out = outputs->getSignal(0);
	TTAudioSignal&		in  = inputs->getSignal(0);
	TTSampleValuePtr	outSample, inSample;
	TTUInt16			channelCount = in.getNumChannelsAsInt();
	TTUInt16			vs = in.getVectorSizeAsInt();
	TTUInt16			channel;

	
	// not recording, just bypassing audio and return
	mNumChannels = channelCount;
	mVs = vs;
	 for (channel=0; channel<channelCount; channel++) {
		 inSample = in.mSampleVectors[channel];
		 outSample = out.mSampleVectors[channel]; // sending audio out
		 memcpy(outSample, inSample, sizeof(TTSampleValue) * vs);
	}
	return kTTErrNone;
} 







