#pragma once
#include "std.h"

class Sound {
public:
};


/**
 * Singleton output object for sound. Starts
 * interrupts for the given samplerate on the given
 * output.
 */
class Output {
public:
	Output(PinName pin, nat samplerate);

private:
	// Output pin.
	AnalogOut output;

	// Samples/s
	nat samplerate;

	// Ticker object (maybe not neccessary to keep alive).
	Ticker ticker;

	// Buffer with samples
	byte *buffer;

	// Size of the buffer
	nat bufferSize;

	// Current position in the buffer with samples.
	nat playPos;

	// Output the next sample.
	void sample();

	// Allocate a buffer.
	void createBuffer();

	// Start the interrupt.
	void startInterrupt();
};
