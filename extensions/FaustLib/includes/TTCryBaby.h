/* 
 * TTCryBaby
 * Copyright © 2011, Nils Peters, ported from FAUST effect lib, see below for details
 *
 *   m->declare("effect.lib/name", "Faust Audio Effect Library");
 *   m->declare("effect.lib/author", "Julius O. Smith (jos at ccrma.stanford.edu)");
 *   m->declare("effect.lib/copyright", "Julius O. Smith III");
 *   m->declare("effect.lib/version", "1.31");
 *   m->declare("effect.lib/license", "STK-4.3");
 *   m->declare("effect.lib/reference", "https://ccrma.stanford.edu/realsimple/faust_strings/");
 *   m->declare("filter.lib/name", "Faust Filter Library");
 *   m->declare("filter.lib/author", "Julius O. Smith (jos at ccrma.stanford.edu)");
 *   m->declare("filter.lib/copyright", "Julius O. Smith III");
 *   m->declare("filter.lib/version", "1.28");
 *   m->declare("filter.lib/license", "STK-4.3");
 *   m->declare("filter.lib/reference", "https://ccrma.stanford.edu/~jos/filters/");
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __TT_CRYBABY_H__
#define __TT_CRYBABY_H__

#include "TTDSP.h"

class TTCryBaby : TTAudioObject {
	TTCLASS_SETUP(TTCryBaby)

	TTFloat64			mPosition, fSlow0, fSlow1, fSlow3, fSlow4, fSlow5, fSlow6, fConst0, fConst1;
	TTSampleVector		fRec11, fRec10;
	TTSampleVector		fRec01;
	TTSampleVector		fRec02;
	TTSampleVector		fRec00;
	TTSampleVector		fRec21;
	TTSampleVector		fRec20;
	TTSampleVector		fRec31;
	TTSampleVector		fRec30;
		
	/**	Receives notifications when there are changes to the inherited 
	 maxNumChannels attribute.  This allocates memory for xm1, xm2, ym1, and ym2 
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
	TTErr setPosition(const TTValue& value);
	
	/**	This algorithm uses an IIR filter, meaning that it relies on feedback.  If the filter should
	 *	not be producing any signal (such as turning audio off and then back on in a host) or if the
	 *	feedback has become corrupted (such as might happen if a NaN is fed in) then it may be 
	 *	neccesary to clear the filter by calling this method.
	 *	@return Returns a TTErr error code.												*/
	
};


#endif // __TT_CRYBABY_H__
