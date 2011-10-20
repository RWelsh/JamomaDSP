/* 
 * GranularLib
 * Extension Class for Jamoma DSP
 * Copyright © 2011, Nathan Wolek
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTDSP.h"

extern "C" TT_EXTENSION_EXPORT TTErr TTLoadJamomaExtension_GranularLib(void)
{
	TTDSPInit();

	
	return kTTErrNone;
}

