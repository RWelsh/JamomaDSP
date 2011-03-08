/*
 * Bessel Function support for Jamoma DSP
 * Copyright © 2011, Timothy Place
 *
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTBessel.h"


TTFloat64 TTBesselFunctionI0(TTFloat64 x)
{
	double y = 1.0;
	double u = 1.0;
	double halfx = x * 0.5;
	double temp;
	long i = 1;
	
	while (u > kTTEpsilon) {
		temp = halfx/(double)i;
		temp *= temp;
		u *= temp;
		y += u;
		i++;
	}
	return y;
}




