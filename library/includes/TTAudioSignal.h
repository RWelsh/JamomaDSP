/* 
 * TTBlue Audio Signal Class
 * Copyright © 2008, Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#ifndef __TT_AUDIO_SIGNAL_H__
#define __TT_AUDIO_SIGNAL_H__

#include "TTFoundation.h"
#include "TTObject.h"
#include "TTSymbol.h"
#include "TTValue.h"


/****************************************************************************************************/
// Class Specification

/** The TTAudioSignal class represents N vectors of audio samples for M channels. 
 
 	All of the members are made public so that direct access to members can be used for
 	speed in cases where efficiency is of the utmost importance.
 
	Where speed is less critical, the preferred method of work with audio signals is the same as for other objects:
	use the dynamic message passing interface.
*/
class TTDSP_EXPORT TTAudioSignal : public TTMatrix {
	TTCLASS_SETUP(TTAudioSignal)

private:
//	TTUInt16		mMaxNumChannels;	///< The number of audio channels for which memory has been allocated.
//	TTUInt16		mVectorSize;		///< Vector Size for this signal.  Every channel in a signal must have the same vector-size.
	TTUInt16		mNumChannels;		///< The number of audio channels that have valid sample values stored in them.
	TTUInt32		mSampleRate;		///< Audio signal metadata, defined in Hertz or set to 0 if not available.

public:
	/**	Attribute accessor. */
	TTErr setMaxNumChannelsWithInt(TTUInt16 newMaxNumChannels);
	TTErr setMaxNumChannels(const TTValue& newMaxNumChannels);
	TTErr getMaxNumChannels(TTValue& returnedMaxNumChannels);

	
	void setSampleRate(const TTUInt32& newSampleRate)
	{
		mSampleRate = newSampleRate;
	}
	
	
	TTUInt32 getSampleRate()
	{
		return mSampleRate;
	}
	
	
	/** Assigns a vector of sample values to a channel in this signal.
	 *	The vector member of this class simply holds a pointer, not a copy of the data.  This makes the 
	 *	operation of this method (and others) fast, but also means that care should be taken to ensure
	 *	that the data being pointed to by this signal is valid, and does not become invalid during the
	 *	lifetime of the signal.
	 *
	 *	It is the responsibility of the user of this method to ensure that the sample-rate and vector-size
	 *	are also set correctly.
	 *	@param		channel			The channel number (zero-based) to assign the vector to.
	 *	@param		vectorSize		The number of samples in the vector.
	 *	@param		newVector		A pointer to the first sample in a vector of samples.
	 *	@result		An error code.																 */
	TTErr setVector(const TTUInt16 channel, const TTUInt16 vectorSize, const TTSampleValuePtr newVector);
	TTErr setVector64(const TTValue& v);	// A version of the above used by the message passing interface.

	
	/**	This version handles vector assignments from 32-bit vectors.
	*/
	TTErr setVector(const TTUInt16 channel, const TTUInt16 vectorSize, const TTFloat32* newVector);
	TTErr setVector32(const TTValue& v);	// A version of the above used by the message passing interface.
	
	TTErr getVectorCopy64(TTValue& v);	// A version of the above used by the message passing interface.
	TTErr getVectorCopy(const TTUInt16 channel, const TTUInt16 theVectorSize, TTSampleValue* returnedVector); // version of getVector that copies
	
	TTErr getVectorCopy32(TTValue& v);	// A version of the above used by the message passing interface.
	TTErr getVectorCopy(const TTUInt16 channel, const TTUInt16 vectorSize, TTFloat32* returnedVector);

	

	TTErr setVectorSize(const TTValue& newVectorSize)
	{
		return setVectorSizeWithInt(TTUInt16(newVectorSize));
	}

	
	TTErr setVectorSizeWithInt(const TTUInt16 newVectorSize)
	{
		TTValue v(TTUInt32(newVectorSize), mDimensions[1]);
		return setDimensions(v);
	}
	
	
	TTUInt16 getVectorSizeAsInt() const
	{
		return mDimensions[0];
	}
	
	
	TTErr getVectorSize(TTValue& returnedMaxNumChannels) const
	{
		returnedMaxNumChannels = getVectorSizeAsInt();
		return kTTErrNone;
	}

	
	TTErr setNumChannels(const TTValue& newNumChannels)
	{
		return setNumChannelsWithInt(TTUInt16(newNumChannels));
	}
	
	
	TTErr setNumChannelsWithInt(const TTUInt16 newNumChannels)
	{
		mNumChannels = TTClip<TTUInt16>(newNumChannels, 0, getMaxNumChannelsAsInt());
		return kTTErrNone;
	}
	
	
	TTUInt16 getNumChannelsAsInt() const
	{
		return mNumChannels;
	}
	
	
	TTUInt16 getMaxNumChannelsAsInt() const;

		
	/**	Copy the audio from one signal into another.	*/
//	static TTErr copy(const TTAudioSignal& source, TTAudioSignal& dest);
	
	/**	Copy the audio from one signal into another.	*/
	static TTErr copy(const TTAudioSignal& source, TTAudioSignal& dest, TTUInt16 channelOffset=0);
	
	/**	Copy the audio from one signal into another, but not taking care to zero channels that aren't used.	*/
	static TTErr copyDirty(const TTAudioSignal& source, TTAudioSignal& dest, TTUInt16 channelOffset=0);

	/**	Copy the audio from one signal into another.	*/
	static TTErr copySubset(const TTAudioSignal& source, TTAudioSignal& dest, TTUInt16 startingChannel=0, TTUInt16 endingChannel=0);
	
	
	
	
	/** Use this class method to determine the least number of channels the two signals have in common.
	 *	In cases where a processAudio method expects to have a matching number of audio inputs and outputs,
	 *	this method can be used to compare the two signals and return the number of channels for which
	 *	it is safe to assume that the number of inputs and outputs are the same.
	 *	@param		signal1			The first of the two signals to be compared.
	 *	@param		signal2			The second of the two signals to be compared.
	 *	@return		The number of channels that are valid for both signal1 and signal2.		*/
	static TTUInt16 getMinChannelCount(const TTAudioSignal& signal1, const TTAudioSignal& signal2);

	/** Use this class method to determine the least number of channels the specified signals have in common.
	 	In cases where a processAudio method expects to have a matching number of audio inputs and outputs,
	 	this method can be used to compare the two signals and return the number of channels for which
	 	it is safe to assume that the number of inputs and outputs are the same.
	 	@param		signal1			The first of three signals to be compared.
	 	@param		signal2			The second of three signals to be compared.
		@param		signal3			The third of three signals to be compared.
	 	@return		The number of channels that are valid for all signals.		*/
	static TTUInt16 getMinChannelCount(const TTAudioSignal& signal1, const TTAudioSignal& signal2, const TTAudioSignal& signal3);

	static TTUInt16 getMaxChannelCount(const TTAudioSignal& signal1, const TTAudioSignal& signal2);
	static TTUInt16 getMaxChannelCount(const TTAudioSignal& signal1, const TTAudioSignal& signal2, const TTAudioSignal& signal3);
	
	/** Use this class method to determine the number of channels of an input or output signal.
	 *	This can be useful in circumstances where input and output signals are not necsessarily expected
	 *  or required to have the same number of channels.
	 *	@param		signal			The signal that we want to investigate.
	 *	@return		The number of channels of the signal.		*/
	static TTUInt16 getNumChannels(const TTAudioSignal& signal);
	
	
	/**	Sum another audio signal's samples with this audio signal's samples.
	 */
	TTAudioSignal& operator += (const TTAudioSignal& rightHandValue)
	{
		short			vs;
		TTSampleValue	inSample;
		TTSampleValue	outSample;
		short			channelCount = getMaxChannelCount(*this, rightHandValue);
		short			channel;
		
		if (channelCount > getMaxNumChannelsAsInt())
			channelCount = getMaxNumChannelsAsInt();
		if (channelCount > rightHandValue.getMaxNumChannelsAsInt())
			channelCount = rightHandValue.getMaxNumChannelsAsInt();
		
		if (getVectorSizeAsInt() > rightHandValue.getVectorSizeAsInt())
			vs = rightHandValue.getVectorSizeAsInt();
		else
			vs = getVectorSizeAsInt();

		for (channel=0; channel<channelCount; channel++) {
			//inSample = rightHandValue.mSampleVectors[channel];
			//outSample = mSampleVectors[channel];
			
			for (int i=0; i<vs; i++) {
				get2d(i, channel, inSample);
				get2d(i, channel, outSample);
				
				//(*outSample) = (*outSample) + (*inSample);
				outSample += inSample;
				
				//outSample++;
				//inSample++;
				set2d(i, channel, outSample);
			}
		}
		return *this;
	}
	
	
	/**	Assign another audio signal's samples and channel/vector configuration with this audio signal's samples. */
	TTAudioSignal& operator = (const TTAudioSignal& rightHandValue)
	{	
		setMaxNumChannelsWithInt(rightHandValue.getMaxNumChannelsAsInt());
		setVectorSizeWithInt(rightHandValue.getVectorSizeAsInt());
		mNumChannels = rightHandValue.mNumChannels;
		mSampleRate = rightHandValue.mSampleRate;
		TTAudioSignal::copy(rightHandValue, *this);
		return *this;
	}

};


typedef TTAudioSignal* TTAudioSignalPtr;


#endif // __TT_AUDIO_SIGNAL_H__

