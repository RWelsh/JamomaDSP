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

TTObjectPtr TTAudioSignal::instantiate(TTSymbolPtr name, TTValue& arguments)
{
	return new thisTTClass (arguments);
}


extern "C" void TTAudioSignal::registerClass() 
{
	TTClassRegister(TT(thisTTClassName), thisTTClassTags, thisTTClass :: instantiate);
}


TTAudioSignal::TTAudioSignal(TTValue& arguments) : 
	TTMatrix(arguments),
	mNumChannels(0),
	mSampleRate(0)
{
	TTUInt16	initialMaxNumChannels = arguments;
	
	this->setType(TT("float64"));
	this->setElementCount(1);
	
	addAttributeWithGetterAndSetter(VectorSize, kTypeUInt16);
	addAttributeWithGetterAndSetter(MaxNumChannels, kTypeUInt16);
	addAttributeWithSetter(			NumChannels, kTypeUInt16);
	
	addMessage(clear);
	
	addMessageWithArgument(setVector32);
	addMessageWithArgument(getVectorCopy32);
	addMessageWithArgument(setVector64);
	addMessageWithArgument(getVectorCopy64);
	
	addMessageProperty(setVector32,		hidden, YES);
	addMessageProperty(getVectorCopy32,	hidden, YES);
	addMessageProperty(setVector64,		hidden, YES);
	addMessageProperty(getVectorCopy64,	hidden, YES);
	
	setAttributeValue(kTTSym_maxNumChannels, initialMaxNumChannels);
	setAttributeValue(kTTSym_numChannels, initialMaxNumChannels);
}


TTAudioSignal::~TTAudioSignal()
{
	;
}


TTErr TTAudioSignal::setMaxNumChannelsWithInt(TTUInt16 newMaxNumChannels)
{
	TTValue v(mDimensions[0], newMaxNumChannels);
	return setDimensions(v);
}


TTErr TTAudioSignal::setMaxNumChannels(const TTValue& newMaxNumChannels)
{
	return setMaxNumChannelsWithInt(TTUInt16(newMaxNumChannels));
}


TTUInt16 TTAudioSignal::getMaxNumChannelsAsInt() const
{
	return mDimensions[1];
}


TTErr TTAudioSignal::getMaxNumChannels(TTValue& returnedMaxNumChannels)
{
	returnedMaxNumChannels = getMaxNumChannelsAsInt();
	return kTTErrNone;
}


TTErr TTAudioSignal::setVector(const TTUInt16 channel, const TTUInt16 newVectorSize, const TTSampleValuePtr newVector)
{
	setVectorSizeWithInt(newVectorSize);
	for (TTUInt32 i=1; i <= mDimensions[0]; i++)
		set2d(i, channel, newVector[i]);	
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


TTErr TTAudioSignal::setVector(const TTUInt16 channel, const TTUInt16 newVectorSize, const TTFloat32* newVector)
{
	setVectorSizeWithInt(newVectorSize);
	for (TTUInt32 i=1; i <= mDimensions[0]; i++)
		set2d(i, channel, newVector[i]);
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


TTErr TTAudioSignal::getVectorCopy(const TTUInt16 channel, const TTUInt16 theVectorSize, TTSampleValue* returnedVector)
{
	for (TTUInt16 i=1; i<=theVectorSize; i++)
		get2d(i, channel, returnedVector[i]);
	return kTTErrNone;
}


TTErr TTAudioSignal::getVectorCopy64(TTValue& v)
{
	TTUInt16		channel;
	TTUInt16		theVectorSize;
	TTPtr			returnedVector;
	
	if (v.getSize() == 3) {
		v.get(0, channel);
		v.get(1, theVectorSize);
		v.get(2, &returnedVector);
		return getVectorCopy(channel, theVectorSize, (TTSampleValue*)(returnedVector));
	}
	return kTTErrWrongNumValues;
}


TTErr TTAudioSignal::getVectorCopy(const TTUInt16 channel, const TTUInt16 theVectorSize, TTFloat32* returnedVector)
{
	for (TTUInt16 i=1; i<=theVectorSize; i++)
		get2d(i, channel, returnedVector[i]);
	return kTTErrNone;
}


TTErr TTAudioSignal::getVectorCopy32(TTValue& v)
{
	TTUInt16		channel;
	TTUInt16		theVectorSize;
	TTPtr			returnedVector;
	
	if (v.getSize() == 3) {
		v.get(0, channel);
		v.get(1, theVectorSize);
		v.get(2, &returnedVector);
		return getVectorCopy(channel, theVectorSize, (TTFloat32*)returnedVector);
	}
	return kTTErrWrongNumValues;
}


TTErr TTAudioSignal::copy(const TTAudioSignal& source, TTAudioSignal& dest, TTUInt16 channelOffset)
{
	TTUInt16		vs;
//	TTSampleValue	inSample;
//	TTSampleValue	outSample;
	TTSampleValue	sample;
	TTUInt16		maxDestChannels = dest.getMaxNumChannelsAsInt();
	TTUInt16		numchannels = TTAudioSignal::getMinChannelCount(source, dest);
	TTUInt16		additionalOutputChannels = dest.mNumChannels - numchannels;
	TTUInt16		channel;
	
	for (channel=0; channel<numchannels; channel++) {
		//inSample = source.mSampleVectors[channel];
		//outSample = dest.mSampleVectors[ TTClip(channel+channelOffset, 0, maxDestChannels-1)  ];
		vs = source.getVectorSizeAsInt();
		for (int i=1; i<=vs; i++) {
			//*outSample++ = *inSample++;
			source.get2d(i, channel, sample);
			dest.set2d(i, TTClip(channel+channelOffset, 0, maxDestChannels-1), sample);


//		//while (vs--)
//		//	*outSample++ = *inSample++;
//		
//		memcpy(outSample, inSample, sizeof(TTSampleValue) * vs);

		}
	}
	for (/*channel*/; channel<(numchannels+additionalOutputChannels-channelOffset); channel++) {
		//outSample = dest.mSampleVectors[channel];
		sample = 0.0;
		vs = dest.getVectorSizeAsInt();
		for (int i=1; i<=vs; i++)
			dest.set2d(i, channel, sample); //*outSample++ = 0.0;

//		memset(outSample, 0, sizeof(TTSampleValue) * vs);
//		//while (vs--)
//		//	*outSample++ = 0.0;

	}
	return kTTErrNone;
}


TTErr TTAudioSignal::copyDirty(const TTAudioSignal& source, TTAudioSignal& dest, TTUInt16 channelOffset)
{
	TTUInt16		vs;
	//	TTSampleValue	inSample;
	//	TTSampleValue	outSample;
	TTSampleValue	sample;
	TTUInt16		maxDestChannels = dest.getMaxNumChannelsAsInt();
	TTUInt16		numchannels = TTAudioSignal::getMinChannelCount(source, dest);
	TTUInt16		channel;
	
	for (channel=0; channel<numchannels; channel++) {
//		inSample = source.mSampleVectors[channel];
//		outSample = dest.mSampleVectors[ TTClip(channel+channelOffset, 0, maxDestChannels-1)  ];
		vs = source.getVectorSizeAsInt();
		for (int i=1; i<=vs; i++) {
			//*outSample++ = *inSample++;
			source.get2d(i, channel, sample);
			dest.set2d(i, TTClip(channel+channelOffset, 0, maxDestChannels-1), sample);
		}
	}
	return kTTErrNone;
}


TTErr TTAudioSignal::copySubset(const TTAudioSignal& source, TTAudioSignal& dest, TTUInt16 startingChannel, TTUInt16 endingChannel)
{
	TTUInt16		vs;
	//	TTSampleValue	inSample;
	//	TTSampleValue	outSample;
	TTSampleValue	sample;
	TTUInt16		sourceChannel;
	TTUInt16		destChannel;
	
	for (sourceChannel=startingChannel, destChannel=0; sourceChannel<=endingChannel; sourceChannel++, destChannel++) {
//		inSample = source.mSampleVectors[sourceChannel];
//		outSample = dest.mSampleVectors[destChannel];
		vs = source.getVectorSizeAsInt();
		for (int i=1; i<=vs; i++) {
//			*outSample++ = *inSample++;
			source.get2d(i, sourceChannel, sample);
			dest.set2d(i, destChannel, sample);
		}
	}
	return kTTErrNone;
}


TTUInt16 TTAudioSignal::getMinChannelCount(const TTAudioSignal& signal1, const TTAudioSignal& signal2)
{
	if (signal1.mNumChannels > signal2.mNumChannels)
		return signal2.mNumChannels;
	else
		return signal1.mNumChannels;
}


TTUInt16 TTAudioSignal::getMinChannelCount(const TTAudioSignal& signal1, const TTAudioSignal& signal2, const TTAudioSignal& signal3)
{
	TTUInt16	numChannels = signal1.mNumChannels;
	
	if (signal2.mNumChannels < numChannels)
		numChannels = signal2.mNumChannels;
	if (signal3.mNumChannels < numChannels)
		numChannels = signal3.mNumChannels;
	
	return numChannels;
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

