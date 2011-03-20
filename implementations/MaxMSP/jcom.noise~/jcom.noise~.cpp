/* 
 * wrapper for TTNoise for Max jcom.noise~
 *
 *	
 *	Copyright © 2008 by Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"

int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	WrappedClassOptionsPtr	options = new WrappedClassOptions;
	WrappedClassPtr			c = NULL;

	TTDSPInit();
	
	options->append(TT("generator"), kTTBoolYes);
	return wrapTTClassAsMaxClass(TT("noise"), "jcom.noise~",  &c, options);
	CLASS_ATTR_ENUM(c->maxClass, "mode", 0, "white pink brown blue");
}
