/* 
 * Kaiser Window Function Unit for Jamoma DSP
 * Copyright © 2010 by Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "KaiserWindow.h"


#define thisTTClass			KaiserWindow
#define thisTTClassName		"kaiser"
#define thisTTClassTags		"audio, processor, function, window"


TT_AUDIO_CONSTRUCTOR
{
	addAttributeWithSetter(Beta, kTypeFloat64);
	
	setAttributeValue(TT("beta"), 6.0);

	setProcessMethod(processAudio);
	setCalculateMethod(calculateValue);
}


KaiserWindow::~KaiserWindow()
{
	;
}


TTErr KaiserWindow::setBeta(const TTValue& newValue)
{
	mBeta = newValue;
	mBesselIOofBeta = TTBesselFunctionI0(mBeta);
	return kTTErrNone;
}


TTErr KaiserWindow::calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt data)
{
	TTFloat64	two_x = 2.0 * (x - 0.5); // x is shifted left by half a cycle so we get a mirror of the window around zero
	TTFloat64	temp = 1.0 - (two_x * two_x);
	TTFloat64	temp2 = sqrt(temp);
	TTFloat64	temp3 = mBeta * temp2;
	TTFloat64	temp4 = TTBesselFunctionI0(temp3);

	y = temp4 / mBesselIOofBeta;	
	return kTTErrNone;
}


TTErr KaiserWindow::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TT_WRAP_CALCULATE_METHOD(calculateValue);
}

