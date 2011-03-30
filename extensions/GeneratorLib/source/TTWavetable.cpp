/*
 * TTBlue Wavetable Oscillator
 * Copyright © 2003, Timothy Place
 *
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "TTWavetable.h"

#define thisTTClass			TTWavetable
#define thisTTClassName		"wavetable"
#define thisTTClassTags		"audio, generator, oscillator, buffer"


TT_AUDIO_CONSTRUCTOR,
	mIndex(0.0),
	mIndexDelta(0.0),
	mWavetable(NULL)
{
	TTUInt16	initialMaxNumChannels = arguments;

	addAttributeWithSetter(Frequency,		kTypeFloat64);
	addAttributeWithSetter(Mode,			kTypeSymbol);
	addAttributeWithSetter(Gain,			kTypeFloat64);
	addAttributeWithSetter(Interpolation,	kTypeSymbol);
	addAttributeWithSetter(Size,			kTypeInt64);

	addUpdate(SampleRate);

	TTObjectInstantiate(TT("buffer"), (TTObjectPtr*)&mWavetable, kTTValNONE);
	if (!mWavetable)
		throw TTException("Could not create internal buffer object");
	mWavetable->setNumChannels(TTUInt32(1));

	// Set Defaults...
	setAttributeValue(TT("maxNumChannels"),	initialMaxNumChannels);
	setAttributeValue(TT("size"), 8192);
	setAttributeValue(TT("mode"), kTTSym_sine);
	setAttributeValue(TT("frequency"), 440.0);
	setAttributeValue(TT("gain"), 0.0);			// 0 dB
	setAttributeValue(TT("interpolation"), TT("linear"));
}


TTWavetable::~TTWavetable()
{
	TTObjectRelease((TTObject**)&mWavetable);
}


TTErr TTWavetable::updateSampleRate(const TTValue&)
{
	setAttributeValue(TT("frequency"), mFrequency);
	return kTTErrNone;
}


TTErr TTWavetable::setFrequency(const TTValue& newValue)
{
	TTValue	v;

	mFrequency = TTClip<TTFloat64>(newValue, 0.0, sr/2.0);
	mWavetable->getAttributeValue(TT("lengthInSamples"), v);
	mIndexDelta = mFrequency * TTFloat64(v) / sr;
	return kTTErrNone;
}


TTErr TTWavetable::setMode(const TTValue& newValue)
{
	mMode = newValue;	// TODO: should be newValue[0]

	if (mMode != TT("externalBuffer"))
		return mWavetable->fill(newValue);
	else {
		// TODO: implement the ability to use an externally defined buffer
		return kTTErrInvalidValue;
	}
}


TTErr TTWavetable::setInterpolation(const TTValue& newValue)
{
	mMode = newValue;
	if (mMode == TT("linear"))
		setProcessMethod(processWithLinearInterpolation);
	else if (mMode == TT("lfo"))
		setProcessMethod(processAsLFO);
	else
		setProcessMethod(processWithNoInterpolation);
	return kTTErrNone;
}


TTErr TTWavetable::setGain(const TTValue& newValue)
{
	mGain = newValue;
	mLinearGain = dbToLinear(mGain);
	return kTTErrNone;
}


TTErr TTWavetable::setSize(const TTValue& newSize)
{
	mSize = newSize;
	return mWavetable->setLengthInSamples(mSize);
}


// LFO mode only updates values once per vector in order to be as fast as possible
// LFO mode cannot be modulated
TTErr TTWavetable::processAsLFO(TTAudioSignalArrayPtr, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&	out = outputs->getSignal(0);
	TTSampleValue	tempSample;
	TTUInt16		vs = out.getVectorSizeAsInt();
	TTUInt16		i=0;
	TTUInt16		numChannels = out.getNumChannelsAsInt();
	TTUInt16		channel;
	TTUInt32		p1 = (TTUInt32)mIndex;						// playback index
	TTSampleValue*	contents = (TTSampleValue*)mWavetable->getLockedPointer();
	
	// Move the play head
	mIndex += (mIndexDelta * vs);
	
	// Wrap the play head
	if (mIndex >= mSize)
		mIndex -= mSize;
	else if (mIndex < 0)
		mIndex += mSize;
	
	// table lookup (no interpolation)
	tempSample = contents[p1] * mLinearGain;
	
	// TODO: in TTBlue 0.2.x this code only assigned the first sample value to save cpu -- should we bring this back as an option?
	while (vs--) {
		for (channel=0; channel<numChannels; channel++)
			out.mSampleVectors[channel][i] = tempSample;
		i++;
	}
	mWavetable->releaseLockedPointer();
	return kTTErrNone;
}


TTErr TTWavetable::processWithNoInterpolation(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignalPtr	in; // can't call getSignal if there is no signal! = inputs->getSignal(0);
	TTAudioSignal&		out = outputs->getSignal(0);
	TTSampleValue		*inSample = NULL;
	TTSampleValue		tempSample;
	TTUInt16			vs = out.getVectorSizeAsInt();
	TTUInt16			i=0;
	TTUInt16			numChannels = out.getNumChannelsAsInt();
	TTUInt16			channel;
	TTBoolean			hasModulation = true;
	TTUInt32			p1 = (TTUInt32)mIndex;						// playback index
	TTSampleValue*		contents = (TTSampleValue*)mWavetable->getLockedPointer();

	// If the input and output signals are the same, then there really isn't an input signal
	// In that case we don't modulate the oscillator with it
	if (in == &out)
		hasModulation = false;
	else {
		in = &inputs->getSignal(0);
		inSample = in->mSampleVectors[0];
	}
	while (vs--) {
		// Move the play head
		mIndex += mIndexDelta;

		// Apply modulation to the play head
		if (hasModulation)
			mIndex += *inSample++ * mSize / sr;

		// Wrap the play head
		if (mIndex >= mSize)
			mIndex -= mSize;
		else if (mIndex < 0)
			mIndex += mSize;

		// table lookup (no interpolation)
		tempSample = contents[p1] * mLinearGain;
		for (channel=0; channel<numChannels; channel++)
			out.mSampleVectors[channel][i] = tempSample;
		i++;
	}
	mWavetable->releaseLockedPointer();
	return kTTErrNone;
}


TTErr TTWavetable::processWithLinearInterpolation(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignalPtr	in; // can't call getSignal if there is no signal! = inputs->getSignal(0);
	TTAudioSignal&	out = outputs->getSignal(0);
	TTSampleValue	*inSample = NULL;
	TTSampleValue	tempSample;
	TTUInt16		vs = out.getVectorSizeAsInt();
	TTUInt16		i=0;
	TTUInt16		numChannels = out.getNumChannelsAsInt();
	TTUInt16		channel;
	TTBoolean		hasModulation = true;
	TTUInt32		p1, p2;									// two playback indices
	TTFloat64		diff;
	TTSampleValue*	contents = (TTSampleValue*)mWavetable->getLockedPointer();

	// If the input and output signals are the same, then there really isn't an input signal
	// In that case we don't modulate the oscillator with it
	if (inputs->numAudioSignals == 0)
		hasModulation = false;
	else {
		in = &inputs->getSignal(0);
		inSample = in->mSampleVectors[0];
	}
	while (vs--) {
		// Move the play head
		mIndex += mIndexDelta;

		// Apply modulation to the play head
		if (hasModulation)
			mIndex += *inSample++ * mSize / sr;

		// Wrap the play head
		if (mIndex >= mSize)
			mIndex -= mSize;
		else if (mIndex < 0)
			mIndex += mSize;

		// table lookup (linear interpolation)
		p1 = (TTUInt32)mIndex;
		p2 = p1 + 1;
		diff = mIndex - p1;
		p2 &= (mSize - 1);	// fast modulo

		tempSample = ((contents[p2] * diff) + (contents[p1] * (1.0 - diff))) * mLinearGain;
		for (channel=0; channel<numChannels; channel++)
			out.mSampleVectors[channel][i] = tempSample;
		i++;
	}
	mWavetable->releaseLockedPointer();
	return kTTErrNone;
}
