/* 
 * EffectsLib -- A library of effects units
 * Extension Class for Jamoma DSP
 * Copyright © 2009-2011, Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */


#include "TTDSP.h"
#include "TTBalance.h"
#include "TTCryBaby.h"
#include "TTDegrade.h"
#include "TTGain.h"
#include "TTLimiter.h"
#include "TTOverdrive.h"
#include "TTPulseSub.h"
#include "TTWah.h"


extern "C" TT_EXTENSION_EXPORT TTErr loadTTExtension(void)
{
	TTDSPInit();
	
	TTBalance::registerClass();	
	TTCryBaby::registerClass();
	TTDegrade::registerClass();	
	TTGain::registerClass();	
	TTLimiter::registerClass();	
	TTOverdrive::registerClass();	
	TTPulseSub::registerClass();	
	TTWah::registerClass();
	
	return kTTErrNone;
}

