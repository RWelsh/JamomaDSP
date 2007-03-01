#include "tt_modulo.h"

TT_INLINE 
tt_modulo::tt_modulo(void)	// Constructor		
{
	set_attr(k_modulo_argument, 1.0);
}

TT_INLINE 
tt_modulo::~tt_modulo(void)		// Destructor
{
	;
}


// ATTRIBUTES
TT_INLINE 
tt_err tt_modulo::set_attr(tt_selector sel, const tt_atom &val)			// Set Attributes
{
	switch (sel){
		case k_modulo_argument:
			argument = val;
			// NOTE: SHOULD CHECK TO SEE IF THE VALUE IS A POWER OF TWO - THEN CAN OPTIMIZE
			//	BY DOING BITAND INSTEAD OF A MATHEMATICAL MODULO
			break;
		default:
			return TT_ERR_ATTR_INVALID;
	}
	return TT_ERR_NONE;
}


TT_INLINE 
tt_err tt_modulo::get_attr(tt_selector sel, tt_atom &val)					// Get Attributes
{
	switch(sel){
		case k_modulo_argument:
			return argument;
		default:
			return TT_ERR_ATTR_INVALID;
	}
	return TT_ERR_NONE;
}



// DSP LOOP - ARGUMENT IS A CONSTANT
TT_INLINE void tt_modulo::dsp_vector_calc(tt_audio_signal *in, tt_audio_signal *out)
{
	temp_vs = in->vectorsize;
	while(temp_vs--)
		*out->vector++ = fmod(*in->vector++, argument);
		//*out++ = *in++ & argument;
//p2 &= ((wavetable->length_samples) - 1);	// fast modulo
	in->reset(); out->reset();
}

// DSP LOOP - ARGUMENT IS A SIGNAL
TT_INLINE void tt_modulo::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = fmod(*in1->vector++, *in2->vector++);
	in1->reset(); in2->reset(); out->reset();
}