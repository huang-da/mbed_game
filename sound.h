#pragma once
#include "std.h"


/**
 * Base class for anything providing sound.
 */
class Sound {
public:
	virtual ~Sound();
	virtual void data(byte *to, nat size) = 0;
	virtual bool finished() = 0;
};

/**
 * Generate a simple square wave.
 */
class SquareWave : public Sound {
public:
	SquareWave();

	virtual void data(byte *to, nat size);
	virtual bool finished();

private:
	bool pos;
};

/**
 * Read from a .raw file (8-bit mono).
 */
class RawFile : public Sound {
public:
	RawFile(const char *name);
	~RawFile();

	virtual void data(byte *to, nat size);
	virtual bool finished();

private:
	FILE *fp;
};


/**
 * Singleton output object for sound. Starts
 * interrupts for the given samplerate on the given
 * output.
 */
class Output {
public:
	Output(PinName pin, nat samplerate);
	~Output();

	// Add an output.
	void add(Sound *s);

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

	// ms-length of the buffer
	nat bufferTime;

	// Current position in the buffer with samples.
	nat playPos;

	// All sources.
	vector<Sound *> src;

	// Sync the 'src'.
	Mutex srcMutex;

	// Thread.
	Thread *workerThread;

	// Output the next sample.
	void sample();

	// Allocate a buffer.
	void createBuffer();

	// Start the interrupt.
	void startInterrupt();

	// Entry point for the buffer fill.
	void fillThread();

	// Fill our buffer.
	void fill(byte *to, nat size);

	friend void startWorker(void const *p);
};
