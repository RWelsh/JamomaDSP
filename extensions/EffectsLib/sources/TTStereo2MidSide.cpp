/* 
 * TTBlue Stereo to mid-side convertion object
 * Copyright © 2011, Trond Lossius
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTStereo2MidSide.h"

#define thisTTClass			TTStereo2MidSide
#define thisTTClassName		"stereo2MidSide"
#define thisTTClassTags		"audio, processor, stereo"


TT_AUDIO_CONSTRUCTOR
{
	setProcessMethod(processAudio);
}


TTStereo2MidSide::~TTStereo2MidSide()
{
	;
}



TTErr TTStereo2MidSide::updateMaxNumChannels(const TTValue& oldMaxNumChannels, TTValue&)
{
    // Two channels is the only option that makes sense for this unit
    maxNumChannels = 2;
    
	return kTTErrNone;
}



TTErr TTStereo2MidSide::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&	in = inputs->getSignal(0);
	TTAudioSignal&	out = outputs->getSignal(0);
	TTUInt16		vs;
	TTSampleValue	*left, *right, *mid, *side;
    TTSampleValue   *outSample;
	TTUInt16		numchannels = TTAudioSignal::getMinChannelCount(in, out);
	TTUInt16		channel;

    if (numchannels==2) {
		left  = in.mSampleVectors[0];
        right = in.mSampleVectors[1];
		mid   = out.mSampleVectors[0];
        side  = out.mSampleVectors[1];
		vs = in.getVectorSizeAsInt();
		while (vs--) {
			*mid++  = 0.5*(*left    + *right);
			*side++ = 0.5*(*right++ - *left++);
        }        
    }
    // If not 2 channels, then silence
    else {
		vs = in.getVectorSizeAsInt();
        for (channel=0; channel<numchannels; channel++) {            
            outSample = out.mSampleVectors[channel];
			memset(outSample,0,sizeof(TTSampleValue)*vs);
            /*while (vs--)
                *outSample++ = 0.;*/
        }
	}
	return kTTErrNone;
}