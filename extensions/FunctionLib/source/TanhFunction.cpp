/* 
 * TanhFunction Unit for TTBlue
 * Originally written for the Jamoma FunctionLib
 * Copyright © 2007 by Trond Lossius
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TanhFunction.h"
#include <math.h>

#define thisTTClass TanhFunction
#define thisTTClassName		"tanh"
#define thisTTClassTags		"audio, processor, function"


TT_AUDIO_CONSTRUCTOR
{
	// Register Attributes...
	addAttributeWithSetter(Offset,	kTypeFloat64);
	addAttributeWithSetter(Width,	kTypeFloat64);
		
	// Set Defaults...
	setAttributeValue(TT("offset"),	0.0);
	setAttributeValue(TT("width"), 0.5);
	
	setProcessMethod(processAudio);
	setCalculateMethod(calculateValue);
}


TanhFunction::~TanhFunction()
{
	;
}


TTErr TanhFunction::setOffset(const TTValue& newValue)
{
	mOffset = newValue;
	b = 0.5 * (mOffset+1);
	calculateOutputScaling();
	return kTTErrNone;
}


TTErr TanhFunction::setWidth(const TTValue& newValue)
{
	mWidth = newValue;
	if (mWidth <= 0)
		a = log(7.0);
	else 
		a = log(7.0)/mWidth;
	calculateOutputScaling();
	return kTTErrNone;
}


void TanhFunction::calculateOutputScaling(void)
{
	double f0, f1;
	
	f0 = tanh(a*(-b));
	f1 = tanh(a*(1-b));
	// This will never be division by zero, due to the fact that we always have width > 0
	alpha = 1/(f1-f0);
	beta = f0;
}


TTErr TanhFunction::calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt data)
{
	y = alpha * (tanh(a * (x-b)) - beta);
	return kTTErrNone;
}


TTErr TanhFunction::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TT_WRAP_CALCULATE_METHOD(calculateValue);
}

