/* 
 * TTBlue Fourth-order CryBaby effect made using moog_vcf
 * Copyright © 2011, Nils Peters, ported from FAUST
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTSmoothDelay.h"

#define thisTTClass			TTSmoothDelay
#define thisTTClassName		"smoothDelay"
#define thisTTClassTags		"audio, processor, delay, faust"


TT_AUDIO_CONSTRUCTOR,
fConst0(1e+03 / sr),
fConst1(0.001 * sr),
mDelayMaxInSamples(524288)
{  		
	// register attributes
	addAttributeWithSetter(Delay,				kTypeFloat64);
	addAttributeWithSetter(Feedback,			kTypeFloat64);
	addAttributeWithSetter(Interpolation,		kTypeFloat64);
	// register for notifications from the parent class so we can allocate memory as required
	addUpdate(MaxNumChannels);
	// register for notifications from the parent class so we can recalculate coefficients as required
	addUpdate(SampleRate);
	// make the clear method available to the outside world
	addMessage(clear);

	// Set Defaults...
	setAttributeValue(kTTSym_maxNumChannels, arguments);		// This attribute is inherited
	setAttributeValue(TT("delay"),			0.0);
	setAttributeValue(TT("feedback"),		0.0);
	setAttributeValue(TT("interpolation"),	0.0);
	setProcessMethod(processAudio);
	setCalculateMethod(calculateValue);
		
}


TTSmoothDelay::~TTSmoothDelay()
{
	;
}

TTErr TTSmoothDelay::init(TTUInt64 newDelayMaxInSamples)
{
	// This is called every time that:
	// 1. maxNumChannels changes
	
	
	if (newDelayMaxInSamples) {
		mDelayMaxInSamples = newDelayMaxInSamples;
		//mDelayMax = mDelayMaxInSamples / srMill;
		
		for (TTDelayBufferIter buffer = mBuffers.begin(); buffer != mBuffers.end(); ++buffer) {
			buffer->resize(mDelayMaxInSamples);
			buffer->clear();
		}
		reset();
	}
	return kTTErrNone;
}

void TTDelay::reset()
{
	for (TTDelayBufferIter buffer = mBuffers.begin(); buffer != mBuffers.end(); ++buffer)
		buffer->setDelay(mDelayInSamples);
}

TTErr TTSmoothDelay::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	fRec00.resize(maxNumChannels);
	fRec01.resize(maxNumChannels);
	fRec02.resize(maxNumChannels);
	fRec10.resize(maxNumChannels);
	fRec11.resize(maxNumChannels);
	fRec21.resize(maxNumChannels);
	fRec20.resize(maxNumChannels);
	fRec31.resize(maxNumChannels);
	fRec30.resize(maxNumChannels);
	fRec41.resize(maxNumChannels);
	fRec40.resize(maxNumChannels);
	mBuffers.resize(maxNumChannels);
	
	clear();
	return init(mDelayMaxInSamples);
}


TTErr TTSmoothDelay::updateSampleRate(const TTValue& oldSampleRate)
{
	
	fConst0 = (1e+03 / sr);
	fConst1 = (0.001 * sr);
	calculateCoefficients();
	return kTTErrNone;
}


TTErr TTSmoothDelay::clear()
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
	fRec41.assign(maxNumChannels, 0.0);
	fRec40.assign(maxNumChannels, 0.0);
	for_each(mBuffers.begin(), mBuffers.end(), mem_fun_ref(&TTDelayBuffer::clear));
	
	return kTTErrNone;
}


TTErr TTSmoothDelay::setDelay(const TTValue& newValue)
{
	mDelay = newValue; //TODO: make sure value is in range 
	//fslider2 = mPosition;
	calculateCoefficients();
	return kTTErrNone;
}	

TTErr TTSmoothDelay::setFeedback(const TTValue& newValue)
{
	mFeedback = newValue;
	//fslider0 = mFeedback;
	calculateCoefficients();
	return kTTErrNone;
}

TTErr TTSmoothDelay::setInterpolation(const TTValue& newValue)
{
	mInterpolation = newValue;
	//fslider1 = mInterpolation;
	calculateCoefficients();
	return kTTErrNone;
}

void TTSmoothDelay::calculateCoefficients()
{
	 	fSlow0 = (0.01 * mFeedback);
	 	fSlow1 = (fConst0 / mInterpolation);
	 	fSlow2 = (0 - fSlow1);
	 	fSlow3 = (fConst1 * mDelay);
}

inline TTErr TTSmoothDelay::calculateValue(const TTFloat64& input0, TTFloat64& output0, TTPtrSizedInt channel)
{  	/*
	*buffer->mWritePointer++ = x;		// write the input into our buffer
	y = *buffer->mReadPointer++;		// fetch the output from our buffer
	*/
   	double fTemp0 = ((double)input0 + (fSlow0 * fRec01[channel]));
	buffer->mWritePointer++ = fTemp0;
	double fTemp1 = ((int((fRec11[channel] != 0.0)))?((int(((fRec21[channel] > 0.0) & (fRec21[channel] < 1.0))))?fRec11[channel]:0):((int(((fRec21[channel] == 0.0) & (fSlow3 != fRec31[channel]))))?fSlow1:((int(((fRec21[channel] == 1.0) & (fSlow3 != fRec41[channel]))))?fSlow2:0)));
	fRec10[channel] = fTemp1;
	fRec20[channel] = max(0.0, min(1.0, (fRec21[channel] + fTemp1)));
	fRec30[channel] = ((int(((fRec21[channel] >= 1.0) & (fRec41[channel] != fSlow3))))?fSlow3:fRec31[channel]);
	fRec40[channel] = ((int(((fRec21[channel] <= 0.0) & (fRec31[channel] != fSlow3))))?fSlow3:fRec41[channel]);
	fRec00[channel] = ((fRec20[channel] * mBuffers[(IOTA[channel]-int((int(fRec40[channel]) & 524287)))&524287]) + ((1.0 - fRec20[channel]) * mBuffers[(IOTA[channel]-int((int(fRec30[channel]) & 524287)))&524287]));
	output0 = fRec00[channel];
	// post processing
	fRec01[channel] = fRec00[channel];
	fRec41[channel] = fRec40[channel];
	fRec31[channel] = fRec30[channel];
	fRec21[channel] = fRec20[channel];
	fRec11[channel] = fRec10[channel];
	//IOTA[channel] = IOTA[channel]+1;
	return kTTErrNone;
}


TTErr TTSmoothDelay::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TT_WRAP_CALCULATE_METHOD(calculateValue);
}



/*
   
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
	int 	IOTA;
	double 	fVec0[524288];
	FAUSTFLOAT 	fslider1;
	double 	fConst0;
	FAUSTFLOAT 	fslider2;
	double 	fConst1;
	double 	fRec1[2];
	double 	fRec2[2];
	double 	fRec3[2];
	double 	fRec4[2];
	double 	fRec0[2];
public:
	static void metadata(Meta* m) 	{ 
	}
	
	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 1; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 0.0;
		IOTA = 0;
		for (int i=0; i<524288; i++) fVec0[i] = 0;
		fslider1 = 1e+01;
		fConst0 = (1e+03 / fSamplingFreq);
		fslider2 = 0.0;
		fConst1 = (0.001 * fSamplingFreq);
		for (int i=0; i<2; i++) fRec1[i] = 0;
		for (int i=0; i<2; i++) fRec2[i] = 0;
		for (int i=0; i<2; i++) fRec3[i] = 0;
		for (int i=0; i<2; i++) fRec4[i] = 0;
		for (int i=0; i<2; i++) fRec0[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("untitled.1.tmp");
		interface->declare(&fslider2, "style", "knob");
		interface->declare(&fslider2, "unit", "ms");
		interface->addHorizontalSlider("delay", &fslider2, 0.0, 0.0, 5e+03, 0.1);
		interface->declare(&fslider0, "style", "knob");
		interface->addHorizontalSlider("feedback", &fslider0, 0.0, 0.0, 1e+02, 0.1);
		interface->declare(&fslider1, "style", "knob");
		interface->declare(&fslider1, "unit", "ms");
		interface->addHorizontalSlider("interpolation", &fslider1, 1e+01, 1.0, 1e+02, 0.1);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		double 	fSlow0 = (0.01 * fslider0);
		double 	fSlow1 = (fConst0 / fslider1);
		double 	fSlow2 = (0 - fSlow1);
		double 	fSlow3 = (fConst1 * fslider2);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			double fTemp0 = ((double)input0[i] + (fSlow0 * fRec0[1]));
			fVec0[IOTA&524287] = fTemp0;
			double fTemp1 = ((int((fRec1[1] != 0.0)))?((int(((fRec2[1] > 0.0) & (fRec2[1] < 1.0))))?fRec1[1]:0):((int(((fRec2[1] == 0.0) & (fSlow3 != fRec3[1]))))?fSlow1:((int(((fRec2[1] == 1.0) & (fSlow3 != fRec4[1]))))?fSlow2:0)));
			fRec1[0] = fTemp1;
			fRec2[0] = max(0.0, min(1.0, (fRec2[1] + fTemp1)));
			fRec3[0] = ((int(((fRec2[1] >= 1.0) & (fRec4[1] != fSlow3))))?fSlow3:fRec3[1]);
			fRec4[0] = ((int(((fRec2[1] <= 0.0) & (fRec3[1] != fSlow3))))?fSlow3:fRec4[1]);
			fRec0[0] = ((fRec2[0] * fVec0[(IOTA-int((int(fRec4[0]) & 524287)))&524287]) + ((1.0 - fRec2[0]) * fVec0[(IOTA-int((int(fRec3[0]) & 524287)))&524287]));
			output0[i] = (FAUSTFLOAT)fRec0[0];
			// post processing
			fRec0[1] = fRec0[0];
			fRec4[1] = fRec4[0];
			fRec3[1] = fRec3[0];
			fRec2[1] = fRec2[0];
			fRec1[1] = fRec1[0];
			IOTA = IOTA+1;
		}
	}
}; 
 */

