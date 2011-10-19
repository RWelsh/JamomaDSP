/* 
 * LinearFunction Unit for TTBlue
 * Originally written for the Jamoma FunctionLib
 * Copyright © 2007 by Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __LINEAR_3D_H__
#define __LINEAR_3D_H__

#include "TTDSP.h"


/**	This implements a function which basically does nothing: 
 y = f(x)
 */
class Linear3D : TTAudioObject {
	TTCLASS_SETUP(Linear3D)
	
	TTFloat64 mA, mB;
	//inline TTErr calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt data);
	
	/**	A standard audio processing method as used by TTBlue objects.*/
	TTErr processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
};


#endif // __LINEAR_3D_H__
