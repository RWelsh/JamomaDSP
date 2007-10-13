//	TTDCBlockObject
//	dc offset filter / blocker 
//	Copyright © 2007 by Timothy A. Place
//	License: GNU LGPL

#import "TTDCBlock.h"


@implementation TTDCBlockObject

- (id)init
{
	[super init];
	[self clear];
	return self;
}	


- (void) dealloc
{
	[super dealloc];
}


- (TTErr) clear
{
	short i;

	for(i=0; i<TT_MAX_NUM_CHANNELS; i++){
		last_input[i] = 0;
		last_output[i] = 0;
	}
	return TT_ERR_NONE;
}


// DSP LOOP
// Note: an algorithm that is frequently used in Max:
//		"biquad~ 1.0 -1.0 -0.9997 0.0"
- (TTErr) processAudioWithInput:(TTAudioSignal *)audioIn andOutput:(TTAudioSignal *)audioOut
{
	short			vs = audioIn->vs;
	float			*in,
					*out;
	short			numchannels = [TTAudioSignal GetMinNumChannelsForASignal:audioIn andAnotherSignal:audioOut];
	short			channel;
	TTSampleValue	temp;
	
	for(channel=0; channel<numchannels; channel++){
		in = audioIn->vectors[channel];
		out = audioOut->vectors[channel];
		
		while(vs--){
			temp = *in++;
			*out++ = last_output[channel] = ttantidenormal(temp - last_input[channel] + (last_output[channel] * 0.9997));
			last_input[channel] = temp;
		}
	}
	return TT_ERR_NONE;
}


@end
