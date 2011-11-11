/* 
 * TTBlue Stereo to mid-side convertion object
 * Copyright © 2011, Trond Lossius
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTStereo2ms.h"

#define thisTTClass			TTStereo2ms
#define thisTTClassName		"stereo2ms"
#define thisTTClassTags		"audio, processor, stereo"


TT_AUDIO_CONSTRUCTOR
{
	setProcessMethod(processAudio);
}


TTStereo2ms::~TTStereo2ms()
{
	;
}


TTErr TTStereo2ms::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&	in = inputs->getSignal(0);
	TTAudioSignal&	out = outputs->getSignal(0);
	TTUInt16		vs;
	TTSampleValue	*inSample, *outSample;
	TTUInt16		numchannels = TTAudioSignal::getMinChannelCount(in, out);
	TTUInt16		channel;

	for (channel=0; channel<numchannels; channel++) {
		inSample = in.mSampleVectors[channel];
		outSample = out.mSampleVectors[channel];
		vs = in.getVectorSizeAsInt();
		while (vs--)
			*outSample++ = (*inSample++);
	}
	return kTTErrNone;
}