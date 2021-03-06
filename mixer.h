#pragma once
#include "std.h"
#include "sound.h"

// Start playback.
void startMixer(AnalogOut &output, nat samplerate);

// Set volume control.
void setVolume(AnalogIn &input);

// Stop playback.
void stopMixer();

// Play 'sound' until it is finished.
void play(Sound *sound);

// Is 'sound' playing.
bool playing(Sound *sound);

// Stop 'sound'.
void stop(Sound *sound);

// Mute 'sound'.
void pause(Sound *sound, bool pause);

/**
 * Configuration.
 */
namespace mixer {
	// Number of parts in the buffer. The part
	// being played is locked, and can not be updated.
	const nat bufferParts = 3;

	// Size of each part.
	const nat bufferPartSize = 256;

    // Length of buffer in bytes.
	const nat bufferSize = bufferPartSize * bufferParts;

    // Stack size for worker thread.
	const nat stackSize = 1024;

	// Maxium number of playing streams.
	const nat streams = 5;
}
