/*
 * TTBlue Zero-crossing detector and counter.
 * Copyright © 2008, Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTZerocross.h"

#define thisTTClass			TTZerocross
#define thisTTClassName		"zerocross"
#define thisTTClassTags		"audio, analysis, frequency"


TT_AUDIO_CONSTRUCTOR
{
	TTUInt16	initialMaxNumChannels = arguments;
	
	// Attributes
	addAttributeWithSetter(Size, kTypeUInt32);
	
	// Messages
	addMessage(Clear);
	addMessageWithArgument(updateMaxNumChannels);
	
	// Set Defaults
	setAttributeValue(TT("MaxNumChannels"),	initialMaxNumChannels);
	setAttributeValue(TT("Size"), 2000);
	setProcessMethod(processAudio);
	Clear();
}


TTZerocross::~TTZerocross()
{
	;
}


TTErr TTZerocross::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	return Clear();
}


TTErr TTZerocross::Clear()
{
	lastSampleWasOverZero = false;
	counter = 0;
	finalCount = 0;
	analysisLocation = 0;
	return kTTErrNone;
}


TTErr TTZerocross::setSize(const TTValue& value)
{
	mSize = value;
	rSize = 1.0 / mSize;
	return kTTErrNone;
}


// TODO: this unit requires 1 input and 2 outputs -- it does not yet configure itself for other arrangements!
TTErr TTZerocross::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&	in = inputs->getSignal(0);
	TTAudioSignal&	out = outputs->getSignal(0);
	TTUInt16		vs = in.getVectorSizeAsInt();
	TTSampleValue*	inSample;
	TTSampleValue*	out1Sample;
	TTSampleValue*	out2Sample;
	TTBoolean		thisSampleIsOverZero;
	TTBoolean		zeroxOccured;

	inSample = in.mSampleVectors[0];
	out1Sample = out.mSampleVectors[0];
	out2Sample = out.mSampleVectors[1];

	while (vs--) {
		thisSampleIsOverZero = (0 < (*inSample++));
		zeroxOccured = lastSampleWasOverZero != thisSampleIsOverZero;
		lastSampleWasOverZero = thisSampleIsOverZero;
		
		counter += zeroxOccured;
		analysisLocation++;
		
		if (analysisLocation >= mSize) {
			finalCount = ((sr * counter) * rSize) * srInv;
			analysisLocation = 0;
			counter = 0;
		}
		
		*out1Sample++ = finalCount;
		*out2Sample++ = zeroxOccured;
	}
		
	return kTTErrNone;
}

