/* 
 * TTDistortion: Cubic nonlinearity distortion
 * Copyright © 2011, Nils Peters, ported from FAUST, see reference below.
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTDistortion.h"

#define thisTTClass			TTDistortion
#define thisTTClassName		"distortion"
#define thisTTClassTags		"audio, processor, filter, distortion, faust"


TT_AUDIO_CONSTRUCTOR
{  		
	// register attributes
	addAttributeWithSetter(Drive,		kTypeFloat64);
	addAttributeWithSetter(Offset,	kTypeFloat64);
	// register for notifications from the parent class so we can allocate memory as required
	addUpdate(MaxNumChannels);
	// register for notifications from the parent class so we can recalculate coefficients as required
	// make the clear method available to the outside world
	addMessage(clear);

	// Set Defaults...
	setAttributeValue(kTTSym_maxNumChannels, arguments);		// This attribute is inherited
	setAttributeValue(TT("drive"),		0.0);
	setAttributeValue(TT("offset"),		0.0);
	setProcessMethod(processAudio);
	setCalculateMethod(calculateValue);
		
}


TTDistortion::~TTDistortion()
{
	;
}


TTErr TTDistortion::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	//fRec10.resize(maxNumChannels);
	//fRec11.resize(maxNumChannels);
	fRec01.resize(maxNumChannels);
	fRec00.resize(maxNumChannels);
	//fRec21.resize(maxNumChannels);
	//fRec20.resize(maxNumChannels);
	fVec01.resize(maxNumChannels);
	fVec00.resize(maxNumChannels);	
	clear();
	return kTTErrNone;
}


TTErr TTDistortion::clear()
{
	//fRec10.assign(maxNumChannels, 0.0);
	//fRec11.assign(maxNumChannels, 0.0);	
	fRec01.assign(maxNumChannels, 0.0);
	fRec00.assign(maxNumChannels, 0.0);
	//fRec21.assign(maxNumChannels, 0.0);
	//fRec20.assign(maxNumChannels, 0.0);
	fVec01.assign(maxNumChannels, 0.0);
	fVec00.assign(maxNumChannels, 0.0);
	
	return kTTErrNone;
}


TTErr TTDistortion::setOffset(const TTValue& newValue)
{
	mOffset = newValue;
	fSlow0 = mOffset; //0.0010000000000000009 * mHarmonics;
	return kTTErrNone;
}	

TTErr TTDistortion::setDrive(const TTValue& newValue)
{
	mDrive = newValue;
	fSlow1 =  2 * mDrive; //0.0010000000000000009 * mDrive;
	fSlow1 = pow(1e+01,fSlow1); // simplification, taking out of the DSP loop
	return kTTErrNone;
}

inline TTErr TTDistortion::calculateValue(const TTFloat64& input0, TTFloat64& output0, TTPtrSizedInt channel)
{	
	
	TTFloat64 fTemp1 = max(-1.0, min(1.0, (input0 * fSlow1 + fSlow0)));
	
	/* // if parameter smoothing is an issue, this has to be uncommented 
	 fRec10[channel] = (fSlow0 + (0.999 * fRec11[channel]));
	 fRec20[channel] = (fSlow1 + (0.999 * fRec21[channel]));
	 TTFloat64 fTemp1 = max(-1.0, min(1.0, ((input0 * pow(1e+01,(2 * fRec20[channel]))) + fRec10[channel]))); 
	 */
	
	TTFloat64 fTemp2 = fTemp1 * (1 - (0.3333333333333333 * fTemp1 * fTemp1));
	fVec00[channel] = fTemp2;
	fRec00[channel] = ((fVec00[channel] + (0.995 * fRec01[channel])) - fVec01[channel]);
	output0 = fRec00[channel];
	// post processing
	fRec01[channel] = fRec00[channel];
	fVec01[channel] = fVec00[channel];
	/* // postpr	o for smoothing 
	fRec21[channel] = fRec20[channel];
	fRec11[channel] = fRec10[channel];	
	 */
	return kTTErrNone;
}


TTErr TTDistortion::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
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
	FAUSTFLOAT 	fslider1;
	double 	fRec2[2];
	FAUSTFLOAT 	fcheckbox0;
	double 	fVec0[2];
	double 	fRec0[2];
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
		fslider0 = 0.0;
		for (int i=0; i<2; i++) fRec1[i] = 0;
		fslider1 = 0.0;
		for (int i=0; i<2; i++) fRec2[i] = 0;
		fcheckbox0 = 0.0;
		for (int i=0; i<2; i++) fVec0[i] = 0;
		for (int i=0; i<2; i++) fRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("CUBIC NONLINEARITY cubicnl          [tooltip: Reference:           https://ccrma.stanford.edu/~jos/pasp/Cubic_Soft_Clipper.html]");
		interface->openHorizontalBox("[1]");
		interface->declare(&fcheckbox0, "0", "");
		interface->declare(&fcheckbox0, "tooltip", "When this is checked, the nonlinearity has no effect");
		interface->addCheckButton("Bypass", &fcheckbox0);
		interface->declare(&fslider1, "1", "");
		interface->declare(&fslider1, "tooltip", "Amount of distortion");
		interface->addHorizontalSlider("Drive", &fslider1, 0.0, 0.0, 1.0, 0.01);
		interface->declare(&fslider0, "2", "");
		interface->declare(&fslider0, "tooltip", "Brings in even harmonics");
		interface->addHorizontalSlider("Offset", &fslider0, 0.0, 0.0, 1.0, 0.01);
		interface->closeBox();
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		double 	fSlow0 = (0.0010000000000000009 * fslider0);
		double 	fSlow1 = (0.0010000000000000009 * fslider1);
		int 	iSlow2 = int(fcheckbox0);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			fRec1[0] = (fSlow0 + (0.999 * fRec1[1]));
			fRec2[0] = (fSlow1 + (0.999 * fRec2[1]));
			double fTemp0 = (double)input0[i];
			double fTemp1 = max(-1, min(1, ((((iSlow2)?0:fTemp0) * pow(1e+01,(2 * fRec2[0]))) + fRec1[0])));
			double fTemp2 = (fTemp1 * (1 - (0.3333333333333333 * faustpower<2>(fTemp1))));
			fVec0[0] = fTemp2;
			fRec0[0] = ((fVec0[0] + (0.995 * fRec0[1])) - fVec0[1]);
			output0[i] = (FAUSTFLOAT)((iSlow2)?fTemp0:fRec0[0]);
			// post processing
			fRec0[1] = fRec0[0];
			fVec0[1] = fVec0[0];
			fRec2[1] = fRec2[0];
			fRec1[1] = fRec1[0];
		}
	}
};



 
 */

