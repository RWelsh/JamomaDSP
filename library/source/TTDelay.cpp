/*
 * Delay Unit for Jamoma DSP
 * Copyright © 2003, Timothy Place
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTDelay.h"

#define thisTTClass			TTDelay
#define thisTTClassName		"delay"
#define thisTTClassTags		"audio, processor, delay"

#ifndef TT_PLATFORM_MAC
#include <algorithm>
#endif

TT_AUDIO_CONSTRUCTOR,
	mDelay(0),
	mDelayInSamples(0),
	mDelayMax(0),
	mDelayMaxInSamples(0)
{
	TTUInt16	initialMaxNumChannels = arguments;

	// declare attributes
	addAttributeWithSetter(Delay,				kTypeFloat64);
	addAttributeWithSetter(DelayInSamples,		kTypeInt64);
	addAttributeWithSetter(DelayMax,			kTypeFloat64);
	addAttributeWithSetter(DelayMaxInSamples,	kTypeInt64);
	addAttributeWithSetter(Interpolation,		kTypeSymbol);

	// declare messages
	addMessage(clear);

	// updates from the parent class
	addUpdate(SampleRate);
	addUpdate(MaxNumChannels);

	// Set Defaults...
	setAttributeValue(kTTSym_maxNumChannels,	initialMaxNumChannels);
	setAttributeValue(TT("delayMaxInSamples"), 256);
	setAttributeValue(TT("delayInSamples"), 100);
	setAttributeValue(TT("interpolation"), TT("cubic"));
}


TTDelay::~TTDelay()
{
	;
}


// This is called every time that:
// 1. sr changes
// 2. maxNumChannels changes
// 3. maxNumSamples change
TTErr TTDelay::init(TTUInt64 newDelayMaxInSamples)
{
	if (newDelayMaxInSamples) {
		mDelayMaxInSamples = newDelayMaxInSamples;
		mDelayMax = mDelayMaxInSamples / srMill;

		for (TTDelayBufferIter buffer = mBuffers.begin(); buffer != mBuffers.end(); ++buffer) {
			buffer->resize(mDelayMaxInSamples);
			buffer->clear();
		}
		reset();
	}
	return kTTErrNone;
}


TTErr TTDelay::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	mBuffers.resize(maxNumChannels);
	return init(mDelayMaxInSamples);
}


TTErr TTDelay::updateSampleRate(const TTValue& oldSampleRate)
{
	init(long(srMill * mDelayMax));		// allocate a larger delay buffer if neccessary
	return setDelay(mDelay);			// hold the delay time in ms constant, despite the change of sr
}


TTErr TTDelay::clear()
{
	for_each(mBuffers.begin(), mBuffers.end(), mem_fun_ref(&TTDelayBuffer::clear));
	return kTTErrNone;
}


void TTDelay::reset()
{
	for (TTDelayBufferIter buffer = mBuffers.begin(); buffer != mBuffers.end(); ++buffer)
		buffer->setDelay(mDelayInSamples);
}


TTErr TTDelay::setDelay(const TTValue& newValue)
{
	mDelay = TTClip<TTFloat64>(newValue, 0.0, mDelayMax);
	mFractionalDelaySamples = mDelay * srMill;
	mDelayInSamples = mFractionalDelaySamples;
	mFractionalDelay = mFractionalDelaySamples - mDelayInSamples;

	reset();
	return kTTErrNone;
}


TTErr TTDelay::setDelayInSamples(const TTValue& newValue)
{
	mFractionalDelaySamples = TTClip<TTUInt64>(newValue, 0, mDelayMaxInSamples);
	mDelayInSamples = mFractionalDelaySamples;
	mFractionalDelay = mFractionalDelaySamples - mDelayInSamples;

	mDelay = mDelayInSamples * 1000.0 * srInv;

	reset();
	return kTTErrNone;
}


TTErr TTDelay::setDelayMax(const TTValue& newValue)
{
	mDelayMax = newValue;
	mDelayMaxInSamples = mDelayMax * srMill;
	init(mDelayMaxInSamples);
	return kTTErrNone;
}


TTErr TTDelay::setDelayMaxInSamples(const TTValue& newValue)
{
	mDelayMaxInSamples = newValue;
	mDelayMax = mDelayMaxInSamples * 1000.0 * srInv;
	init(mDelayMaxInSamples);
	return kTTErrNone;
}


// TODO: Update these when the new interpolation routines are written
TTErr TTDelay::setInterpolation(const TTValue& newValue)
{
	mInterpolation = newValue;

	if (mInterpolation == TT("none")) {
		setProcess((TTProcessMethod)&TTDelay::processAudioNoInterpolation);
	}
	else if (mInterpolation == TT("linear")) {
		setProcess((TTProcessMethod)&TTDelay::processAudioLinearInterpolation);
	}
	else if (mInterpolation == TT("cubic")) {
		setProcess((TTProcessMethod)&TTDelay::processAudioCubicInterpolation);
	}
	else {
		setProcess((TTProcessMethod)&TTDelay::processAudioLinearInterpolation);
		return kTTErrInvalidValue;
	}
	return kTTErrNone;
}


#if 0
#pragma mark -
#pragma mark dsp routines
#endif


#define TTDELAY_WRAP_CALCULATE_METHOD(methodName) \
	TTAudioSignal&		in = inputs->getSignal(0); \
	TTAudioSignal&		out = outputs->getSignal(0); \
	TTUInt16			vs; \
	TTSampleValue		inSample; \
	TTSampleValue		outSample; \
	TTUInt16			numchannels = TTAudioSignal::getMinChannelCount(in, out); \
	TTPtrSizedInt		channel; \
	TTDelayBufferPtr	buffer; \
	\
	for (channel=1; channel<=numchannels; channel++) { \
		vs = in.getVectorSizeAsInt(); \
		buffer = &mBuffers[channel]; \
		\
		for (int i=0; i<vs; i++) { \
			in.get2d(i, channel, inSample);\
			methodName (inSample, outSample, buffer); \
			out.set2d(i, channel, outSample); \
		} \
	}\
	return kTTErrNone;


inline TTErr TTDelay::calculateNoInterpolation(const TTFloat64& x, TTFloat64& y, TTDelayBufferPtr buffer)
{
	*buffer->mWritePointer++ = x;		// write the input into our buffer
	y = *buffer->mReadPointer++;		// fetch the output from our buffer

	// wrap the pointers in the buffer, if needed
	if (buffer->mWritePointer > buffer->tail())
		buffer->mWritePointer = buffer->head();
	if (buffer->mReadPointer > buffer->tail())
		buffer->mReadPointer = buffer->head();

	return kTTErrNone;
}

TTErr TTDelay::calculateNoInterpolation(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel)
{
	TTDelayBufferPtr buffer = &mBuffers[channel];
	return calculateNoInterpolation(x, y, buffer);
}


TTErr TTDelay::processAudioNoInterpolation(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTDELAY_WRAP_CALCULATE_METHOD(calculateNoInterpolation);
}


inline TTErr TTDelay::calculateLinearInterpolation(const TTFloat64& x, TTFloat64& y, TTDelayBufferPtr buffer)
{
	*buffer->mWritePointer = x;		// write the input into our buffer

	// move the record head
	buffer->mWritePointer++;
	if (buffer->mWritePointer > buffer->tail())
		buffer->mWritePointer = buffer->head();

	// move the play head
	buffer->mReadPointer++;
	if (buffer->mReadPointer > buffer->tail())
		buffer->mReadPointer = buffer->head();

	// store the value of the next sample in the buffer for interpolation
	TTSampleValuePtr next = buffer->mReadPointer + 1;
	next = buffer->wrapPointer(next);

	y = ((*next) * (1.0 - mFractionalDelay)) + ((*buffer->mReadPointer) * mFractionalDelay);
	return kTTErrNone;
}


TTErr TTDelay::processAudioLinearInterpolation(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTDELAY_WRAP_CALCULATE_METHOD(calculateLinearInterpolation);
}


// Four-point interpolation as described @ http://crca.ucsd.edu/~msp/techniques/latest/book-html/node114.html
// and http://crca.ucsd.edu/~msp/techniques/latest/book-html/node31.html#tab02.1
// similar to what is implemented in Pd's vd~ object
// note that in initial tests there appears to be slight signal boost
inline TTErr TTDelay::calculateCubicInterpolation(const TTFloat64& x, TTFloat64& y, TTDelayBufferPtr buffer)
{
	TTSampleValue	a, b, c, d;
	TTSampleValue	cMinusB;

	*buffer->mWritePointer = x;		// write the input into our buffer

	// move the record head
	buffer->mWritePointer++;
	if (buffer->mWritePointer > buffer->tail())
		buffer->mWritePointer = buffer->head();

	// move the play head
	buffer->mReadPointer++;
	if (buffer->mReadPointer > buffer->tail())
		buffer->mReadPointer = buffer->head();

	// store the value of the next sample in the buffer for interpolation
	a = *buffer->wrapPointer(buffer->mReadPointer + 1);
	b = *buffer->wrapPointer(buffer->mReadPointer + 0);
	c = *buffer->wrapPointer(buffer->mReadPointer - 1);
	d = *buffer->wrapPointer(buffer->mReadPointer - 2);
	cMinusB = c - b;

	y = b + mFractionalDelay * (cMinusB - 0.1666667 * (1.0 - mFractionalDelay) * ((d - a - (3.0 * cMinusB)) * mFractionalDelay + (d + (2.0 * a) - (3.0 * b))));
	return kTTErrNone;
}


TTErr TTDelay::processAudioCubicInterpolation(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTDELAY_WRAP_CALCULATE_METHOD(calculateCubicInterpolation);
}

