#include "tt_crossfade.h"


// OBJECT LIFE					
TT_INLINE tt_crossfade::tt_crossfade(void)									// Constructor		
{
	position = 0.5;
	shape = k_shape_equalpower;
	set_attr(k_mode, k_mode_lookup);
}

TT_INLINE tt_crossfade::~tt_crossfade(void)								// Destructor
{
	;
}


// ATTRIBUTES
TT_INLINE 
tt_err tt_crossfade::set_attr(tt_selector sel, const tt_atom &a)	// Set Attributes
{
	tt_float32 val = a;
	
	switch (sel){
		case k_position:
			position = clip(val, (float)0.0, (float)1.0);
			break;
		case k_shape:
			shape = val;
			break;
		case k_mode:
			mode = val;
			break;
		default:
			return TT_ERR_ATTR_INVALID;
	}
	// set the function pointer for the correct dsp loop to run
	if(shape == k_shape_linear)
		dsp_executor = &tt_crossfade::dsp_vector_calc_linear;
	else if(mode == k_mode_calculate)
		dsp_executor = &tt_crossfade::dsp_vector_calc_equalpower_calc;
	else if(mode == k_mode_lookup)
		dsp_executor = &tt_crossfade::dsp_vector_calc_equalpower_lookup;

	return TT_ERR_NONE;	
}

TT_INLINE 
tt_err tt_crossfade::get_attr(tt_selector sel, tt_atom &a)				// Get Attributes
{
	switch (sel){
		case k_position:
			a = position;
			break;
		case k_shape:
			a = shape;
			break;
		case k_mode:
			a = mode;
			break;
		default:
			return TT_ERR_ATTR_INVALID;
	}
	return TT_ERR_NONE;
}


/*****************************************************
 * DSP LOOPS
 *****************************************************/

// Publically exposed interface for this object's dsp routine
TT_INLINE void tt_crossfade::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *out)
{
	(*this.*dsp_executor)(in1, in2, out);	// Run the function pointed to by our function pointer
}


// DSP LOOP: LINEAR SHAPE
void tt_crossfade::dsp_vector_calc_linear(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = (*in2->vector++ * position) + (*in1->vector++ * (1.0 - position));
	in1->reset(); in2->reset(); out->reset();
}

// DSP LOOP: EQUAL POWER LOOKUP MODE
void tt_crossfade::dsp_vector_calc_equalpower_lookup(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *out)
{
	int index;
	temp_vs = in1->vectorsize;
	while(temp_vs--){
		index = (int)(position * 511.0);
		*out->vector++ = (*in2->vector++ * lookup_equalpower[511 - index]) + (*in1->vector++ * lookup_equalpower[index]);
	}
	in1->reset(); in2->reset(); out->reset();
}

// DSP LOOP: EQUAL POWER CALCULATED
void tt_crossfade::dsp_vector_calc_equalpower_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = (*in2->vector++ * (sin(position * 1.5707963))) + (*in1->vector++ * (sin((1 - position) * 1.5707963)));
	in1->reset(); in2->reset(); out->reset();
}