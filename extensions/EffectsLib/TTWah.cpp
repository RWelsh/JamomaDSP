/* 
 * TTBlue Fourth-order wah effect made using moog_vcf
 * Copyright © 2011, Nils Peters, ported from FAUST
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTWah.h"

#define thisTTClass			TTWah
#define thisTTClassName		"wah"
#define thisTTClassTags		"audio, processor, filter, wahwah"


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
	setAttributeValue(kTTSym_maxNumChannels, arguments);		// This attribute is inherited
	setAttributeValue(TT("frequency"),		200.0);
	setProcessMethod(processAudio);
	setCalculateMethod(calculateValue);
		
}


TTWah::~TTWah()
{
	;
}


TTErr TTWah::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	fRec10.resize(maxNumChannels);
	fRec11.resize(maxNumChannels);
	fRec01.resize(maxNumChannels);
	fRec00.resize(maxNumChannels);
	fRec21.resize(maxNumChannels);
	fRec20.resize(maxNumChannels);
	fRec31.resize(maxNumChannels);
	fRec30.resize(maxNumChannels);
	fRec41.resize(maxNumChannels);
	fRec40.resize(maxNumChannels);
	fRec51.resize(maxNumChannels);
	fRec50.resize(maxNumChannels);	
	clear();
	return kTTErrNone;
}


TTErr TTWah::updateSampleRate(const TTValue& oldSampleRate)
{
	TTValue	v(mFrequency);
	calculateCoefficients();
	return setFrequency(v);
}


TTErr TTWah::clear()
{
	fRec10.assign(maxNumChannels, mFrequency);
	fRec11.assign(maxNumChannels, mFrequency);	
	fRec01.assign(maxNumChannels, 0.0);
	fRec00.assign(maxNumChannels, 0.0);
	fRec21.assign(maxNumChannels, 0.0);
	fRec20.assign(maxNumChannels, 0.0);
	fRec31.assign(maxNumChannels, 0.0);
	fRec30.assign(maxNumChannels, 0.0);
	fRec41.assign(maxNumChannels, 0.0);
	fRec40.assign(maxNumChannels, 0.0);
	fRec51.assign(maxNumChannels, 0.0);
	fRec50.assign(maxNumChannels, 0.0);	
	return kTTErrNone;
}


TTErr TTWah::setFrequency(const TTValue& newValue)
{
	mFrequency = newValue;
	mFrequencyRad = mRadial * mFrequency;
	return kTTErrNone;
}


void TTWah::calculateCoefficients()
{
	mRadial = (kTTTwoPi / sr);
}	


inline TTErr TTWah::calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel)
{
	
	// control smoothing, this doesn't work surprisingly ...
	fRec10[channel] = (mFrequencyRad + (0.999 * fRec11[channel]));
	// audio processing
	TTFloat64 fTemp0 = mFrequencyRad; // should be TTFloat64 fTemp0 = fRec10[channel];
	TTFloat64 fTemp1 = (1.0 - fTemp0);
	//float fTemp2 = (float)input0[i];
	TTFloat64 fTemp2 = x;
	//fRec50[channel] = ((((iSlow1)?0:fTemp2) + (fTemp1 * fRec51[channel])) - (3.2 * fRec01[channel]));
	fRec50[channel] = (((fTemp2) + (fTemp1 * fRec51[channel])) - (3.2 * fRec01[channel]));
	fRec40[channel] = (fRec50[channel] + (fTemp1 * fRec41[channel]));
	fRec30[channel] = (fRec40[channel] + (fTemp1 * fRec31[channel]));
	fRec20[channel] = (fRec30[channel] + (fTemp1 * fRec21[channel]));
	fRec00[channel] = (fRec20[channel] * pow(fTemp0, 4.0));
	//output0[i] = (FAUSTFLOAT)((iSlow1)?fTemp2:(4 * fRec0[0]));
	y = (4 * fRec00[channel]);
	// post processing
	fRec01[channel] = fRec00[channel];
	fRec21[channel] = fRec20[channel];
	fRec31[channel] = fRec30[channel];
	fRec41[channel] = fRec40[channel];
	fRec51[channel] = fRec50[channel];
	fRec11[channel] = fRec10[channel];	
	return kTTErrNone;
}


TTErr TTWah::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TT_WRAP_CALCULATE_METHOD(calculateValue);
}
