#include "tt_mixer_mono.h"


// OBJECT LIFE					
tt_mixer_mono::tt_mixer_mono(void)								// Constructor		
{
	int i;
	for(i=0; i<MAX_NUM_CHANNELS; i++)
		channel_gain[i] = 0.2;
	master_gain = 1.0;
}

tt_mixer_mono::~tt_mixer_mono(void)								// Destructor
{
	;
}


// ATTRIBUTES - TWO ARGUMENTS
TT_INLINE
tt_err tt_mixer_mono::set_attr(tt_selector sel, const tt_value &a)		// Set Attributes
{
//	tt_value		temp_atom;
//	
//	temp_atom.set_num_items(2);
//	temp_atom.set(0, );
	tt_uint16	chan;
	tt_float32	val;
	
	switch (sel){
		case k_master_gain:
			a.get(0, val);
			master_gain = decibels_to_amplitude(val);
			break;
		case k_channel_gain:
			a.get(0, chan);
			a.get(1, val);
			channel_gain[chan] = decibels_to_amplitude(val);
			break;
		default:
			return TT_ERR_ATTR_INVALID;
	}
	return TT_ERR_NONE;
}


tt_err tt_mixer_mono::get_attr(tt_selector sel, tt_value &a)				// Get Attributes
{
	tt_uint16	chan;

	switch (sel){
		case k_master_gain:
			a = amplitude_to_decibels(master_gain);
			break;	
		case k_channel_gain:
			chan = a;
			a = amplitude_to_decibels(channel_gain[chan]);
			break;
		default:
			return TT_ERR_ATTR_INVALID;
	}
	return TT_ERR_NONE;
}


// DSP LOOP - 2 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]));
	in1->reset(); in2->reset(); out->reset();
}

// DSP LOOP - 3 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]));
	in1->reset(); in2->reset(); in3->reset(); out->reset();
}

// DSP LOOP - 4 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); out->reset();
}

// DSP LOOP - 5 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *in5, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]) + (*in5->vector++ * channel_gain[4]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); in5->reset(); out->reset();
}

// DSP LOOP - 6 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *in5, tt_audio_signal *in6, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]) + (*in5->vector++ * channel_gain[4]) + 
			(*in6->vector++ * channel_gain[5]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); in5->reset(); in6->reset(); out->reset();
}

// DSP LOOP - 7 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *in5, tt_audio_signal *in6, tt_audio_signal *in7, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]) + (*in5->vector++ * channel_gain[4]) + 
			(*in6->vector++ * channel_gain[5]) + (*in7->vector++ * channel_gain[6]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); in5->reset(); in6->reset(); in7->reset(); out->reset();
}

// DSP LOOP - 8 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *in5, tt_audio_signal *in6, tt_audio_signal *in7, tt_audio_signal *in8, 
	tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]) + (*in5->vector++ * channel_gain[4]) + 
			(*in6->vector++ * channel_gain[5]) + (*in7->vector++ * channel_gain[6]) + (*in8->vector++ * channel_gain[7]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); in5->reset(); in6->reset(); in7->reset(); in8->reset();
	out->reset();
}

// DSP LOOP - 9 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *in5, tt_audio_signal *in6, tt_audio_signal *in7, tt_audio_signal *in8, tt_audio_signal *in9, 
	tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]) + (*in5->vector++ * channel_gain[4]) + 
			(*in6->vector++ * channel_gain[5]) + (*in7->vector++ * channel_gain[6]) + (*in8->vector++ * channel_gain[7]) + 
			(*in9->vector++ * channel_gain[8]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); in5->reset(); in6->reset(); in7->reset(); in8->reset();
	in9->reset(); out->reset();
}

// DSP LOOP - 10 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *in5, tt_audio_signal *in6, tt_audio_signal *in7, tt_audio_signal *in8, tt_audio_signal *in9, 
	tt_audio_signal *in10, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]) + (*in5->vector++ * channel_gain[4]) + 
			(*in6->vector++ * channel_gain[5]) + (*in7->vector++ * channel_gain[6]) + (*in8->vector++ * channel_gain[7]) + 
			(*in9->vector++ * channel_gain[8]) + (*in10->vector++ * channel_gain[9]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); in5->reset(); in6->reset(); in7->reset(); in8->reset();
	in9->reset(); in10->reset(); out->reset();
}		

// DSP LOOP - 11 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *in5, tt_audio_signal *in6, tt_audio_signal *in7, tt_audio_signal *in8, tt_audio_signal *in9, 
	tt_audio_signal *in10, tt_audio_signal *in11, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]) + (*in5->vector++ * channel_gain[4]) + 
			(*in6->vector++ * channel_gain[5]) + (*in7->vector++ * channel_gain[6]) + (*in8->vector++ * channel_gain[7]) + 
			(*in9->vector++ * channel_gain[8]) + (*in10->vector++ * channel_gain[9]) + (*in11->vector++ * channel_gain[10]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); in5->reset(); in6->reset(); in7->reset(); in8->reset();
	in9->reset(); in10->reset(); in11->reset(); out->reset();
}		

// DSP LOOP - 12 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *in5, tt_audio_signal *in6, tt_audio_signal *in7, tt_audio_signal *in8, tt_audio_signal *in9, 
	tt_audio_signal *in10, tt_audio_signal *in11, tt_audio_signal *in12, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]) + (*in5->vector++ * channel_gain[4]) + 
			(*in6->vector++ * channel_gain[5]) + (*in7->vector++ * channel_gain[6]) + (*in8->vector++ * channel_gain[7]) + 
			(*in9->vector++ * channel_gain[8]) + (*in10->vector++ * channel_gain[9]) + (*in11->vector++ * channel_gain[10])
			 + (*in12->vector++ * channel_gain[11]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); in5->reset(); in6->reset(); in7->reset(); in8->reset();
	in9->reset(); in10->reset(); in11->reset(); in12->reset(); out->reset();
}		

// DSP LOOP - 13 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *in5, tt_audio_signal *in6, tt_audio_signal *in7, tt_audio_signal *in8, tt_audio_signal *in9, 
	tt_audio_signal *in10, tt_audio_signal *in11, tt_audio_signal *in12, tt_audio_signal *in13, 
	tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]) + (*in5->vector++ * channel_gain[4]) + 
			(*in6->vector++ * channel_gain[5]) + (*in7->vector++ * channel_gain[6]) + (*in8->vector++ * channel_gain[7]) + 
			(*in9->vector++ * channel_gain[8]) + (*in10->vector++ * channel_gain[9]) + (*in11->vector++ * channel_gain[10])
			 + (*in12->vector++ * channel_gain[11]) + (*in13->vector++ * channel_gain[12]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); in5->reset(); in6->reset(); in7->reset(); in8->reset();
	in9->reset(); in10->reset(); in11->reset(); in12->reset(); in13->reset(); out->reset();
}		

// DSP LOOP - 14 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *in5, tt_audio_signal *in6, tt_audio_signal *in7, tt_audio_signal *in8, tt_audio_signal *in9, 
	tt_audio_signal *in10, tt_audio_signal *in11, tt_audio_signal *in12, tt_audio_signal *in13, tt_audio_signal *in14, 
	tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]) + (*in5->vector++ * channel_gain[4]) + 
			(*in6->vector++ * channel_gain[5]) + (*in7->vector++ * channel_gain[6]) + (*in8->vector++ * channel_gain[7]) + 
			(*in9->vector++ * channel_gain[8]) + (*in10->vector++ * channel_gain[9]) + (*in11->vector++ * channel_gain[10])
			 + (*in12->vector++ * channel_gain[11]) + (*in13->vector++ * channel_gain[12]) + (*in14->vector++ * channel_gain[13]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); in5->reset(); in6->reset(); in7->reset(); in8->reset();
	in9->reset(); in10->reset(); in11->reset(); in12->reset(); in13->reset(); in14->reset(); out->reset();
}		

// DSP LOOP - 15 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *in5, tt_audio_signal *in6, tt_audio_signal *in7, tt_audio_signal *in8, tt_audio_signal *in9, 
	tt_audio_signal *in10, tt_audio_signal *in11, tt_audio_signal *in12, tt_audio_signal *in13, tt_audio_signal *in14, 
	tt_audio_signal *in15, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]) + (*in5->vector++ * channel_gain[4]) + 
			(*in6->vector++ * channel_gain[5]) + (*in7->vector++ * channel_gain[6]) + (*in8->vector++ * channel_gain[7]) + 
			(*in9->vector++ * channel_gain[8]) + (*in10->vector++ * channel_gain[9]) + (*in11->vector++ * channel_gain[10])
			 + (*in12->vector++ * channel_gain[11]) + (*in13->vector++ * channel_gain[12]) + (*in14->vector++ * channel_gain[13])
			 + (*in15->vector++ * channel_gain[14]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); in5->reset(); in6->reset(); in7->reset(); in8->reset();
	in9->reset(); in10->reset(); in11->reset(); in12->reset(); in13->reset(); in14->reset(); in15->reset();
	out->reset();
}		

// DSP LOOP - 16 CHANNELS
void tt_mixer_mono::dsp_vector_calc(tt_audio_signal *in1, tt_audio_signal *in2, tt_audio_signal *in3, tt_audio_signal *in4, 
	tt_audio_signal *in5, tt_audio_signal *in6, tt_audio_signal *in7, tt_audio_signal *in8, tt_audio_signal *in9, 
	tt_audio_signal *in10, tt_audio_signal *in11, tt_audio_signal *in12, tt_audio_signal *in13, tt_audio_signal *in14, 
	tt_audio_signal *in15, tt_audio_signal *in16, tt_audio_signal *out)
{
	temp_vs = in1->vectorsize;
	while(temp_vs--)
		*out->vector++ = master_gain * ((*in1->vector++ * channel_gain[0]) + (*in2->vector++ * channel_gain[1]) + 
			(*in3->vector++ * channel_gain[2]) + (*in4->vector++ * channel_gain[3]) + (*in5->vector++ * channel_gain[4]) + 
			(*in6->vector++ * channel_gain[5]) + (*in7->vector++ * channel_gain[6]) + (*in8->vector++ * channel_gain[7]) + 
			(*in9->vector++ * channel_gain[8]) + (*in10->vector++ * channel_gain[9]) + (*in11->vector++ * channel_gain[10])
			 + (*in12->vector++ * channel_gain[11]) + (*in13->vector++ * channel_gain[12]) + (*in14->vector++ * channel_gain[13])
			 + (*in15->vector++ * channel_gain[14]) + (*in16->vector++ * channel_gain[15]));
	in1->reset(); in2->reset(); in3->reset(); in4->reset(); in5->reset(); in6->reset(); in7->reset(); in8->reset();
	in9->reset(); in10->reset(); in11->reset(); in12->reset(); in13->reset(); in14->reset(); in15->reset(); in16->reset();
	out->reset();
}		
