/*
 *******************************************************
 *		DEGRADE
 *		sample-rate and bit-depth reduction 
 *******************************************************
 *		TTBlue Object
 *		Copyright � 2003 by Timothy A. Place
 *
 */

// Check against redundant including
#ifndef TT_DEGRADE_H
#define TT_DEGRADE_H

// Include appropriate headers
#include "tt_audio_base.h"
#include "tt_audio_signal.h"


/********************************************************
	CLASS INTERFACE
 ********************************************************/

#define BIG_INT	0x00800000
#define ONE_OVER_BIG_INT 1.1920928955E-7

class tt_degrade:public tt_audio_base{

	private:
		tt_attribute_value_discrete 	bitdepth;
		tt_attribute_value				sr_ratio;
		short							bit_shift;
		float							accumulator;
		tt_sample_value					output;
	
	public:
		enum selectors{								
			k_bitdepth,								// Attribute Selectors
			k_sr_ratio
		};
		
		// OBJECT LIFE					
		tt_degrade();								// Constructor		
		~tt_degrade();								// Destructor

		// ATTRIBUTES
		tt_err 		set_attr(tt_selector sel, const tt_atom &val);
		tt_err		get_attr(tt_selector sel, tt_atom &value);
				
		// DSP LOOP
		void dsp_vector_calc(tt_audio_signal *in, tt_audio_signal *out);
};

#endif	// TT_DEGRADE_H