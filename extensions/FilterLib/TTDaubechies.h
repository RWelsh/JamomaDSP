/* 
 * A resampling FIR filter using coefficients from orthogonal wavelet sequences defined by Daubechies.
 * Copyright © 2010, Tim Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __TT_DAUBECHIES_H__
#define __TT_DAUBECHIES_H__

#include "TTDSP.h"


/**	A resampling FIR filter using coefficients from orthogonal wavelet sequences defined by Daubechies.
	The D2 sequence is the same as the Haar sequence.
	This filter forms one stage of an iterated filterbank for implementing a Discrete Wavelet Transform.
 */
class TTDaubechies : TTAudioObject {
	TTCLASS_SETUP(TTDaubechies)
	
	#define DAUBECHIES_MAX_NUM_COEFFICIENTS 74
	#define DAUBECHIES_NUM_COEFFICIENT_SETS 4
	static const TTSampleValue kCoefficients[DAUBECHIES_NUM_COEFFICIENT_SETS][DAUBECHIES_MAX_NUM_COEFFICIENTS];

	TTSampleVector*	mX;					///< array of vectors, each representing previous input samples for a single channel
	TTSymbolPtr		mSequence;			///< e.g. 'D2', 'D12', 'D20', etc.
	TTUInt16		mNumCoefficients;	///< e.g. 2, 12, 20, etc.
	TTSampleVector	mG;					///< lowpass coefficients
	TTSampleVector	mH;					///< highpass coefficients
	
	// Notifications
	TTErr updateMaxNumChannels(const TTValue& oldMaxNumChannels);

	// attribute accessor
	TTErr setSequence(const TTValue& newValue);
	
	// internal worker for the setSequence method
	void setNumCoefficients(TTUInt16 numCoefficients);

	// Do the processing
	TTErr processForward	(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	TTErr processReverse	(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	TTErr processDownsample	(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	TTErr processUpsample	(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	
public:
	TTErr calculateLowpass		(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel=0);
	TTErr calculateHighpass		(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel=0);
	TTErr calculateDownsample	(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel=0);
	TTErr calculateUpsample		(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt channel=0);
};


#endif // __TT_DAUBECHIES_H__
