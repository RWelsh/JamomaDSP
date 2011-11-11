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
    WrappedClassOptionsPtr	options = new WrappedClassOptions;    
    TTValue value(2);
    options->append(TT("fixedNumChannels"), value);
    
    TTDSPInit();
	return wrapTTClassAsMaxClass(TT("stereo2MidSide"), "jcom.stereo2MidSide~", NULL, options);
}
