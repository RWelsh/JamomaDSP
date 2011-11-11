/* 
 *	External object for Max/MSP
 *	Copyright © 2011 by Trond Lossius
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"

int TTCLASSWRAPPERMAX_EXPORT main(void)
{
    TTDSPInit();
	return wrapTTClassAsMaxClass(TT("stereo2MidSide"), "jcom.stereo2MidSide~", NULL);
}
