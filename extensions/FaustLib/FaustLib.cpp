/* 
 * FaustLib -- A library of effects units from the Faust project 
 * Extension Class for Jamoma DSP
 * Copyright © 2011, Nils Peters
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */


#include "TTDSP.h"
#include "TTCryBaby.h"
#include "TTWah.h"


extern "C" TT_EXTENSION_EXPORT TTErr loadTTExtension(void)
{
	TTDSPInit();
	
	TTCryBaby::registerClass();
	TTWah::registerClass();
	
	return kTTErrNone;
}

