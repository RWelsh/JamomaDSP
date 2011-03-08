/* 
 * TTBlue 4th order Linkwitz Riley Highpass filter
 * Copyright © 2008, Trond Lossius
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTHighpassLinkwitzRiley4.h"

#define thisTTClass			TTHighpassLinkwitzRiley4
#define thisTTClassName		"highpass.linkwitzriley.4"
#define thisTTClassTags		"audio, processor, filter, highpass, crossover"


TT_AUDIO_CONSTRUCTOR
{
	// register attributes
	addAttributeWithSetter(Frequency,	kTypeFloat64);
	addAttributeProperty(Frequency,			range,			TTValue(10.0, sr*0.475));
	addAttributeProperty(Frequency,			rangeChecking,	TT("clip"));

	// register for notifications from the parent class so we can allocate memory as required
	addUpdate(MaxNumChannels);
	// register for notifications from the parent class so we can recalculate coefficients as required
	addUpdate(SampleRate);
	// make the clear method available to the outside world
	addMessage(clear);

	// Set Defaults...
	setAttributeValue(kTTSym_maxNumChannels,	arguments);			// This attribute is inherited
	setAttributeValue(TT("frequency"),		1000.0);
	setProcessMethod(processAudio);
	setCalculateMethod(calculateValue);

}


TTHighpassLinkwitzRiley4::~TTHighpassLinkwitzRiley4()
{
	;
}


TTErr TTHighpassLinkwitzRiley4::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	mX1.resize(maxNumChannels);
	mX2.resize(maxNumChannels);
	mX3.resize(maxNumChannels);
	mX4.resize(maxNumChannels);
	mY1.resize(maxNumChannels);
	mY2.resize(maxNumChannels);
	mY3.resize(maxNumChannels);
	mY4.resize(maxNumChannels);
	clear();
	return kTTErrNone;
}


TTErr TTHighpassLinkwitzRiley4::updateSampleRate(const TTValue& oldSampleRate)
{
	TTValue	v(mFrequency);
	return setFrequency(v);
}


TTErr TTHighpassLinkwitzRiley4::clear()
{	
	mX1.assign(maxNumChannels, 0.0);
	mX2.assign(maxNumChannels, 0.0);
	mX3.assign(maxNumChannels, 0.0);
	mX4.assign(maxNumChannels, 0.0);
	mY1.assign(maxNumChannels, 0.0);
	mY2.assign(maxNumChannels, 0.0);
	mY3.assign(maxNumChannels, 0.0);
	mY4.assign(maxNumChannels, 0.0);
	return kTTErrNone;
}


TTErr TTHighpassLinkwitzRiley4::setFrequency(const TTValue& newValue)
{
	mFrequency = newValue;
    
	mRadians = kTTTwoPi*mFrequency;
	mK =  mRadians/tan(kTTPi*mFrequency/sr); // kTTTwoPi*frequency/tan(kTTPi*frequency/sr);
    calculateCoefficients();
	return kTTErrNone;
}	
	
	
void TTHighpassLinkwitzRiley4::calculateCoefficients() //TODO: with a little bit of thinking, this can be optimized
{   
	TTFloat64 temp;
	temp = (4*pow(mRadians,2)*pow(mK,2)+kTTTwoSqrt2*pow(mRadians,3)*mK+pow(mK,4)+kTTTwoSqrt2*mRadians*pow(mK,3)+pow(mRadians,4));
	
	mA0 = pow(mK,4)/	temp;
	mA1 = -4*pow(mK,4)/	temp;
	mA2 = 6*pow(mK,4)/	temp;
	//mA3 = mA1;//mA3 = -4*pow(mK,4)/	temp;
	//mA4 = mA0; //mA4 = pow(mK,4)/	temp;			
																																
	mB1 = (4*pow(mRadians,4)+4*kTTSqrt2*pow(mRadians,3)*mK-4*pow(mK,4)-4*kTTSqrt2*mRadians*pow(mK,3))/							temp;
	mB2 = (6*pow(mRadians,4)-8*pow(mRadians,2)*pow(mK,2)+6*pow(mK,4))/															temp;
	mB3 = (-4*kTTSqrt2*pow(mRadians,3)*mK+4*pow(mRadians,4)+4*kTTSqrt2*mRadians*pow(mK,3)-4*pow(mK,4))/							temp;
	mB4 = (pow(mK,4)-kTTTwoSqrt2*pow(mRadians,3)*mK+pow(mRadians,4)-kTTTwoSqrt2*mRadians*pow(mK,3)+4*pow(mRadians,2)*pow(mK,2))/	temp;		
}


inline TTErr TTHighpassLinkwitzRiley4::calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel)
{
	//y = TTAntiDenormal(mA0*x + mA1*mX1[channel] + mA2*mX2[channel] + mA3*mX3[channel] + mA4*mX4[channel] - mB1*mY1[channel] - mB2*mY2[channel] -mB3*mY3[channel] - mB4*mY4[channel]);
    // since mA3 = mA1 and mA0 =  mA4, we can simplyfy to
	y = mA0*(x + mX4[channel]) + mA1*( mX1[channel] + mX3[channel] ) + mA2*mX2[channel] - mB1*mY1[channel] - mB2*mY2[channel] -mB3*mY3[channel] - mB4*mY4[channel];
	TTZeroDenormal(y);
	mX4[channel] = mX3[channel];
	mX3[channel] = mX2[channel];
	mX2[channel] = mX1[channel];
	mX1[channel] = x;
	mY4[channel] = mY3[channel];
	mY3[channel] = mY2[channel];
	mY2[channel] = mY1[channel];
	mY1[channel] = y;
	return kTTErrNone;
}


TTErr TTHighpassLinkwitzRiley4::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TT_WRAP_CALCULATE_METHOD(calculateValue);
}
