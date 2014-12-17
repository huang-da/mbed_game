#pragma once
#include "std.h"
#include "sound.h"

// Start playback.
void startMixer(AnalogOut &output, nat samplerate);

// Stop playback.
void stopMixer();

// Play 'sound' until it is finished.
void play(Sound *sound);


/**
 * Configuration.
 */
namespace mixer {
	// Number of parts in the buffer. The part
	// being played is locked, and can not be updated.
	const nat bufferParts = 2;

	// Size of each part.
	const nat bufferPartSize = 1024;

    // Length of buffer in bytes.
	const nat bufferSize = bufferPartSize * bufferParts;

    // Stack size for worker thread.
	const nat stackSize = 1024;

	// Maxium number of playing streams.
	const nat streams = 5;
}
