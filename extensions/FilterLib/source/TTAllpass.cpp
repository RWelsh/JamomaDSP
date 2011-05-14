/* 
 * Generalized Allpass Filter Wrapper for Jamoma DSP
 * Copyright © 2010, Tim Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTAllpass.h"

#define thisTTClass			TTAllpass
#define thisTTClassName		"allpass"
#define thisTTClassTags		"audio, processor, filter, allpass"

#ifdef TT_PLATFORM_WIN
#include <Algorithm>
#endif

TT_AUDIO_CONSTRUCTOR,
	mFilterObject(NULL)
{
	addAttributeWithSetter(Filter,	kTypeSymbol);
	
	addMessage(clear);
	addUpdate(SampleRate);
	addUpdate(MaxNumChannels);
	
	addMessageWithArgument(SetCoefficients);
	
	setAttributeValue(kTTSym_maxNumChannels, arguments);
	setAttributeValue(TT("filter"), TT("allpass.1a"));
	setProcessMethod(processAudio);
}


TTAllpass::~TTAllpass()
{
	delete mFilterObject;
}


TTErr TTAllpass::setFilter(const TTValue& filter)
{
	TTErr err;
	
	mFilter = filter;
	err = TTObjectInstantiate(mFilter, &mFilterObject, maxNumChannels);
	return err;
}


TTErr TTAllpass::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	if (mFilterObject)
		return mFilterObject->setAttributeValue(kTTSym_maxNumChannels, maxNumChannels);
	else
		return kTTErrNone;
}


TTErr TTAllpass::SetCoefficients(const TTValue& coefficients)
{
	TTErr err = kTTErrGeneric;
	
	if (mFilter == TT("allpass.1a") || mFilter == TT("allpass.1b")) {
		if (coefficients.getSize() == 1) {
			TTFloat64 alpha;
			
			coefficients.get(0, alpha);
			err = mFilterObject->setAttributeValue(TT("alpha"), maxNumChannels);
		}
	}
	else if (mFilter == TT("allpass.2a") || mFilter == TT("allpass.2b")) {
		mFilterObject->setAttributeValue(kTTSym_maxNumChannels, maxNumChannels);
		if (coefficients.getSize() == 2) {
			TTFloat64 c1, c2;
			
			coefficients.get(0, c1);
			coefficients.get(1, c2);
			err = mFilterObject->setAttributeValue(TT("c1"), c1);
			err = mFilterObject->setAttributeValue(TT("c2"), c2);
		}
	}
	else if (mFilter == TT("allpass.2c")) {
		mFilterObject->setAttributeValue(kTTSym_maxNumChannels, maxNumChannels);
		if (coefficients.getSize() == 2) {
			TTFloat64 e1, e2;
			
			coefficients.get(0, e1);
			coefficients.get(1, e2);
			err = mFilterObject->setAttributeValue(TT("e1"), e1);
			err = mFilterObject->setAttributeValue(TT("e2"), e2);
		}
	}
	else if (mFilter == TT("allpass.4a")) {
		mFilterObject->setAttributeValue(kTTSym_maxNumChannels, maxNumChannels);
		if (coefficients.getSize() == 4) {
			TTFloat64 d1, d2, d3, d4;
			
			coefficients.get(0, d1);
			coefficients.get(1, d2);
			coefficients.get(2, d3);
			coefficients.get(3, d4);
			err = mFilterObject->setAttributeValue(TT("d1"), d1);
			err = mFilterObject->setAttributeValue(TT("d2"), d2);
			err = mFilterObject->setAttributeValue(TT("d3"), d3);
			err = mFilterObject->setAttributeValue(TT("d4"), d4);
		}
	}
	
	return err;
}


TTErr TTAllpass::updateSampleRate(const TTValue& oldSampleRate)
{
	return mFilterObject->setAttributeValue(kTTSym_sampleRate, (uint)sr);
}


TTErr TTAllpass::clear()
{
	return mFilterObject->sendMessage(TT("clear"));
}


#if 0
#pragma mark -
#pragma mark dsp routines
#endif


inline TTErr TTAllpass::calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt data)
{	
	return mFilterObject->calculate(x, y);
}


TTErr TTAllpass::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	return mFilterObject->process(inputs, outputs);
}

