#include "tt_polar.h"


// OBJECT LIFE					
/*tt_polar::tt_polar()										// Constructor		
{
	set_attr(k_mode, k_mode_cartopol);
}
*/
tt_polar::tt_polar(selectors init_mode)
{
	set_attr(k_mode, init_mode);
}

tt_polar::~tt_polar()									// Destructor
{
	;
}


// ATTRIBUTES
TT_INLINE 
tt_err tt_polar::set_attr(tt_selector sel, const tt_value &val)	// Set Attributes
{
	switch (sel){
		case k_mode:
			mode = val;
			if(mode == k_mode_cartopol)
				dsp_executor = &tt_polar::dsp_vector_calc_cartopol;
			else if(mode == k_mode_poltocar)
				dsp_executor = &tt_polar::dsp_vector_calc_poltocar;
			break;
		default:
			return TT_ERR_ATTR_INVALID;	// really should make this throw and exception (applies to all objects)!
	}
	return TT_ERR_NONE;
}

TT_INLINE 
tt_err tt_polar::get_attr(tt_selector sel, tt_value &a)				// Get Attributes
{
	switch(sel){
		case k_mode:
			a = mode;
			break;
		default:
			return TT_ERR_ATTR_INVALID;	// really should make this throw and exception (applies to all objects)!
	}
	return TT_ERR_NONE;
}


/*****************************************************
 * DSP LOOPS
 *****************************************************/

// Publically exposed interface for the dsp routine
TT_INLINE 
void tt_polar::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *out1, tt_audio_signal *out2)
{
	(*this.*dsp_executor)(in1, in2, out1, out2);	// Run the function pointed to by our function pointer
}


// DSP LOOP: CARTOPOL
TT_INLINE 
void tt_polar::dsp_vector_calc_cartopol(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *out1, tt_audio_signal *out2)
{
	tt_sample_value	real, imaginary, magnitude, phase;
	temp_vs = in1->vectorsize;
							
    while (temp_vs--){
		real = *in1->vector++;
		imaginary = *in2->vector++;

		magnitude = sqrt((real * real) + (imaginary * imaginary));
		
		if (real == 0)
			real = 0.000001;				// prevent divide by zero
		phase = atan(imaginary / real);	
		if ((real < 0) && (imaginary < 0))		// arctangent corrections
			phase = phase - 3.14159265;
		else if ((real < 0) && (imaginary >= 0))
			phase = phase + 3.14159265;

		*out1->vector++ = magnitude;
		*out2->vector++ = phase;	
    }
    in1->reset(); in2->reset(); out1->reset(); out2->reset();
}


// DSP LOOP: POLTOCAR
TT_INLINE void tt_polar::dsp_vector_calc_poltocar(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *out1, tt_audio_signal *out2)
{
	tt_sample_value	real, imaginary, magnitude, phase;
	temp_vs = in1->vectorsize;
							
    while (temp_vs--){
		magnitude = *in1->vector++;
		phase = *in2->vector++;

		real = magnitude * cos(phase);
		imaginary = magnitude * sin(phase);

		*out1->vector++ = real;
		*out2->vector++ = imaginary;	
    }
    in1->reset(); in2->reset(); out1->reset(); out2->reset();
}		
