/* 
 * TTBlue Fourth-order CryBaby effect made using moog_vcf
 * Copyright © 2011, Nils Peters, ported from FAUST
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTCryBaby.h"

#define thisTTClass			TTCryBaby
#define thisTTClassName		"cryBaby"
#define thisTTClassTags		"audio, processor, filter, guitar, crybaby"


TT_AUDIO_CONSTRUCTOR,
fConst0(1413.7166941154069 / sr),
fConst1(2827.4333882308138 / sr)
{  		
	// register attributes
	addAttributeWithSetter(Position,		kTypeFloat64);
	addAttributeProperty(Position,			range,			TTValue(0.0, 1.0));
	addAttributeProperty(Position,			rangeChecking,	TT("clip"));

	// register for notifications from the parent class so we can allocate memory as required
	addUpdate(MaxNumChannels);
	// register for notifications from the parent class so we can recalculate coefficients as required
	addUpdate(SampleRate);
	// make the clear method available to the outside world
	addMessage(clear);

	// Set Defaults...
	setAttributeValue(kTTSym_maxNumChannels, arguments);		// This attribute is inherited
	setAttributeValue(TT("position"),		0.0);
	setProcessMethod(processAudio);
	setCalculateMethod(calculateValue);
		
}


TTCryBaby::~TTCryBaby()
{
	;
}


TTErr TTCryBaby::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	fRec00.assign(maxNumChannels, 0.0);
	fRec01.assign(maxNumChannels, 0.0);
	fRec02.assign(maxNumChannels, 0.0);
	fRec10.assign(maxNumChannels, 0.0);
	fRec11.assign(maxNumChannels, 0.0);
	fRec21.assign(maxNumChannels, 0.0);
	fRec20.assign(maxNumChannels, 0.0);
	fRec31.assign(maxNumChannels, 0.0);
	fRec30.assign(maxNumChannels, 0.0);
	clear();
	return kTTErrNone;
}


TTErr TTCryBaby::updateSampleRate(const TTValue& oldSampleRate)
{
	fConst0 = (1413.7166941154069 / sr);
	fConst1 = (2827.4333882308138 / sr);
	TTValue	v(mPosition);
	return setPosition(v);
}


TTErr TTCryBaby::clear()
{
	fRec00.assign(maxNumChannels, 0.0);
	fRec01.assign(maxNumChannels, 0.0);
	fRec02.assign(maxNumChannels, 0.0);
	fRec10.assign(maxNumChannels, 0.0);
	fRec11.assign(maxNumChannels, 0.0);
	fRec21.assign(maxNumChannels, 0.0);
	fRec20.assign(maxNumChannels, 0.0);
	fRec31.assign(maxNumChannels, 0.0);
	fRec30.assign(maxNumChannels, 0.0);
	return kTTErrNone;
}


TTErr TTCryBaby::setPosition(const TTValue& newValue)
{
	mPosition = newValue;
	fSlow0 = mPosition;
	calculateCoefficients();
	return kTTErrNone;
}	

void TTCryBaby::calculateCoefficients()
{
	fSlow1 = (0.0001000000000000001 * pow(4.0,fSlow0));
	fSlow3 = pow(2.0,(2.3 * fSlow0));
	fSlow4 = (1 - (fConst0 * (fSlow3 / pow(2.0,(1.0 + (2.0 * (1.0 - fSlow0)))))));
	fSlow5 = (0.0010000000000000009 * (0 - (2.0 * (cos((fConst1 * fSlow3)) * fSlow4))));
	fSlow6 = (0.0010000000000000009 * fSlow4 * fSlow4);
}

inline TTErr TTCryBaby::calculateValue(const TTFloat64& input0, TTFloat64& output0, TTPtrSizedInt channel)
{   // smoothing parameter (one-pole pole location)
	fRec10[channel] = (fSlow1 + (0.999 * fRec11[channel]));
	fRec20[channel] = (fSlow5 + (0.999 * fRec21[channel]));
	fRec30[channel] = (fSlow6 + (0.999 * fRec31[channel]));
    // up to this point, tis doens't have to be computed for each channel, because mFrequencyRad is the same
	
	// audio processing
		//fTemp0 = input0;
		fRec00[channel] = 0 - ((fRec30[channel] * fRec02[channel]) + (fRec20[channel] * fRec01[channel])) - (input0 * fRec10[channel]);
		output0 = fRec00[channel] - fRec01[channel];
		// post processing
		fRec02[channel] = fRec01[channel]; 
	    fRec01[channel] = fRec00[channel];
		fRec31[channel] = fRec30[channel];
		fRec21[channel] = fRec20[channel];
		fRec11[channel] = fRec10[channel];
	return kTTErrNone;
}


TTErr TTCryBaby::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
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
#include <cmath>
template <int N> inline float faustpower(float x) 		{ return powf(x,N); } 
template <int N> inline double faustpower(double x) 	{ return pow(x,N); }
template <int N> inline int faustpower(int x) 			{ return faustpower<N/2>(x) * faustpower<N-N/2>(x); } 
template <> 	 inline int faustpower<0>(int x) 		{ return 1; }
template <> 	 inline int faustpower<1>(int x) 		{ return x; }

class mydsp : public dsp{
private:
	FAUSTFLOAT 	fslider0;
	double 	fRec1[2];
	FAUSTFLOAT 	fcheckbox0;
	double 	fConst0;
	double 	fConst1;
	double 	fRec2[2];
	double 	fRec3[2];
	double 	fRec0[3];
public:
	static void metadata(Meta* m) 	{ 
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
		fslider0 = 0.8;
		for (int i=0; i<2; i++) fRec1[i] = 0;
		fcheckbox0 = 0.0;
		fConst0 = (1413.7166941154069 / fSamplingFreq);
		fConst1 = (2827.4333882308138 / fSamplingFreq);
		for (int i=0; i<2; i++) fRec2[i] = 0;
		for (int i=0; i<2; i++) fRec3[i] = 0;
		for (int i=0; i<3; i++) fRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openHorizontalBox("CRYBABY [tooltip: Reference: https://ccrma.stanford.edu/~jos/pasp/vegf.html]");
		interface->declare(&fcheckbox0, "0", "");
		interface->declare(&fcheckbox0, "tooltip", "When this is checked, the wah pedal has no effect");
		interface->addCheckButton("Bypass", &fcheckbox0);
		interface->declare(&fslider0, "1", "");
		interface->declare(&fslider0, "tooltip", "wah pedal angle between 0 (rocked back) and 1 (rocked forward)");
		interface->addHorizontalSlider("Wah", &fslider0, 0.8, 0.0, 1.0, 0.01);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		double 	fSlow0 = fslider0;
		double 	fSlow1 = (0.0001000000000000001 * pow(4.0,fSlow0));
		int 	iSlow2 = int(fcheckbox0);
		double 	fSlow3 = pow(2.0,(2.3 * fSlow0));
		double 	fSlow4 = (1 - (fConst0 * (fSlow3 / pow(2.0,(1.0 + (2.0 * (1.0 - fSlow0)))))));
		double 	fSlow5 = (0.0010000000000000009 * (0 - (2.0 * (cos((fConst1 * fSlow3)) * fSlow4))));
		double 	fSlow6 = (0.0010000000000000009 * faustpower<2>(fSlow4));
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			fRec1[0] = (fSlow1 + (0.999 * fRec1[1]));
			double fTemp0 = (double)input0[i];
			fRec2[0] = (fSlow5 + (0.999 * fRec2[1]));
			fRec3[0] = (fSlow6 + (0.999 * fRec3[1]));
			fRec0[0] = (0 - (((fRec3[0] * fRec0[2]) + (fRec2[0] * fRec0[1])) - (((iSlow2)?0:fTemp0) * fRec1[0])));
			output0[i] = (FAUSTFLOAT)((iSlow2)?fTemp0:(fRec0[0] - fRec0[1]));
			// post processing
			fRec0[2] = fRec0[1]; fRec0[1] = fRec0[0];
			fRec3[1] = fRec3[0];
			fRec2[1] = fRec2[0];
			fRec1[1] = fRec1[0];
		}
	}
};






 
 */

