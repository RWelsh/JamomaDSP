/* 
 * TTWah Fourth-order wah effect made using moog_vcf
 * Copyright © 2011, Nils Peters, ported from FAUST effect lib, see below for details
 *
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __TT_WAH_H__
#define __TT_WAH_H__

#include "TTDSP.h"


/**	4th order Linkwitz Riley Lowpass Filter
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
 */
class TTWah : TTAudioObject {
	TTCLASS_SETUP(TTWah)

	TTFloat64			mFrequency, mRadial, mFrequencyRad;				///< filter cutoff frequency
	TTSampleVector		fRec11, fRec10;
	TTSampleVector		fRec01;
	TTSampleVector		fRec00;
	TTSampleVector		fRec21;
	TTSampleVector		fRec20;
	TTSampleVector		fRec31;
	TTSampleVector		fRec30;
	TTSampleVector		fRec41;
	TTSampleVector		fRec40;
	TTSampleVector		fRec51;
	TTSampleVector		fRec50;
	
	/**	Receives notifications when there are changes to the inherited 
	 maxNumChannels attribute.  This allocates memory for xm1, xm2, ym1, and ym2 
	 so that each channel's previous values are remembered.		*/
	TTErr updateMaxNumChannels(const TTValue& oldMaxNumChannels);
	TTErr updateSampleRate(const TTValue& oldSampleRate);
	TTErr clear();
	
	void calculateCoefficients();
	
    /**	Standard single value calculate method as used by DSP objects. */
	inline TTErr calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel);
	
	/**	Standard audio processing method as used by TTBlue objects. */
	TTErr processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	
	/**	Setter for the frequency attribute. */
	TTErr setFrequency(const TTValue& value);
	
	/**	This algorithm uses an IIR filter, meaning that it relies on feedback.  If the filter should
	 *	not be producing any signal (such as turning audio off and then back on in a host) or if the
	 *	feedback has become corrupted (such as might happen if a NaN is fed in) then it may be 
	 *	neccesary to clear the filter by calling this method.
	 *	@return Returns a TTErr error code.												*/
	
};


#endif // __TT_WAH_H__
