#pragma once
#include "std.h"


/**
 * Base class for anything providing sound.
 */
class Sound {
public:
	Sound();
	virtual ~Sound();

	// Get one sample. Return 'false' if at the end.
	virtual bool data(byte *to) = 0;

	// The samplefreq (in Hz).
	nat samplefreq;

	// Set up this object to resample to a sampletime of 'time' us.
	void resampleTo(nat samplerate);

	// Get 'n' samples, resampled. Samples are added to 'to'. Returns false if done.
	bool addResampled(ushort *to, nat count);

private:
	// Resampling data.
	// 'skipIncrease' and 'skipTarget' together represents a fraction
	// of how often to skip samples. Each sample, 'skipIncrease' is added
	// to 'skipCounter'. When 'skipCounter' >= 'skipTarget', a sample is
	// skipped.
	nat repeatCounter;
	nat repeatIncrease;
	nat repeatTarget;

	// Last value (as a float).
	float last;

	// Done?
	bool done;

	// Get one float. Returns 0xFF/2 if done.
	ushort next();
};

/**
 * Generate a simple square wave.
 */
class SquareWave : public Sound {
public:
	SquareWave(nat freq, nat length = 0);

	virtual bool data(byte *to);

private:
	bool pos;
	nat length;
};

/**
 * Read from a .raw file (8-bit mono).
 */
class RawFile : public Sound {
public:
	RawFile(const char *name, nat samplefreq);
	~RawFile();

	virtual bool data(byte *to);

private:
	FILE *fp;
	bool eof;
};


