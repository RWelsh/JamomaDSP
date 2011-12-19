/* 
 * TTBlue 1st order Butterworth Highpass Filter Object
 * Copyright © 2008, Trond Lossius
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTHighpassButterworth1.h"

#define thisTTClass			TTHighpassButterworth1
#define thisTTClassName		"highpass.butterworth.1"
#define thisTTClassTags		"audio, processor, filter, highpass, butterworth"


TT_AUDIO_CONSTRUCTOR{
	// register attributes
	addAttributeWithSetter(Frequency,	kTypeFloat64);
	addAttributeProperty(Frequency,			range,			TTValue(10.0, sr*0.475));
	addAttributeProperty(Frequency,			rangeChecking,	TT("clip"));

	// register for notifications from the parent class so we can allocate memory as required
	addUpdates(MaxNumChannels);
	// register for notifications from the parent class so we can recalculate coefficients as required
	addUpdates(SampleRate);
	// make the clear method available to the outside world
	addMessage(clear);

	// Set Defaults...
	setAttributeValue(kTTSym_maxNumChannels,	arguments);			// This attribute is inherited
	setAttributeValue(TT("frequency"),		1000.0);
	setProcessMethod(processAudio);
	setCalculateMethod(calculateValue);
}


TTHighpassButterworth1::~TTHighpassButterworth1()
{
	;
}


TTErr TTHighpassButterworth1::updateMaxNumChannels(const TTValue& oldMaxNumChannels, TTValue&)
{
	mX1.resize(maxNumChannels);
	mY1.resize(maxNumChannels);	
	clear();
	return kTTErrNone;
}


TTErr TTHighpassButterworth1::updateSampleRate(const TTValue& oldSampleRate, TTValue&)
{
	TTValue	v(mFrequency);
	return setFrequency(v);
}


TTErr TTHighpassButterworth1::clear()
{
	mX1.assign(maxNumChannels, 0.0);
	mY1.assign(maxNumChannels, 0.0);
	return kTTErrNone;
}


TTErr TTHighpassButterworth1::setFrequency(const TTValue& newValue)
{	
	mFrequency = newValue;
	mRadians = kTTTwoPi*mFrequency;
	mK = mRadians/tan(kTTPi*mFrequency/sr);
	calculateCoefficients();
	return kTTErrNone;
}

	
	
void TTHighpassButterworth1::calculateCoefficients()
{
	mA0 = mK/(mK+mRadians); 
	mA1 = -mA0;//-k/(k+wc); 
	mB1 = (mRadians-mK)/(mK+mRadians);
}	


inline TTErr TTHighpassButterworth1::calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel)
{
	//y = TTAntiDenormal(mA0*x + mA1*mX1[channel] - mB1*mY1[channel]);
    //since mA1 = -mA0, we can simplyfiy to
	y = mA0*(x - mX1[channel]) - mB1*mY1[channel];
	TTZeroDenormal(y);
	mX1[channel] = x;
	mY1[channel] = y;
	return kTTErrNone;
}


TTErr TTHighpassButterworth1::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TT_WRAP_CALCULATE_METHOD(calculateValue);
}


