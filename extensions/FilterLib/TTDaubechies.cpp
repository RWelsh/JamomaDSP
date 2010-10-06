/* 
 * A resampling FIR filter using coefficients from orthogonal wavelet sequences defined by Daubechies.
 * Copyright © 2010, Tim Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTDaubechies.h"

#define thisTTClass			TTDaubechies
#define thisTTClassName		"daubechies.3"
#define thisTTClassTags		"audio, processor, filter, lowpass, highpass, resampling, wavelet"


TT_AUDIO_CONSTRUCTOR,
	mX(NULL)	
{
	addAttributeWithSetter(Sequence, kTypeSymbol);		
	addUpdate(MaxNumChannels);

	setAttributeValue(kTTSym_maxNumChannels,	arguments);
	setAttributeValue(TT("sequence"),			4);			// default to D4 wavelet
	setProcessMethod(processAudio);
}


TTDaubechies::~TTDaubechies()
{
	delete[] mX;
}


TTErr TTDaubechies::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	if (TTUInt16(oldMaxNumChannels) != maxNumChannels) {
		delete[] mX;
		
		mX = new TTSampleVector[maxNumChannels];
		for (TTUInt16 channel=0; channel<maxNumChannels; channel++) {
			mX[channel].resize(mNumCoefficients);
			mX[channel].assign(mNumCoefficients, 0.0);
		}
	}
	return kTTErrNone;
}


void TTDaubechies::setNumCoefficients(TTUInt16 numCoefficients)
{
	mNumCoefficients = numCoefficients;
	mG.resize(mNumCoefficients);
	mH.resize(mNumCoefficients);
	memcpy(&mG[0], kCoefficients[(mNumCoefficients/2)-1], mG.size() * sizeof(TTSampleValue));
}


TTErr TTDaubechies::setSequence(const TTValue& newValue)
{
	mSequence = newValue;

	if (mSequence == TT("D2"))
		setNumCoefficients(2);
	else if (mSequence == TT("D4"))
		setNumCoefficients(4);
	else if (mSequence == TT("D6"))
		setNumCoefficients(4);
	else if (mSequence == TT("D8"))
		setNumCoefficients(4);
	else if (mSequence == TT("D10"))
		setNumCoefficients(4);
	else if (mSequence == TT("D12"))
		setNumCoefficients(4);
	else if (mSequence == TT("D14"))
		setNumCoefficients(4);
	else if (mSequence == TT("D16"))
		setNumCoefficients(4);
	else if (mSequence == TT("D18"))
		setNumCoefficients(4);
	else if (mSequence == TT("D20"))
		setNumCoefficients(4);
	else if (mSequence == TT("D22"))
		setNumCoefficients(4);
	else if (mSequence == TT("D24"))
		setNumCoefficients(4);
	else if (mSequence == TT("D26"))
		setNumCoefficients(4);
	else if (mSequence == TT("D28"))
		setNumCoefficients(4);
	else if (mSequence == TT("D30"))
		setNumCoefficients(4);
	else if (mSequence == TT("D32"))
		setNumCoefficients(4);
	else if (mSequence == TT("D34"))
		setNumCoefficients(4);
	else if (mSequence == TT("D36"))
		setNumCoefficients(4);
	else if (mSequence == TT("D38"))
		setNumCoefficients(4);
	else if (mSequence == TT("D40"))
		setNumCoefficients(4);
	else if (mSequence == TT("D42"))
		setNumCoefficients(4);
	else if (mSequence == TT("D44"))
		setNumCoefficients(4);
	else if (mSequence == TT("D46"))
		setNumCoefficients(4);
	else if (mSequence == TT("D48"))
		setNumCoefficients(4);
	else if (mSequence == TT("D50"))
		setNumCoefficients(4);
	else if (mSequence == TT("D52"))
		setNumCoefficients(4);
	else if (mSequence == TT("D54"))
		setNumCoefficients(4);
	else if (mSequence == TT("D56"))
		setNumCoefficients(4);
	else if (mSequence == TT("D58"))
		setNumCoefficients(4);
	else if (mSequence == TT("D60"))
		setNumCoefficients(4);
	else if (mSequence == TT("D62"))
		setNumCoefficients(4);
	else if (mSequence == TT("D64"))
		setNumCoefficients(4);
	else if (mSequence == TT("D68"))
		setNumCoefficients(4);
	else if (mSequence == TT("D70"))
		setNumCoefficients(4);
	else if (mSequence == TT("D72"))
		setNumCoefficients(4);
	else if (mSequence == TT("D74"))
		setNumCoefficients(4);
	else if (mSequence == TT("D76"))
		setNumCoefficients(4);
	else {
		logError("cannot set sequence -- invalid sequence specified (%s)", mSequence->getCString());
		return kTTErrInvalidValue;
	}
	return kTTErrNone;
}

/*
TTErr TTHalfband3::calculateLowpass(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel)
{
	TTFloat64 outputFromTopPath;
	TTFloat64 outputFromBottomPathDelay;
	
	mF0->calculateValue(x, outputFromTopPath, channel);
	mDelay->calculateValue(x, outputFromBottomPathDelay, channel);
	y = (outputFromTopPath + outputFromBottomPathDelay) * 0.5;
	return kTTErrNone;
}


TTErr TTHalfband3::calculateHighpass(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel)
{
	TTFloat64 outputFromTopPath;
	TTFloat64 outputFromBottomPathDelay;
	
	mF0->calculateValue(x, outputFromTopPath, channel);
	mDelay->calculateValue(x, outputFromBottomPathDelay, channel);
	y = (outputFromTopPath - outputFromBottomPathDelay) * 0.5;
	return kTTErrNone;
}


// for every second input sample, we calculate an output sample
// see TTHalfband5 for more details
TTErr TTHalfband3::calculateDownsample(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel)
{
	if (mX1[channel]) {
		TTFloat64 temp;

		mR0->calculateValue(x, temp, channel);
		y = (temp + mX1[channel]) * 0.5;
		mX1[channel] = 0;
	}
	else
		mX1[channel] = x;
	
	return kTTErrNone;
}


// for every second output sample, we calculate from a given input sample
TTErr TTHalfband3::calculateUpsample(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel)
{
	TTFloat64 temp;
	
	mDelay->calculateValue(x, temp, channel);
	if (mX1[channel]) {
		y = (temp + mX1[channel]) * 0.5;
		mX1[channel] = 0;
	}
	else {
		mR0->calculateValue(x, mX1[channel], channel);
		y = (temp + mX1[channel]) * 0.5;
	}
	return kTTErrNone;
}



TTErr TTHalfband3::processLowpass(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TT_WRAP_CALCULATE_METHOD(calculateLowpass);
}


TTErr TTHalfband3::processHighpass(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TT_WRAP_CALCULATE_METHOD(calculateHighpass);
}


TTErr TTHalfband3::processDownsample(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&	in = inputs->getSignal(0);
	TTAudioSignal&	out = outputs->getSignal(0);
	TTSampleValue*	inSample;
	TTSampleValue*	outSample;
	TTUInt16		numchannels = TTAudioSignal::getMinChannelCount(in, out);
	TTPtrSizedInt	channel;
	TTUInt16		targetVectorSize = in.getVectorSizeAsInt() / 2;
	//TTErr			err;
	
	out.changeVectorSize(targetVectorSize);
	out.setSampleRate(in.getSampleRate() / 2);
	for (channel=0; channel<numchannels; channel++) {
		TTUInt16 n = targetVectorSize;

		inSample = in.mSampleVectors[channel];
		outSample = out.mSampleVectors[channel];

		while (n--) {
			calculateDownsample(*inSample, *outSample, channel);
			inSample++;
			calculateDownsample(*inSample, *outSample, channel);
			inSample++;
			outSample++;
		}
	}
	return kTTErrNone;
}


TTErr TTHalfband3::processUpsample(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&	in = inputs->getSignal(0);
	TTAudioSignal&	out = outputs->getSignal(0);
	TTSampleValue*	inSample;
	TTSampleValue*	outSample;
	TTUInt16		numchannels = TTAudioSignal::getMinChannelCount(in, out);
	TTPtrSizedInt	channel;
	TTUInt16		targetVectorSize = in.getVectorSizeAsInt() * 2;
	TTErr			err;
	
	err = out.changeVectorSize(targetVectorSize);
	if (!err) {
		out.setSampleRate(in.getSampleRate() / 2);
		for (channel=0; channel<numchannels; channel++) {
			TTUInt16 n = in.getVectorSizeAsInt();
			
			inSample = in.mSampleVectors[channel];
			outSample = out.mSampleVectors[channel];
			
			while (n--) {
				calculateUpsample(*inSample, *outSample, channel);
				outSample++;
				calculateUpsample(*inSample, *outSample, channel);
				outSample++;
				inSample++;
			}
		}
	}
	return kTTErrNone;
}
*/

