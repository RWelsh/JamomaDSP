/* 
 * TTBlue Limiter Object
 * Copyright © 2008, Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTLimiter.h"

#define thisTTClass			TTLimiter
#define thisTTClassName		"limiter"
#define thisTTClassTags		"audio, processor, dynamics, limiter"


TT_AUDIO_CONSTRUCTOR
, recover(0.0), lookaheadBufferIndex(0), lookaheadBuffer(NULL), gain(NULL), last(0.0)
, dcBlocker(NULL), preamp(NULL), maxBufferSize(512), attrMode(TT("exponential"))
{
	TTUInt16	initialMaxNumChannels = arguments;
	
	// register our attributes
	registerAttribute(TT("preamp"),		kTypeFloat64,	NULL,			(TTGetterMethod)&TTLimiter::getPreamp,		(TTSetterMethod)&TTLimiter::setPreamp);
	registerAttribute(TT("postamp"),	kTypeFloat64,	&attrPostamp,	(TTGetterMethod)&TTLimiter::getPostamp,		(TTSetterMethod)&TTLimiter::setPostamp);
	registerAttribute(TT("threshold"),	kTypeFloat64,	&attrThreshold,	(TTGetterMethod)&TTLimiter::getThreshold,	(TTSetterMethod)&TTLimiter::setThreshold);
	registerAttribute(TT("lookahead"),	kTypeUInt32,	&attrLookahead, (TTSetterMethod)&TTLimiter::setLookahead);
	registerAttribute(TT("release"),	kTypeFloat64,	&attrRelease,	(TTSetterMethod)&TTLimiter::setRelease);
	registerAttribute(TT("mode"),		kTypeSymbol,	&attrMode,		(TTSetterMethod)&TTLimiter::setMode);
	registerAttribute(TT("dcBlocker"),	kTypeBoolean,	&attrDCBlocker,	(TTSetterMethod)&TTLimiter::setDCBlocker);

	// register for notifications from the parent class so we can allocate memory as required
	addUpdates(MaxNumChannels);
	// register for notifications from the parent class so we can update the release/recover values
	addUpdates(SampleRate);

	// clear the history
	addMessage(clear);

	TTObjectInstantiate(kTTSym_dcblock, &dcBlocker, initialMaxNumChannels);
	TTObjectInstantiate(kTTSym_gain, &preamp, initialMaxNumChannels);

	// Set Defaults...
	setAttributeValue(kTTSym_maxNumChannels,	initialMaxNumChannels);
	setAttributeValue(TT("threshold"),		0.0);
	setAttributeValue(TT("preamp"),			0.0);
	setAttributeValue(TT("postamp"),		0.0);
	setAttributeValue(TT("lookahead"),		100);
	setAttributeValue(TT("mode"),			TT("exponential"));
	setAttributeValue(TT("release"),		1000.0);
	setAttributeValue(TT("dcBlocker"),		kTTBoolYes);
	setAttributeValue(TT("bypass"),			kTTBoolNo);

	clear();
	setProcessMethod(processAudio);
}


TTLimiter::~TTLimiter()
{
	short i;
	
	for (i=0; i<maxNumChannels; i++)
		delete [] lookaheadBuffer[i];
	delete [] lookaheadBuffer;
	delete [] gain;
	TTObjectRelease(&dcBlocker);
	TTObjectRelease(&preamp);
}


// TODO: These message receiver args should be reversed -- this is a change that should be applied throughout TTBlue
TTErr TTLimiter::updateMaxNumChannels(const TTValue& oldMaxNumChannels, TTValue&)
{
	TTUInt16	channel;
	TTUInt16	numChannels = oldMaxNumChannels;

	if (lookaheadBuffer) {
		for (channel=0; channel<numChannels; channel++)
			delete [] lookaheadBuffer[channel];
		delete [] lookaheadBuffer;
	}
	delete gain;

	gain = new TTSampleValue[maxBufferSize];
	lookaheadBuffer = new TTSampleValuePtr[maxNumChannels];
	for (channel=0; channel<maxNumChannels; channel++)
		lookaheadBuffer[channel] = new TTSampleValue[maxBufferSize];

	clear();
	
	dcBlocker->setAttributeValue(kTTSym_maxNumChannels, maxNumChannels);
	preamp->setAttributeValue(kTTSym_maxNumChannels, maxNumChannels);
	
	return kTTErrNone;
}


TTErr TTLimiter::updateSampleRate(const TTValue& oldSampleRate, TTValue&)
{
	setRecover();
	return kTTErrNone;
}


TTErr TTLimiter::setPreamp(const TTValue& newValue)
{
	return preamp->setAttributeValue(TT("gain"), const_cast<TTValue&>(newValue));
}

TTErr TTLimiter::getPreamp(TTValue& value)
{
	return preamp->getAttributeValue(TT("gain"), value);
}


TTErr TTLimiter::setPostamp(const TTValue& newValue)
{
	attrPostamp = dbToLinear(newValue);
	return kTTErrNone;
}

TTErr TTLimiter::getPostamp(TTValue& value)
{
	value = linearToDb(attrPostamp);
	return kTTErrNone;
}


TTErr TTLimiter::setThreshold(const TTValue& newValue)
{
	attrThreshold = dbToLinear(newValue);
	return kTTErrNone;
}

TTErr TTLimiter::getThreshold(TTValue& value)
{
	value = linearToDb(attrThreshold);
	return kTTErrNone;
}


TTErr TTLimiter::setLookahead(TTValue& newValue)
{
	attrLookahead = TTClip(TTUInt32(newValue), TTUInt32(1), maxBufferSize-1);
    lookaheadInv = 1.0 / TTFloat64(attrLookahead);
	return kTTErrNone;
}


TTErr TTLimiter::setRelease(TTValue& newValue)
{
	attrRelease = newValue;
	setRecover();
	return kTTErrNone;
}


TTErr TTLimiter::setMode(TTValue& newValue)
{
	attrMode = newValue;
	if (attrMode == TT("linear"))
		isLinear = true;
	else
		isLinear = false;
	setRecover();
	return kTTErrNone;
}


TTErr TTLimiter::setDCBlocker(TTValue& newValue)
{
	attrDCBlocker = newValue;
	return dcBlocker->setAttributeValue(TT("bypass"), !attrDCBlocker);
}


TTErr TTLimiter::clear()
{
	TTUInt32	i;
	TTUInt32	channel;
	
	for (i=0; i<maxBufferSize; i++) {
		for (channel=0; channel < maxNumChannels; channel++)
    		lookaheadBuffer[channel][i] = 0.0;
    	gain[i] = 1.0;		// gain is shared across channels
    }

	lookaheadBufferIndex = 0;	// was bp
	last = 1.0;
	setRecover();

	dcBlocker->sendMessage(TT("clear"));
	return kTTErrNone;
}


// set variables related to the time it takes for the limiter to recover from a peak in the audio
// it is based on the release time attr, which is specified in seconds
void TTLimiter::setRecover()
{
	recover = 1000.0 / (attrRelease * sr);		
	if (attrMode == TT("linear"))
		recover = recover * 0.5;
	else 
		recover = recover * 0.707;
}


TTErr TTLimiter::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&	in = inputs->getSignal(0);
	TTAudioSignal&	out = outputs->getSignal(0);
	TTUInt16		vs = in.getVectorSizeAsInt();
	TTUInt16		i, j;
	TTSampleValue	tempSample;
	TTSampleValue	hotSample;
	TTFloat64		maybe,
					curgain,
					newgain,
					inc,
					acc;
	TTInt16			flag,
					lookaheadBufferPlayback,
					ind;
	TTUInt16		numchannels = TTAudioSignal::getMinChannelCount(in, out);
	TTUInt16		channel;

	// Pre-Process the input
	dcBlocker->process(in, out);	// filter out DC-Offsets (unless it is bypassed)
	preamp->process(out, in);		// copy output back to input for the rest of this process

	for (i=1; i<=vs; i++) {
		hotSample = 0.0;
	
		// Analysis Stage ...
		for (channel=1; channel<=numchannels; channel++) {
//			tempSample = in.mSampleVectors[channel][i];
			in.get2d(i, channel, tempSample);
			lookaheadBuffer[channel][lookaheadBufferIndex] = tempSample * attrPostamp;
			tempSample = fabs(tempSample);
			if (tempSample > hotSample)
				hotSample = tempSample;
		}
			
		if (isLinear)
			tempSample = last + recover;
		else {
			if (last > 0.01)
				tempSample = last + recover * last;
			else
				tempSample = last + recover;
		}

		if (tempSample > attrThreshold)
			maybe = attrThreshold;
		else
			maybe = tempSample;
		gain[lookaheadBufferIndex] = maybe;
	
		lookaheadBufferPlayback = lookaheadBufferIndex - attrLookahead;
		if (lookaheadBufferPlayback < 0)
			lookaheadBufferPlayback += attrLookahead;		
		
		// Process Stage ...
		// this ***very slow*** : with a lookahead of 100, and vs = 64, we loop 640 times!
		if (hotSample * maybe > attrThreshold) {
			curgain = attrThreshold / hotSample;
			inc = (attrThreshold - curgain);
			acc = 0;
			flag = 0;
			for (j=0; flag==0 && j<attrLookahead; j++) {
				ind = lookaheadBufferIndex - j;
				if (ind<0)
					ind += maxBufferSize;
					
				if (isLinear) //TODO: can't we move this condition outside the loop? isLinear won't change during a vs [NP]
					newgain = curgain + inc * acc;
				else
					newgain = curgain + inc * (acc * acc);
					
				if (newgain < gain[ind])
					gain[ind] = newgain;
				else
					flag = 1;
				acc = acc + lookaheadInv;
			}
		}
		
		// Actual application of the gain
		for (channel=1; channel<=numchannels; channel++) {
			TTSampleValue s = lookaheadBuffer[channel][lookaheadBufferPlayback] * gain[lookaheadBufferPlayback];
			out.set2d(i, channel, s);
		}
		
		last = gain[lookaheadBufferIndex];
		lookaheadBufferIndex++;
		if (lookaheadBufferIndex >= attrLookahead)
			lookaheadBufferIndex = 0;		
	}
	return kTTErrNone;
}
