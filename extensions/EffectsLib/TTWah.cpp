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
	TTFloat64 fTemp2 = x;
	fRec50[channel] = (((fTemp2) + (fTemp1 * fRec51[channel])) - (3.2 * fRec01[channel]));
	fRec40[channel] = (fRec50[channel] + (fTemp1 * fRec41[channel]));
	fRec30[channel] = (fRec40[channel] + (fTemp1 * fRec31[channel]));
	fRec20[channel] = (fRec30[channel] + (fTemp1 * fRec21[channel]));
	fRec00[channel] = (fRec20[channel] * pow(fTemp0, 4.0));
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



/*
   
 The original Faust C-code
 
 //-----------------------------------------------------
 //
 // Code generated with Faust 0.9.30 (http://faust.grame.fr)
 //-----------------------------------------------------
 #ifndef FAUSTFLOAT
 #define FAUSTFLOAT float
 #endif  
 
 typedef long double quad;
 // link with  
#include <math.h>

class mydsp : public dsp{
private:
	FAUSTFLOAT 	fslider0;
	float 	fRec1[2];
	float 	fConst0;
	FAUSTFLOAT 	fcheckbox0;
	float 	fRec5[2];
	float 	fRec4[2];
	float 	fRec3[2];
	float 	fRec2[2];
	float 	fRec0[2];
public:
	static void metadata(Meta* m) 	{ 
		m->declare("thisTTClass", "TTWah");
		m->declare("effect.lib/name", "Faust Audio Effect Library");
		m->declare("effect.lib/author", "Julius O. Smith (jos at ccrma.stanford.edu)");
		m->declare("effect.lib/copyright", "Julius O. Smith III");
		m->declare("effect.lib/version", "1.31");
		m->declare("effect.lib/license", "STK-4.3");
		m->declare("effect.lib/reference", "https://ccrma.stanford.edu/realsimple/faust_strings/");
		m->declare("filter.lib/name", "Faust Filter Library");
		m->declare("filter.lib/author", "Julius O. Smith (jos at ccrma.stanford.edu)");
		m->declare("filter.lib/copyright", "Julius O. Smith III");
		m->declare("filter.lib/version", "1.28");
		m->declare("filter.lib/license", "STK-4.3");
		m->declare("filter.lib/reference", "https://ccrma.stanford.edu/~jos/filters/");
	}
	
	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 1; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 2e+02f;
		for (int i=0; i<2; i++) fRec1[i] = 0;
		fConst0 = (6.283185307179586f / fSamplingFreq);
		fcheckbox0 = 0.0;
		for (int i=0; i<2; i++) fRec5[i] = 0;
		for (int i=0; i<2; i++) fRec4[i] = 0;
		for (int i=0; i<2; i++) fRec3[i] = 0;
		for (int i=0; i<2; i++) fRec2[i] = 0;
		for (int i=0; i<2; i++) fRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openHorizontalBox("WAH4         [tooltip: Fourth-order wah effect made using moog_vcf]");
		interface->declare(&fcheckbox0, "0", "");
		interface->declare(&fcheckbox0, "tooltip", "When this is checked, the wah pedal has no effect");
		interface->addCheckButton("Bypass", &fcheckbox0);
		interface->declare(&fslider0, "1", "");
		interface->declare(&fslider0, "tooltip", "wah resonance frequency in Hz");
		interface->addHorizontalSlider("Resonance Frequency", &fslider0, 2e+02f, 1e+02f, 2e+03f, 1.0f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = (0.0010000000000000009f * fslider0);
		int 	iSlow1 = int(fcheckbox0);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			fRec1[0] = (fSlow0 + (0.999f * fRec1[1]));
			float fTemp0 = (fConst0 * fRec1[0]);
			float fTemp1 = (1.0f - fTemp0);
			float fTemp2 = (float)input0[i];
			fRec5[0] = ((((iSlow1)?0:fTemp2) + (fTemp1 * fRec5[1])) - (3.2f * fRec0[1]));
			fRec4[0] = (fRec5[0] + (fTemp1 * fRec4[1]));
			fRec3[0] = (fRec4[0] + (fTemp1 * fRec3[1]));
			fRec2[0] = (fRec3[0] + (fTemp1 * fRec2[1]));
			fRec0[0] = (fRec2[0] * powf(fTemp0,4.0f));
			output0[i] = (FAUSTFLOAT)((iSlow1)?fTemp2:(4 * fRec0[0]));
			// post processing
			fRec0[1] = fRec0[0];
			fRec2[1] = fRec2[0];
			fRec3[1] = fRec3[0];
			fRec4[1] = fRec4[0];
			fRec5[1] = fRec5[0];
			fRec1[1] = fRec1[0];
		}
	}
};
 
 
 */

