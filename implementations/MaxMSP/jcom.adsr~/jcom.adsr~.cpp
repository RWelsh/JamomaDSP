/* 
 *	tt.dcblock~
 *	External object for Max/MSP
 *	Remove DC Offsets from a signal
 *	Example project for TTBlue
 *	Copyright © 2008 by Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"

int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	WrappedClassOptionsPtr	options = new WrappedClassOptions;
	//TTValue					value;
	WrappedClassPtr			c = NULL;
	TTDSPInit();
	options->append(TT("generator"), kTTBoolYes);
	return wrapTTClassAsMaxClass(TT("adsr"), "jcom.adsr~",  &c, options);
	CLASS_ATTR_ENUM(c->maxClass,	"mode",		0, "exponential hybrid linear");
	CLASS_ATTR_STYLE(c->maxClass,	"trigger",	0,	"onoff");
}

