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


