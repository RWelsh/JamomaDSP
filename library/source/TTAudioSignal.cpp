/* 
 * TTBlue Audio Signal Class
 * Copyright © 2008, Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTDSP.h"

#define thisTTClass			TTAudioSignal
#define thisTTClassName		"audiosignal"
#define thisTTClassTags		"audio, processor, dynamics, envelope"


/****************************************************************************************************/

TT_OBJECT_CONSTRUCTOR,
	mIsLocallyOwned(false),
	mMaxNumChannels(0),
	mVectorSize(0),
	mNumChannels(0),
	mBitdepth(0),
	mSampleRate(0),
	mSampleVectors(NULL)
{
	TTUInt16	initialMaxNumChannels = arguments;
	
	addAttributeWithSetter(	VectorSize, kTypeUInt16);
	addAttributeWithSetter(	NumChannels, kTypeUInt16);
	addAttributeWithSetter(	MaxNumChannels, kTypeUInt16);
	addAttribute(			Bitdepth, kTypeUInt8);
	addAttributeProperty(	Bitdepth, readOnly, kTTVal1);
	
	addMessage(clear);
	
	addMessage(alloc);
	addMessageWithArgument(allocWithNewVectorSize);
	addMessageWithArgument(setVector32);
	addMessageWithArgument(getVector32);
	addMessageWithArgument(setVector64);
	addMessageWithArgument(getVector64);
	
	addMessageProperty(alloc,					hidden, YES);
	addMessageProperty(allocWithNewVectorSize,	hidden, YES);
	addMessageProperty(setVector32,				hidden, YES);
	addMessageProperty(getVector32,				hidden, YES);
	addMessageProperty(setVector64,				hidden, YES);
	addMessageProperty(getVector64,				hidden, YES);
	
	setAttributeValue(kTTSym_maxNumChannels, initialMaxNumChannels);
	setAttributeValue(kTTSym_numChannels, initialMaxNumChannels);
}


TTAudioSignal::~TTAudioSignal()
{
	chuck();
}


void TTAudioSignal::chuck()
{
	TTUInt32	i;
	
	if (mIsLocallyOwned) {
		for (i=0; i<mMaxNumChannels; i++) {
			TTFree16(mSampleVectors[i]);
			mSampleVectors[i] = NULL;
		}
		mIsLocallyOwned = false;
	}
	TTFree16(mSampleVectors);
	mMaxNumChannels = 0;
	mSampleVectors = NULL;
}


TTErr TTAudioSignal::setMaxNumChannels(const TTValue& newValue)
{
	TTUInt32	i;
	TTBoolean	wasLocallyOwned = mIsLocallyOwned;
	TTUInt16	newMaxNumChannels = newValue;

	TTLimitMin<TTUInt16>(newMaxNumChannels, 1);
	if (TTUInt16(newMaxNumChannels) != mMaxNumChannels) {
		chuck();
		mMaxNumChannels = newMaxNumChannels;
		if (mMaxNumChannels) {
			mSampleVectors = (TTSampleValue**)TTMalloc16(sizeof(TTSampleValue*) * mMaxNumChannels);
			for (i=0; i<mMaxNumChannels; i++)
				mSampleVectors[i] = NULL;
		}
		if (wasLocallyOwned)
			alloc();
	}
	return kTTErrNone;
}


TTUInt16 TTAudioSignal::getMaxNumChannelsAsInt()
{
	return mMaxNumChannels;
}


TTErr TTAudioSignal::setVector(const TTUInt16 channel, const TTUInt16 newVectorSize, const TTSampleValuePtr newVector)
{
	TTUInt32	i;
	
	// could check against maxnumchannels here
	
	mBitdepth = 64;
	mVectorSize = newVectorSize;
	
	if (mIsLocallyOwned) {
		for (i=0; i<mMaxNumChannels; i++) {
			TTFree16(mSampleVectors[i]);
			mSampleVectors[i] = NULL;
		}
		mIsLocallyOwned = false;
	}
	mSampleVectors[channel] = newVector;
	return kTTErrNone;
}

TTErr TTAudioSignal::setVector64(const TTValue& v)
{
	TTUInt16		channel;
	TTUInt16		newVectorSize;
	TTPtr			newVector;
	
	if (v.getSize() == 3) {
		v.get(0, channel);
		v.get(1, newVectorSize);
		v.get(2, &newVector);
		return setVector(channel, newVectorSize, (TTSampleValue*)(newVector));
	}
	return kTTErrWrongNumValues;
}

TTErr TTAudioSignal::setVector64Copy(const TTUInt16 channel, const TTUInt16 vectorSize, const TTSampleValuePtr newVector)
{   
	if (mBitdepth != 64 || !mIsLocallyOwned || vectorSize != mVectorSize) {
		mBitdepth = 64;
		mVectorSize = vectorSize;
		alloc();
	}
	memcpy(mSampleVectors[channel], newVector, sizeof(TTSampleValue) * mVectorSize);		
	
	return kTTErrNone;
}

TTErr TTAudioSignal::setClearVector(const TTUInt16 channel, const TTUInt16 vectorSize)
{   
	if (mBitdepth != 64 || !mIsLocallyOwned || vectorSize != mVectorSize) {
		mBitdepth = 64;
		mVectorSize = vectorSize;
		alloc();
	}
	memset(mSampleVectors[channel],0,sizeof(TTSampleValue) * mVectorSize);	
	//memcpy(mSampleVectors[channel], newVector, sizeof(TTSampleValue) * mVectorSize);		
	
	return kTTErrNone;
}

/*
	It sucks if someone sets a 32-bit audio vector, since we have translate it into a 64-bit buffer.
	There may be a better way to do this...
	
	For now, we don't simply reference the data passed in.  Instead we allocate our own buffer and copy the data.
	Unfortunately, this is very slow.
	
	Also note that we are relying on the vector size already being set!
	
	If we passed the vs in to this method, we could avoid having to realloc the memory every single time.
	This would probably be a very good idea.
*/
TTErr TTAudioSignal::setVector(const TTUInt16 channel, const TTUInt16 newVectorSize, const TTFloat32* newVector)
{
	TTUInt32	i;
	
	// 1. could check against maxnumchannels here

	// 2. allocate the vector if need be
	if (mBitdepth != 32 || !mIsLocallyOwned || newVectorSize != mVectorSize) {
		mBitdepth = 32;
		mVectorSize = newVectorSize;
		alloc();
	}
	
	// 3. copy the vector (from 32-bits to 64-bits)
	for (i=0; i<mVectorSize; i++)
		mSampleVectors[channel][i] = newVector[i];
	
	return kTTErrNone;
}

TTErr TTAudioSignal::setVector32(const TTValue& v)
{
	TTUInt16		channel;
	TTUInt16		newVectorSize;
	TTPtr			newVector;
	
	if (v.getSize() == 3) {
		v.get(0, channel);
		v.get(1, newVectorSize);
		v.get(2, &newVector);
		return setVector(channel, newVectorSize, (TTFloat32*)newVector);
	}
	return kTTErrWrongNumValues;
}

TTErr TTAudioSignal::getVector(const TTUInt16 channel, const TTUInt16 returnedVectorSize, TTSampleValue* returnedVector)
{
	returnedVector = mSampleVectors[channel];
	return kTTErrNone;
}

TTErr TTAudioSignal::getVectorCopy(const TTUInt16 channel, const TTUInt16 theVectorSize, TTSampleValue* returnedVector)
{
	TTUInt16 i;
	
	for (i=0; i<theVectorSize; i++)
		returnedVector[i] = mSampleVectors[channel][i];
	return kTTErrNone;
}


TTErr TTAudioSignal::getVector64(TTValue& v)
{
	TTUInt16		channel;
	TTUInt16		theVectorSize;
	TTPtr			returnedVector;
	
	if (v.getSize() == 3) {
		v.get(0, channel);
		v.get(1, theVectorSize);
		v.get(2, &returnedVector);
		return getVector(channel, theVectorSize, (TTSampleValue*)(returnedVector));
	}
	return kTTErrWrongNumValues;
}


TTErr TTAudioSignal::getVector(const TTUInt16 channel, const TTUInt16 theVectorSize, TTFloat32* returnedVector)
{
	TTUInt16 i;
	
	for (i=0; i<theVectorSize; i++)
		returnedVector[i] = (TTFloat32)mSampleVectors[channel][i];
	return kTTErrNone;
}

TTErr TTAudioSignal::getVector32(TTValue& v)
{
	TTUInt16		channel;
	TTUInt16		theVectorSize;
	TTPtr			returnedVector;
	
	if (v.getSize() == 3) {
		v.get(0, channel);
		v.get(1, theVectorSize);
		v.get(2, &returnedVector);
		return getVector(channel, theVectorSize, (TTFloat32*)returnedVector);
	}
	return kTTErrWrongNumValues;
}


TTErr TTAudioSignal::alloc()
{
	TTUInt32	i;
	if (mIsLocallyOwned) {
		for (i=0; i<mMaxNumChannels; i++) {
			TTFree16(mSampleVectors[i]);
			mSampleVectors[i] = NULL;
		}
	}

	for (i=0; i<mMaxNumChannels; i++) {
		mSampleVectors[i] = (TTSampleValue*)TTMalloc16(sizeof(TTSampleValue) * mVectorSize);
	}
	mIsLocallyOwned = mMaxNumChannels > 0 ? true : false;
	// we can't do this here!  we are called by the setVector method for 32bit signals!
	//bitdepth = 64;
	return kTTErrNone;
}


TTErr TTAudioSignal::allocWithVectorSize(const TTUInt16 newVectorSize)
{
	// NOTE: we once tried removing the check for !mIsLocallyOwned
	// doing so causes Plugtastic plug-ins to crash in auval
	if ((newVectorSize != mVectorSize) || !mIsLocallyOwned) {
		mVectorSize = newVectorSize;
		return alloc();
	}
	else
		return kTTErrNone;
}

TTErr TTAudioSignal::allocWithNewVectorSize(const TTValue& newVectorSize)
{
	return allocWithVectorSize(TTUInt16(newVectorSize));
}


TTErr TTAudioSignal::reference(const TTAudioSignal& source, TTAudioSignal& dest)
{
	dest.chuck(); // sets isLocallyOwned to false
	dest.mSampleVectors = source.mSampleVectors;
	
	dest.mVectorSize = source.mVectorSize;
	dest.mMaxNumChannels = source.mMaxNumChannels;
	dest.mNumChannels = source.mNumChannels;
	dest.mBitdepth = source.mBitdepth;
	dest.mSampleRate = source.mSampleRate;
	
	return kTTErrNone;
}


TTErr TTAudioSignal::copy(const TTAudioSignal& source, TTAudioSignal& dest, TTUInt16 channelOffset)
{
	TTUInt16		vs;
	TTSampleValue*	inSample;
	TTSampleValue*	outSample;
	TTUInt16		maxDestChannels = dest.mMaxNumChannels;
	TTUInt16		numchannels = TTAudioSignal::getMinChannelCount(source, dest);
	TTUInt16		additionalOutputChannels = dest.mNumChannels - numchannels;
	TTUInt16		channel;
	
	for (channel=0; channel<numchannels; channel++) {
		inSample = source.mSampleVectors[channel];
		outSample = dest.mSampleVectors[ TTClip(channel+channelOffset, 0, maxDestChannels-1)  ];
		vs = source.getVectorSizeAsInt();
		//while (vs--)
		//	*outSample++ = *inSample++;
		
		memcpy(outSample, inSample, sizeof(TTSampleValue) * vs);
	}
	for (/*channel*/; channel<(numchannels+additionalOutputChannels-channelOffset); channel++) {
		outSample = dest.mSampleVectors[channel];
		vs = dest.getVectorSizeAsInt();
		memset(outSample, 0, sizeof(TTSampleValue) * vs);
		//while (vs--)
		//	*outSample++ = 0.0;
	}
	return kTTErrNone;
}


TTErr TTAudioSignal::copyDirty(const TTAudioSignal& source, TTAudioSignal& dest, TTUInt16 channelOffset)
{
	TTUInt16		vs;
	TTSampleValue*	inSample;
	TTSampleValue*	outSample;
	TTUInt16		maxDestChannels = dest.mMaxNumChannels;
	TTUInt16		numchannels = TTAudioSignal::getMinChannelCount(source, dest);
	TTUInt16		channel;
	
	for (channel=0; channel<numchannels; channel++) {
		inSample = source.mSampleVectors[channel];
		outSample = dest.mSampleVectors[ TTClip(channel+channelOffset, 0, maxDestChannels-1)  ];
		vs = source.getVectorSizeAsInt();
		//while (vs--)
		//	*outSample++ = *inSample++;
		memcpy(outSample, inSample, sizeof(TTSampleValue) * vs);
	}
	return kTTErrNone;
}


TTErr TTAudioSignal::copySubset(const TTAudioSignal& source, TTAudioSignal& dest, TTUInt16 startingChannel, TTUInt16 endingChannel)
{
	TTUInt16		vs;
	TTSampleValue*	inSample;
	TTSampleValue*	outSample;
	TTUInt16		sourceChannel;
	TTUInt16		destChannel;
	
	for (sourceChannel=startingChannel, destChannel=0; sourceChannel<=endingChannel; sourceChannel++, destChannel++) {
		inSample = source.mSampleVectors[sourceChannel];
		outSample = dest.mSampleVectors[destChannel];
		vs = source.getVectorSizeAsInt();
		//while (vs--)
		//	*outSample++ = *inSample++;
		memcpy(outSample, inSample, sizeof(TTSampleValue) * vs);
	}
	return kTTErrNone;
}

TTUInt16 TTAudioSignal::getMaxChannelCount(const TTAudioSignal& signal1, const TTAudioSignal& signal2)
{
	if (signal1.mNumChannels < signal2.mNumChannels)
		return signal2.mNumChannels;
	else
		return signal1.mNumChannels;
}

TTUInt16 TTAudioSignal::getMaxChannelCount(const TTAudioSignal& signal1, const TTAudioSignal& signal2, const TTAudioSignal& signal3)
{
	TTUInt16	numChannels = signal1.mNumChannels;
	
	if (signal2.mNumChannels > numChannels)
		numChannels = signal2.mNumChannels;
	if (signal3.mNumChannels > numChannels)
		numChannels = signal3.mNumChannels;
	
	return numChannels;
}


TTUInt16 TTAudioSignal::getNumChannels(const TTAudioSignal& signal)
{
	return signal.mNumChannels;
}


// TODO: implement fill() method --- SET ALL VALUES IN THE SIGNAL TO A CONSTANT

