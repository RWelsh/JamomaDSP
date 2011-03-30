/*
 * Jamoma DSP Audio Buffer Object 
 * Copyright © 2003, Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTBuffer.h"

#define thisTTClass			TTBuffer
#define thisTTClassName		"buffer"
#define thisTTClassTags		"audio, buffer"


TTObjectPtr TTBuffer::instantiate(TTSymbolPtr name, TTValue& arguments)
{
	return new TTBuffer(arguments);
}


extern "C" void TTBuffer::registerClass() 
{
	TTClassRegister(TT("buffer"), thisTTClassTags, TTBuffer::instantiate);
}


TTBuffer::TTBuffer(TTValue& arguments) : 
	TTMatrix(arguments),
	mSampleRate(44100.0)
{
	addAttributeWithGetterAndSetter(NumChannels,		kTypeUInt16);
	addAttributeWithGetterAndSetter(Length,				kTypeFloat64);
	addAttributeWithGetterAndSetter(LengthInSamples,	kTypeUInt64);
	addAttribute(SampleRate,							kTypeFloat64);
	
	addMessage(clear);
	addMessageWithArgument(fill);

	addMessageWithArgument(getValueAtIndex);
	registerMessage(TT("peek"), (TTMethod)&TTBuffer::getValueAtIndex);

	addMessageWithArgument(setValueAtIndex);
	registerMessage(TT("poke"), (TTMethod)&TTBuffer::setValueAtIndex);

	// TODO: more messages to implement
	//	"readFile"   (requires libsndfile straightening-out)
	//	"writeFile"  (requires libsndfile straightening-out)
	//	a way to query attributes: for example what is the sr and bpm of an AIFF file?
}


TTBuffer::~TTBuffer()
{
	;
}


TTErr TTBuffer::setNumChannels(const TTValue& newNumChannels)
{
	TTValue v(mDimensions[0], TTUInt32(newNumChannels));
	
	return setDimensions(v);
}


TTErr TTBuffer::getNumChannels(TTValue& returnedChannelCount)
{
	returnedChannelCount = mDimensions[0];
	return kTTErrNone;
}


TTErr TTBuffer::setLength(const TTValue& newLength)
{
	TTValue v(TTFloat64(newLength) * mSampleRate * 0.001, mDimensions[1]);
	
	return setDimensions(v);
}


TTErr TTBuffer::getLength(TTValue& returnedLength)
{
	returnedLength = (mDimensions[1] / mSampleRate) * 1000.0;
	return kTTErrNone;
}


TTErr TTBuffer::setLengthInSamples(const TTValue& newLengthInSamples)
{
	TTValue v(TTUInt32(newLengthInSamples), mDimensions[1]);
	
	return setDimensions(v);
}


TTErr TTBuffer::getLengthInSamples(TTValue& returnedLengthInSamples)
{
	returnedLengthInSamples = mDimensions[1];
	return kTTErrNone;
}


TTErr TTBuffer::getValueAtIndex(TTValue& index)
{
	TTUInt32		sampleIndex;
	TTUInt16		sampleChannel = 0;
	TTSampleValue	sampleValue;
	TTUInt8			i = 0;
	TTErr			err;

	index.get(i++, sampleIndex);
	if (index.getSize() > 2)		// TODO: sure would be nice to change the name of this method to "size" or something...
		index.get(i++, sampleChannel);

	err = peek(sampleIndex, sampleChannel, sampleValue);
	if (!err)
		index.set(i++, sampleValue);
	return err;
}


TTErr TTBuffer::peek(const TTUInt64 index, const TTUInt16 channel, TTSampleValue& value)
{
	// TODO: perhaps we should range check the input here first...
	get2d(index, channel, value);
	return kTTErrNone;
}


/**	Set the sample value for a given index.
	The first number passed in the index parameter will be interpreted as the sample index.
	If there are three numbers passed, then the second number, if passed, will designate the channel index (defaults to zero).
	The final value will be used as the sample value that will be copied to the designated index.
*/
TTErr TTBuffer::setValueAtIndex(const TTValue& index)
{
	TTUInt32		sampleIndex;
	TTUInt16		sampleChannel = 0;
	TTSampleValue	sampleValue;
	TTUInt8			i = 0;

	index.get(i++, sampleIndex);
	if (index.getSize() > 2)
		index.get(i++, sampleChannel);
	index.get(i++, sampleValue);

	return poke(sampleIndex, sampleChannel, sampleValue);
}

TTErr TTBuffer::poke(const TTUInt64 index, const TTUInt16 channel, const TTSampleValue value)
{
	// TODO: perhaps we should range check the input here first...
	set2d(index, channel, value);
	return kTTErrNone;
}


#define mLengthInSamples mDimensions[0]
#define mNumChannels mDimensions[1]


TTErr TTBuffer::fill(const TTValue& value)
{
	TTSymbol*	fillAlgorithm = value;

	if (fillAlgorithm == kTTSym_sine) {
		for (TTUInt16 channel=0; channel<mNumChannels; channel++) {
			for (TTUInt64 i=0; i<mLengthInSamples; i++)
				set2d(i, channel, sin(kTTTwoPi * (i / (TTFloat64(mDimensions[0]) - 1.0))));
		}
	}
	else if (fillAlgorithm == kTTSym_sineMod) {							// (modulator version: ranges from 0.0 to 1.0, rather than -1.0 to 1.0)
		for (TTUInt16 channel=0; channel<mNumChannels; channel++) {
			for (TTUInt64 i=0; i<mLengthInSamples; i++)
				set2d(i, channel, 0.5 + (0.5 * sin(kTTTwoPi * (i / (TTFloat64(mDimensions[0]) - 1.0)))));
		}
	}
	else if (fillAlgorithm == kTTSym_cosine) {
		for (TTUInt16 channel=0; channel<mNumChannels; channel++) {
			for (TTUInt64 i=0; i<mLengthInSamples; i++)
				set2d(i, channel, cos(kTTTwoPi * (i / (TTFloat64(mDimensions[0]) - 1.0))));
		}
	}
	else if (fillAlgorithm == kTTSym_cosineMod) {
		for (TTUInt16 channel=0; channel<mNumChannels; channel++) {
			for (TTUInt64 i=0; i<mLengthInSamples; i++)
				set2d(i, channel, 0.5 + (0.5 * cos(kTTTwoPi * (i / (TTFloat64(mDimensions[0]) - 1.0)))));
		}
	}
	else if (fillAlgorithm == kTTSym_ramp) {
		for (TTUInt16 channel=0; channel<mNumChannels; channel++) {
			for (TTUInt64 i=0; i<mDimensions[0]; i++)
				set2d(i, channel, -1.0 + (2.0 * (float(i) / mDimensions[0])));
		}
	}
	else if (fillAlgorithm == kTTSym_rampMod) {
		for (TTUInt16 channel=0; channel<mNumChannels; channel++) {
			for (TTUInt64 i=0; i<mDimensions[0]; i++)
				set2d(i, channel, float(i) / mDimensions[0]);
		}
	}

	return kTTErrNone;
}

