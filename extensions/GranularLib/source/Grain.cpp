/*
 * Grain - defines the necessary info for a grain of sound 
 * Copyright © 2011, Nathan Wolek
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

//using TTAverage as a starting point NEW 19 Oct 2011

#include "Grain.h"

#define thisTTClass			Grain
#define thisTTClassName		"grain"
#define thisTTClassTags		"audio, sampling, grain"


#ifndef TT_PLATFORM_MAC
#include <algorithm>
#endif


TT_AUDIO_CONSTRUCTOR
{
	//TODO: define default vars here
	
}


Grain::~Grain()
{
	;// deconstruct me
}


TTErr Grain::init()
{
	
		// init me
		reset();

return kTTErrNone;
}





