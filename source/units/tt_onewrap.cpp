#include "tt_onewrap.h"

TT_INLINE tt_onewrap::tt_onewrap(void)									// Constructor		
{
	;
}

TT_INLINE tt_onewrap::~tt_onewrap(void)								// Destructor
{
	;
}


// ATTRIBUTES
TT_INLINE void tt_onewrap::set_attr(tt_selector sel, tt_attribute_value val)	// Set Attributes
{
	;
}


TT_INLINE tt_attribute_value tt_onewrap::get_attr(tt_selector sel)			// Get Attributes
{
	;
}		


// DSP LOOP - ARGUMENT IS A CONSTANT
TT_INLINE void tt_onewrap::dsp_vector_calc(tt_audio_signal *in, tt_audio_signal *out)
{
	tt_sample_value temp;
	temp_vs = in->vectorsize;
	
	while(temp_vs--){
		//*out++ = onewrap(*in++, 0.0f, 1.0f);
		temp = *in->vector++;
		if(temp >= 1.0)
			temp -= 1.0;
		else if(temp < 0.0)
			temp += 1.0;
		*out->vector++ = temp;
	}
	in->reset(); out->reset();
}