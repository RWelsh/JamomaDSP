/* 
 * First-Order Allpass Filter Object for Jamoma DSP
 * Copyright © 2010, Tim Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTAllpass1a.h"

#define thisTTClass			TTAllpass1a
#define thisTTClassName		"allpass.1a"
#define thisTTClassTags		"audio, processor, filter, allpass"

#ifdef TT_PLATFORM_WIN
#include <Algorithm>
#endif

TT_AUDIO_CONSTRUCTOR,
	mAlpha(0)
{
	TTUInt16 initialMaxNumChannels = arguments;

	addAttribute(Alpha, kTypeFloat64);
	
	addMessage(clear);
	addUpdate(MaxNumChannels);

	setAttributeValue(kTTSym_maxNumChannels,	initialMaxNumChannels);
	setProcessMethod(processAudio);
}


TTAllpass1a::~TTAllpass1a()
{
	;
}


TTErr TTAllpass1a::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	mX1.resize(maxNumChannels);
	mY1.resize(maxNumChannels);
	clear();
	return kTTErrNone;
}


TTErr TTAllpass1a::clear()
{
	mX1.assign(maxNumChannels, 0.0);
	mY1.assign(maxNumChannels, 0.0);
	return kTTErrNone;
}


TTErr TTAllpass1a::calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel)
{
	y = mX1[channel] + mAlpha * (x - mY1[channel]);
	TTZeroDenormal(y);
	mX1[channel] = x;
	mY1[channel] = y;

	return kTTErrNone;
}


TTErr TTAllpass1a::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TT_WRAP_CALCULATE_METHOD(calculateValue);
}

