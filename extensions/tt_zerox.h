/*
 *******************************************************
 *		ZERO CROSSING DETECTOR / COUNTER
 *******************************************************
 *		TT Object
 *		Copyright � 2003 by Timothy A. Place
 *
 */

// Check against redundant including
#ifndef TT_ZEROX_H
#define TT_ZEROX_H

// Include appropriate headers
#include "tt_audio_base.h"
#include "tt_audio_bundle.h"


/********************************************************
	CLASS INTERFACE
 ********************************************************/

class tt_zerox:public tt_audio_base{

	private:
		tt_attribute_value_discrete 	analysis_samps;				// analysis period in samples
		double							r_analysis_samps;			// reciprocal of analysis size
		bool							last_samp_was_over_zero;	// was the last sample over zero?
		long							counter;					// counts zero-crossings
		tt_sample_value					final_count;
		long							analysis_location;			// keep track of how many samples so far
	
	public:
		enum selectors{												// Attribute Selectors
			k_analysis_size,
		};
		
		// OBJECT LIFE					
		tt_zerox();													// Constructor		
		~tt_zerox();												// Destructor

		// ATTRIBUTES
		tt_err 		set_attr(tt_selector sel, const tt_value &val);
		tt_err		get_attr(tt_selector sel, tt_value &value);
				
		// DSP LOOP
//		void dsp_vector_calc(tt_audio_signal *in, tt_audio_signal *out1, tt_audio_signal *out2);
		/**
		 	WARNING: this unit requires 1 input and 2 outputs -- it does not yet configure itself for other arrangements!
		 */
		void process(tt_audio_bundle *in, tt_audio_bundle *out);

		// clear
		void clear();
};

#endif	// TT_ZEROX_H
