#pragma once
#include "std.h"

// High quality resampling (uses much CPU), but with noticeable
// difference for example when resampling from 44100Hz to 45454.54..Hz (22us)
// There are different levels of the HQ-sound implementation:
// 1 - best, uses floats to interpolate samples
// 2 - good, uses fixed-point to interpolate samples. Should be as good as #1, but not as easy to understand.
// 3 - none, noticeable worse than any other.
//
// Approx CPU when playing 1 track at 44100 Hz (upsampled to 22us):
// 1 : 66%
// 2 : 57%
// 3 : 48%
#define HQ_SOUND 2

/**
 * Base class for anything providing sound.
 */
class Sound {
public:
	Sound();
	virtual ~Sound();

	// The samplefreq (in Hz).
	nat samplefreq;

	// Set up this object to resample to a sampletime of 'time' us.
	void resampleTo(nat samplerate);

	// Get 'n' samples, resampled. Samples are added to 'to'. Returns false if done.
	bool addResampled(ushort *to, nat count);

protected:
	// Get one sample. Return 'false' if at the end. (no data is filled then).
	virtual bool data(byte *to, nat count) = 0;

private:
	// Resampling data.
	// 'skipIncrease' and 'skipTarget' together represents a fraction
	// of how often to skip samples. Each sample, 'skipIncrease' is added
	// to 'skipCounter'. When 'skipCounter' >= 'skipTarget', a sample is
	// skipped.
	nat repeatCounter;
	nat repeatIncrease;
	nat repeatTarget;

	// Interpolation variable.
#if HQ_SOUND == 1
	float repeatTargetInv;
#elif HQ_SOUND == 2
	nat repeatTargetInv;
#endif

	// Last value (as a float).
	float last;

	// Done?
	bool done;

	// Get one float. Returns 0xFF/2 if done.
	ushort next();

	// Buffer size.
	enum { bufferSize = 32 };

	// Internal data buffer.
	byte buffer[bufferSize];

	// Position in buffer.
	nat bufferPos;
};

/**
 * Generate a simple square wave.
 */
class SquareWave : public Sound {
public:
	SquareWave(nat freq, nat length = 0);

protected:
	virtual bool data(byte *to, nat size);

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

protected:
	virtual bool data(byte *to, nat size);

private:
	FILE *fp;
	bool eof;
};


/**
 * Read from a .wav file (currently only supports 8-bit mono).
 */
class WavFile : public Sound {
public:
	WavFile(const char *name, bool repeat = false);
	~WavFile();

protected:
	virtual bool data(byte *to, nat size);

private:
	FILE *fp;

	nat pos, size;

	bool repeat;

	bool loadFile();
};
