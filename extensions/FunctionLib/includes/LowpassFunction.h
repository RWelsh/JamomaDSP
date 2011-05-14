/* 
 * LowpassFunction Unit for TTBlue
 * Originally written for the Jamoma FunctionLib
 * Copyright © 2007 by Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __LOWPASSFUNCTION_H__
#define __LOWPASSFUNCTION_H__

#include "TTDSP.h"


/**	A simple one-pole lowpass function.
	This is most useful as a way of smoothing the input according to a coefficient.
	At a basic level it is the same as the TTLowpassOnepole class, however this version
	is much simpler and is optimized for calls to calculate a single output rather than
	multichannel audio signals.
 */
class LowpassFunction : TTAudioObject {
	TTCLASS_SETUP(LowpassFunction)

	TTFloat64		mCoefficient;
	TTFloat64		mOneMinusCoefficient;
	TTFloat64*		mFeedback;

	TTErr updateMaxNumChannels(const TTValue& oldMaxNumChannels);
	TTErr clear();
	TTErr setCoefficient(const TTValue& newValue);

	/** y = f(x) for a single value */
	TTErr calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt data);
	
	/**	A standard audio processing method as used by TTBlue objects.*/
	TTErr processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);

};


#endif // __LOWPASSFUNCTION_H__
