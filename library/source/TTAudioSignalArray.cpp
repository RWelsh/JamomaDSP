/* 
 * TTBlue Audio Signal Class
 * Copyright © 2008, Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTDSP.h"

#define thisTTClass			TTAudioSignalArray
#define thisTTClassName		"audiosignalarray"
#define thisTTClassTags		"audio, processor, dynamics, envelope"


/****************************************************************************************************/

TT_OBJECT_CONSTRUCTOR, 
	audioSignals(NULL),
	maxNumAudioSignals(0),
	numAudioSignals(0)
{
	TTUInt16 initialMaxNumAudioSignals = arguments;
	
	// TT_ASSERT(audio_signal_array_has_valid arg, initialMaxNumAudioSignals > 0);
	// Can't assert because, for example, an audio graph can have objects with no inlets
	// So we just enforce a lower-limit of 1:

	if (initialMaxNumAudioSignals < 1)
		initialMaxNumAudioSignals = 1;
	setMaxNumAudioSignals(initialMaxNumAudioSignals);
}


TTAudioSignalArray::~TTAudioSignalArray()
{
	delete[] audioSignals;
}


void TTAudioSignalArray::init()
{
	delete[] audioSignals;
	audioSignals = new TTAudioSignalPtr[maxNumAudioSignals];
	for (TTUInt16 i=0; i<maxNumAudioSignals ;i++)
		audioSignals[i] = NULL;
	numAudioSignals = 0;
}


void TTAudioSignalArray::releaseAll()
{
	for (TTUInt16 i=0; i<maxNumAudioSignals; i++)
		TTObjectRelease(&audioSignals[i]);
}

void TTAudioSignalArray::setVectorSize(TTUInt16 vs)
{
	for (TTUInt16 i=0; i<maxNumAudioSignals; i++)
		audioSignals[i]->setVectorSizeWithInt(vs);
}

TTUInt16 TTAudioSignalArray::getVectorSize()
{
	return audioSignals[0]->getVectorSizeAsInt();
}

void TTAudioSignalArray::setAllMaxNumChannels(TTUInt16 newMaxNumChannels)
{
	for (TTUInt16 i=0; i<maxNumAudioSignals; i++)
		audioSignals[i]->setMaxNumChannels(newMaxNumChannels);
}

void TTAudioSignalArray::setAllNumChannels(TTUInt16 newNumChannels)
{
	for (TTUInt16 i=0; i<maxNumAudioSignals; i++)
		audioSignals[i]->setNumChannels(newNumChannels);
}

