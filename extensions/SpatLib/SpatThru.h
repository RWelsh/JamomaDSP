/* 
 * Super simple Spatialization object for Jamoma DSP
 * Copyright © 2011 by Trond Lossius, Nils Peters, and Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __SPATTHRU_H__
#define __SPATTHRU_H__

#include "SpatLib.h"


/**	This implements a window function which basically does nothing: 
	y = 1.0
 */
class SpatThru : TTAudioObject {
	TTCLASS_SETUP(SpatThru)
	
	/**	A standard audio processing method as used by TTBlue objects.*/
	TTErr processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);

};


#endif // __SPATTHRU_H__
