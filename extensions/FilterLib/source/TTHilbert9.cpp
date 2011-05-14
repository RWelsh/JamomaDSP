/* 
 * 9th-order Hilbert Transform filter built up from a 2-path allpass structure
 * Copyright © 2010, Tim Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTHilbert9.h"

#define thisTTClass			TTHilbert9
#define thisTTClassName		"hilbert.9"
#define thisTTClassTags		"audio, processor, filter, hilbert"

#ifdef TT_PLATFORM_WIN
#include <Algorithm>
#endif

TT_AUDIO_CONSTRUCTOR,
	mF0(NULL),
	mF2(NULL),
	mF1(NULL),
	mF3(NULL),
	mDelay(NULL)
{
	TTUInt16	initialMaxNumChannels = arguments;
	TTErr		err;
	
	addMessage(clear);
	addUpdate(MaxNumChannels);

	err = TTObjectInstantiate(TT("allpass.1b"), (TTObjectPtr*)&mF0, initialMaxNumChannels);
	err = TTObjectInstantiate(TT("allpass.1b"), (TTObjectPtr*)&mF1, initialMaxNumChannels);
	err = TTObjectInstantiate(TT("allpass.1b"), (TTObjectPtr*)&mF2, initialMaxNumChannels);
	err = TTObjectInstantiate(TT("allpass.1b"), (TTObjectPtr*)&mF3, initialMaxNumChannels);
	err = TTObjectInstantiate(TT("allpass.1a"), (TTObjectPtr*)&mDelay, initialMaxNumChannels);

	setAttributeValue(kTTSym_maxNumChannels,	initialMaxNumChannels);
		
	// for the simple 1-sample delay, we set alpha (the feedback coefficient) to zero
	mDelay->setAttributeValue(TT("alpha"), 0.0);
	
	// These coefficients are sign-flipped copies from the TTHalfband9 filter
	mF0->setAttributeValue(TT("alpha"), -0.043646929608759);
	mF2->setAttributeValue(TT("alpha"), -0.399125646691078);
	
	mF1->setAttributeValue(TT("alpha"), -0.174628080915462);
	mF3->setAttributeValue(TT("alpha"), -0.749510679417446);
	
	setProcessMethod(processAudio);
}


TTHilbert9::~TTHilbert9()
{
	TTObjectRelease((TTObjectPtr*)&mF0);
	TTObjectRelease((TTObjectPtr*)&mF1);
	TTObjectRelease((TTObjectPtr*)&mF2);
	TTObjectRelease((TTObjectPtr*)&mF3);
	TTObjectRelease((TTObjectPtr*)&mDelay);
}


TTErr TTHilbert9::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	// TODO: update internal filters

	clear();
	return kTTErrNone;
}


TTErr TTHilbert9::clear()
{
	// TODO: update internal filters
	return kTTErrNone;
}


TTErr TTHilbert9::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&	in = inputs->getSignal(0);
	TTAudioSignal&	outReal = outputs->getSignal(0);
	TTAudioSignal&	outImaginary = outputs->getSignal(1);
	TTSampleValue*	inSample;
	TTSampleValue*	outRealSample;
	TTSampleValue*	outImaginarySample;
	TTUInt16		numChannels = TTAudioSignal::getMinChannelCount(in, outReal, outImaginary);
	TTPtrSizedInt	channel;
	TTFloat64		temp_0;
	TTFloat64		x;
	TTUInt16		n;
	TTFloat64		delayOutput;
		
	for (channel=0; channel<numChannels; channel++) {
		n = in.getVectorSizeAsInt();
		
		inSample = in.mSampleVectors[channel];
		outRealSample = outReal.mSampleVectors[channel];
		outImaginarySample = outImaginary.mSampleVectors[channel];
		
		while (n--) {
			x = *inSample++;

			mF0->calculateValue(x,				temp_0,					channel);
			mF2->calculateValue(temp_0,			*outRealSample++,		channel);

			mDelay->calculateValue(x,			delayOutput,			channel);
			mF1->calculateValue(delayOutput,	temp_0,					channel);
			mF3->calculateValue(temp_0,			*outImaginarySample++,	channel);
		}
	}
	return kTTErrNone;
}

