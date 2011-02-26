/* 
 * TTDistortion: Cubic nonlinearity distortion
 * Copyright © 2011, Nils Peters, ported from FAUST
 *
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __TT_DISTORTION_H__
#define __TT_DISTORTION_H__

#include "TTDSP.h"

class TTDistortion : TTAudioObject {
	TTCLASS_SETUP(TTDistortion)

	TTFloat64			mOffset, mDrive, fSlow0, fSlow1;				
	//TTSampleVector		fRec10;
	//TTSampleVector		fRec11;
	TTSampleVector		fRec01;
	TTSampleVector		fRec00;
	//TTSampleVector		fRec21;
	//TTSampleVector		fRec20;
	TTSampleVector		fVec01;
	TTSampleVector		fVec00;
	
	
	/**	Receives notifications when there are changes to the inherited 
	 maxNumChannels attribute.  This allocates memory for xm1, xm2, ym1, and ym2 
	 so that each channel's previous values are remembered.		*/
	TTErr updateMaxNumChannels(const TTValue& oldMaxNumChannels);
	TTErr clear();
		
    /**	Standard single value calculate method as used by DSP objects. */
	inline TTErr calculateValue(const TTFloat64& input0, TTFloat64& output0, TTPtrSizedInt channel);
	
	/**	Standard audio processing method as used by TTBlue objects. */
	TTErr processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	
	/**	Setter for the attributes. */
	TTErr setDrive(const TTValue& value);
	TTErr setOffset(const TTValue& value);
		
};


#endif // __TT_DISTORTION_H__
