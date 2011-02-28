/* 
 * TTCryBaby
 * Copyright © 2011, Nils Peters, ported from FAUST effect lib, see below for details
 *
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __TT_SMOOTHDELAY_H__
#define __TT_SMOOTHDELAY_H__

#include "TTDSP.h"

class TTSmoothDelay : TTAudioObject {
	TTCLASS_SETUP(TTSmoothDelay)

	TTFloat64			mInterpolation, mFeedback, mDelay, fSlow0, fSlow1, fSlow2, fSlow3, fSlow4, fSlow5, fSlow6, fConst0, fConst1;
	TTSampleVector		IOTA; // is there a TTSampleVector for ints?
	TTSampleVector		fRec11, fRec10;
	TTSampleVector		fRec01;
	TTSampleVector		fRec02;
	TTSampleVector		fRec00;
	TTSampleVector		fRec21;
	TTSampleVector		fRec20;
	TTSampleVector		fRec31;
	TTSampleVector		fRec30;
	TTSampleVector		fRec41;
	TTSampleVector		fRec40;
		
	/**	Receives notifications when there are changes to the inherited 
	 maxNumChannels attribute.  This allocates memory for the TTSampleVector variables 
	 so that each channel's previous values are remembered.		*/
	TTErr updateMaxNumChannels(const TTValue& oldMaxNumChannels);
	TTErr updateSampleRate(const TTValue& oldSampleRate);
	TTErr clear();
	
	void calculateCoefficients();
    /**	Standard single value calculate method as used by DSP objects. */
	inline TTErr calculateValue(const TTFloat64& input0, TTFloat64& output0, TTPtrSizedInt channel);
	
	/**	Standard audio processing method as used by TTBlue objects. */
	TTErr processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	
	/**	Setter for the frequency attribute. */
	TTErr setFeedback(const TTValue& value);
	TTErr setDelay(const TTValue& value);
	TTErr setInterpolation(const TTValue& value);
};


#endif // __TT_SMOOTHDELAY_H__
