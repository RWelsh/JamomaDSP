/* 
 * TTAudioMatrix Object
 * Copyright © 2010, Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTAudioMatrix.h"
#ifdef TT_PLATFORM_WIN
#include <algorithm>
#endif

#define thisTTClass			TTAudioMatrix
#define thisTTClassName		"audiomatrix"
#define thisTTClassTags		"audio, matrix"


TT_AUDIO_CONSTRUCTOR,
	mGainMatrix(NULL),
	mInterpolated(0),
	oldGainMatrix(NULL),
	tempGainMatrix(NULL),
	mNumInputs(1),
	mNumOutputs(1)
{
	TTObjectInstantiate(kTTSym_matrix, (TTObjectPtr*)&mGainMatrix, NULL);
	TTObjectInstantiate(kTTSym_matrix, (TTObjectPtr*)&oldGainMatrix, NULL);
	TTObjectInstantiate(kTTSym_matrix, (TTObjectPtr*)&tempGainMatrix, NULL);
	
	addAttributeWithSetter(NumInputs,	kTypeUInt16);
	addAttributeWithSetter(NumOutputs,	kTypeUInt16);
   	addAttribute(Interpolated,		kTypeBoolean);	
	
	addMessageWithArguments(setGain);
	addMessageWithArguments(setLinearGain);
	addMessageWithArguments(setMidiGain);

	//registerMessageWithArgument(updateMaxNumChannels);
	addMessage(clear);	
	mGainMatrix->setAttributeValue(TT("type"), TT("float64"));
	oldGainMatrix->setAttributeValue(TT("type"), TT("float64"));
	tempGainMatrix->setAttributeValue(TT("type"), TT("float64"));
	TTValue		v(1, 1);
	mGainMatrix->setAttributeValue(TT("dimensions"), v);
	oldGainMatrix->setAttributeValue(TT("dimensions"), v);
	tempGainMatrix->setAttributeValue(TT("dimensions"), v);
	clear();
	setProcessMethod(processAudio);
}


TTAudioMatrix::~TTAudioMatrix()
{
	TTObjectRelease((TTObjectPtr*)&mGainMatrix);
	TTObjectRelease((TTObjectPtr*)&oldGainMatrix);
	TTObjectRelease((TTObjectPtr*)&tempGainMatrix);
}


// conceptually:
//	columns == inputs
//	rows == outputs

/*TTErr TTAudioMatrix::setInterpolated(const TTValue& newValue)
{
	mInterpolated = newValue;
	if (mInterpolated)
		setProcessMethod(processAudioInterpolated);
	else 
		setProcessMethod(processAudio);	
	return kTTErrNone;
}*/



TTErr TTAudioMatrix::setNumInputs(const TTValue& newValue)
{
	TTUInt16	numInputs = newValue;
	TTValue		v(numInputs, mNumOutputs);
	
	if (numInputs != mNumInputs) {
		mNumInputs = numInputs;
		tempGainMatrix->adaptTo(mGainMatrix); //1. copy mGainMtrix to tempGainMatrix;
		TTMatrix::copy(*mGainMatrix, *tempGainMatrix);
		mGainMatrix->setAttributeValue(TT("dimensions"), v); //2. resize
		oldGainMatrix->setAttributeValue(TT("dimensions"), v);
		clear();						//3. clear mGainMatrix
		restoreMatrix(); //4. element-wise copy tempGainMatrix content over to mGainMatrix
	}
	return kTTErrNone;
}


TTErr TTAudioMatrix::setNumOutputs(const TTValue& newValue)
{
	TTUInt16	numOutputs = newValue;
	TTValue		v(mNumInputs, numOutputs);
	
	if (numOutputs != mNumOutputs) {
		mNumOutputs = numOutputs;
		tempGainMatrix->adaptTo(mGainMatrix); //1. copy mGainMtrix to tempGainMatrix;
		TTMatrix::copy(*mGainMatrix, *tempGainMatrix);
		mGainMatrix->setAttributeValue(TT("dimensions"), v);
		oldGainMatrix->setAttributeValue(TT("dimensions"), v);
		clear();						//3. clear mGainMatrix
		restoreMatrix(); //4. element-wise copy tempGainMatrix content over to mGainMatrix
	}
	return kTTErrNone;
}
	
TTErr TTAudioMatrix::restoreMatrix()
{
	TTValue dim;
	TTFloat64 tempValue; 	
	TTUInt16 xx, yy;
	tempGainMatrix->getDimensions(dim);
	dim.get(0,xx);
	dim.get(1,yy);
	TTLimit(xx,(TTUInt16) 1, mNumInputs); // in case xx or yy is greater than the current mGainMatrix ...
	TTLimit(yy,(TTUInt16) 1, mNumOutputs);
	for (TTUInt16 y=0; y < yy; y++) {
		for (TTUInt16 x=0; x < xx; x++) {
			tempGainMatrix->get2dZeroIndex(x, y, tempValue);
			mGainMatrix->set2dZeroIndex(   x, y, tempValue);
			oldGainMatrix->set2dZeroIndex( x, y, tempValue);
		}
	}
	return kTTErrNone;
}	


TTErr TTAudioMatrix::clear()
{
	mGainMatrix->clear();
	oldGainMatrix->clear();
	return kTTErrNone;
}

TTErr TTAudioMatrix::checkMatrixSize(TTUInt16 x, TTUInt16 y)
//this function will resize mGainMatrix if necessary while preserving its content 
{	
	if (x > (mNumInputs-1)){
		if (y > (mNumOutputs-1)) 
			mNumOutputs = y+1;
		setNumInputs(x+1); 
	}
	else{ 
		if (y > (mNumOutputs-1)) 			
			setNumOutputs(y+1);		
	}	
	return kTTErrNone;
}




TTErr TTAudioMatrix::setGain(const TTValue& newValue, TTValue&)
{
	TTUInt16	x;
	TTUInt16	y;
	TTFloat64	gainValue;
	
	if (newValue.getSize() != 3)
		return kTTErrWrongNumValues;
	
	newValue.get(0, x);
	newValue.get(1, y);
	newValue.get(2, gainValue);

	checkMatrixSize(x,y);
    
	mGainMatrix->set2dZeroIndex(x, y, dbToLinear(gainValue)); //the Matrix starts similar to Matlab with 1-index 
	if (mInterpolated) 
		setProcessMethod(processAudioInterpolated);
 	return kTTErrNone;
}


TTErr TTAudioMatrix::setLinearGain(const TTValue& newValue, TTValue&)
{
	TTUInt16	x;
	TTUInt16	y;
	TTFloat64	gainValue;
	
	if (newValue.getSize() != 3)
		return kTTErrWrongNumValues;
	
	newValue.get(0, x);
	newValue.get(1, y);
	newValue.get(2, gainValue);	

	checkMatrixSize(x,y);
	
	mGainMatrix->set2dZeroIndex(x, y, gainValue); 
	if (mInterpolated) 
		setProcessMethod(processAudioInterpolated);
	return kTTErrNone;	
}


TTErr TTAudioMatrix::setMidiGain(const TTValue& newValue, TTValue&)
{
	TTUInt16	x;
	TTUInt16	y;
	TTFloat64	gainValue;
	
	if (newValue.getSize() != 3)
		return kTTErrWrongNumValues;
	
	newValue.get(0, x);
	newValue.get(1, y);
	newValue.get(2, gainValue);

	checkMatrixSize(x,y);
	mGainMatrix->set2dZeroIndex(x, y, midiToLinearGain(gainValue)); //the Matrix starts similar to Matlab with 1-index 
	if (mInterpolated) 
		setProcessMethod(processAudioInterpolated);
	return kTTErrNone;
}


// Here we are mixing channels within a signal rather than between multiple signals
TTErr TTAudioMatrix::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	
	TTAudioSignal&		in = inputs->getSignal(0);
	TTAudioSignal&		out = outputs->getSignal(0);
	TTUInt16			vs = in.getVectorSizeAsInt();
	TTSampleValuePtr	inSample;
	TTSampleValuePtr	outSample;
	TTUInt16			numInputChannels = in.getNumChannelsAsInt();
	TTUInt16			numOutputChannels = out.getNumChannelsAsInt();
	TTUInt16			outChannel;
	TTUInt16			inChannel;
    TTSampleValue       gainValue;

	if (numInputChannels > mNumInputs) {
		numInputChannels = mNumInputs;
		//setNumInputs(numInputChannels);
	}
	if (numOutputChannels != mNumOutputs) {
		TTValue v = mNumOutputs;
		
		out.setMaxNumChannels(v);
		out.setNumChannels(v);
		numOutputChannels = mNumOutputs;
	}
	out.clear();
	
	// TODO: this multiply-nested for-loop has got to be horrendously slow, there should be a much faster way to do this?
	
		for (outChannel=0; outChannel<numOutputChannels; outChannel++) {
			outSample = out.mSampleVectors[outChannel];
			for (inChannel=0; inChannel<numInputChannels; inChannel++) {
				mGainMatrix->get2dZeroIndex(inChannel, outChannel, gainValue);
				if (gainValue != 0.0){
					inSample = in.mSampleVectors[inChannel];					
					for (int i=0; i<vs; i++) {				
						outSample[i] += inSample[i] * gainValue;
				}
			}
		}
	}		
	return kTTErrNone;
}


TTErr TTAudioMatrix::processAudioInterpolated(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	
	TTAudioSignal&		in = inputs->getSignal(0);
	TTAudioSignal&		out = outputs->getSignal(0);
	TTUInt16			vs = in.getVectorSizeAsInt();
	TTSampleValuePtr	inSample;
	TTSampleValuePtr	outSample;
	TTUInt16			numInputChannels = in.getNumChannelsAsInt();
	TTUInt16			numOutputChannels = out.getNumChannelsAsInt();
	TTUInt16			outChannel;
	TTUInt16			inChannel;
    TTSampleValue       gainValue, increment;
	
	if (numInputChannels > mNumInputs) {
		numInputChannels = mNumInputs;
		//setNumInputs(numInputChannels);
	}
	if (numOutputChannels != mNumOutputs) {
		TTValue v = mNumOutputs;
		
		out.setMaxNumChannels(v);
		out.setNumChannels(v);
		numOutputChannels = mNumOutputs;
	}
	
	out.clear();
	TTSampleValue value, oldValue; 
	// TODO: this multiply-nested for-loop has got to be horrendously slow, there should be a much faster way to do this?
	
	for (outChannel=0; outChannel<numOutputChannels; outChannel++) {
		outSample = out.mSampleVectors[outChannel];
		for (inChannel=0; inChannel<numInputChannels; inChannel++) {		
			mGainMatrix->get2dZeroIndex(inChannel, outChannel, value);
			if (value != 0.0){
				oldGainMatrix->get2dZeroIndex(inChannel, outChannel, oldValue);
				increment = (value-oldValue)/vs;
				TTAntiDenormal(increment);
				gainValue = oldValue;
				inSample = in.mSampleVectors[inChannel];				
				for (int i=0; i<vs; i++) {
					gainValue += increment;
					outSample[i] += inSample[i] * gainValue;
				}				
			}
			oldGainMatrix->set2dZeroIndex(inChannel, outChannel, value);
		}
	}	
	setProcessMethod(processAudio);
	return kTTErrNone;
}

