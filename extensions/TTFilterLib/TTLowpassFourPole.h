/* 
 * TTBlue 4-Pole Lowpass Filter Object
 * Copyright © 2008, Tim Place
 * 
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html 
 */

#ifndef __TT_LOWPASS_FOURPOLE_H__
#define __TT_LOWPASS_FOURPOLE_H__

#include "TTBlueAPI.h"


/**	A resonant two-pole, no-zero, lowpass filter.  Based on moog-variation2 @ musicdsp.org. */
TTAUDIOCLASS(TTLowpassFourPole)

	TTFloat64		frequency;			///< filter cutoff frequency
	TTFloat64		resonance;			///< filter resonance -- range is best between 1.0 and 16.0
	TTFloat64		coefficientF;			///< filter coefficient
	TTFloat64		coefficientFB;			///< filter coefficient
	TTFloat64		coefficientG;			///< filter coefficient
	TTFloat64		*x1;					///< previous input sample for each channel
	TTFloat64		*x2;					///< 2nd previous input sample for each channel
	TTFloat64		*x3;					///< 3rd previous input sample for each channel
	TTFloat64		*x4;					///< 4th previous input sample for each channel
	TTFloat64		*y1;					///< previous output sample for each channel
	TTFloat64		*y2;					///< 2nd previous output sample for each channel
	TTFloat64		*y3;					///< 3rd previous output sample for each channel
	TTFloat64		*y4;					///< 4th previous output sample for each channel
	TTFloat64		deciResonance;			///< attrResonance * 0.1

	// Notifications
	TTErr updateMaxNumChannels(const TTValue& oldMaxNumChannels);
	TTErr updateSr();

	void calculateCoefficients();
	
	TTErr calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel);
	
	/**	Standard audio processing method as used by TTBlue objects. */
	TTErr processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	
	// Attributes
	TTErr setfrequency(const TTValue& newValue);
	TTErr setresonance(const TTValue& newValue);
	
	/**	This algorithm uses an IIR filter, meaning that it relies on feedback.  If the filter should
	 *	not be producing any signal (such as turning audio off and then back on in a host) or if the
	 *	feedback has become corrupted (such as might happen if a NaN is fed in) then it may be 
	 *	neccesary to clear the filter by calling this method.
	 *	@return Returns a TTErr error code.												*/
	TTErr clear();
};


#endif // __TT_LOWPASS_FOURPOLE_H__