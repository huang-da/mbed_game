#include "sound.h"
#include <assert.h>

static const ushort zero = 0xFF/2;

Sound::Sound() : samplefreq(1), last(zero), done(false) {}

Sound::~Sound() {}

void Sound::resampleTo(nat rate) {
	if (1000000 / rate < samplefreq) {
		double toFreq = 1000000.0 / rate;
		printf("Trying to downsample from %u to %f Hz, downsampling is not supported!\n",
			samplefreq, toFreq);
		assert(false);
		return;
	}

	nat difference = 1000000 - rate * samplefreq;
	repeatTarget = rate * samplefreq;
	repeatIncrease = difference;
	repeatCounter = 0;

	// double repRate = double(repeatIncrease) / repeatTarget;
	// printf("Resampling %u us (%f Hz) to %u Hz:\n", rate, toFreq, samplefreq);
	// printf("Repeat when n*%u >= %u (every %f)\n", repeatIncrease, repeatTarget, repRate);
}

bool Sound::addResampled(ushort *to, nat count) {
	// float t = 1.0f / repeatTarget;
	for (nat i = 0; i < count; i++) {
		if (repeatCounter >= repeatTarget) {
			to[i] += last;
			repeatCounter -= repeatTarget;
		} else {
			// ushort now = next();
			// to[i] += ushort(frac*last + (1.0f-frac)*now);
			// last = now;

			to[i] += last = next();

			repeatCounter += repeatIncrease;
		}
	}

	return !done;
}

ushort Sound::next() {
	if (done)
		return zero;

	byte out;
	if (!data(&out)) {
		done = true;
		return zero;
	}

	return out;
}


SquareWave::SquareWave(nat freq, nat length) : pos(false), length(length) {
	samplefreq = freq * 2;
}

bool SquareWave::data(byte *to) {
	*to = pos ? 0xFF : 0x00;
	pos = !pos;

	if (length == 0)
		return true;
	return --length > 0;
}

static Mutex fileMutex;

RawFile::RawFile(const char *name, nat samplefreq) {
	fileMutex.lock();
	this->samplefreq = samplefreq;
	fp = fopen(name, "r");
	eof = false;
	fileMutex.unlock();
}

RawFile::~RawFile() {
	fclose(fp);
}

bool RawFile::data(byte *to) {
	if (eof)
		return false;

	fileMutex.lock();
	nat pos = fread(to, sizeof(*to), 1, fp);
	fileMutex.unlock();
	eof = pos == 0;
	return true;
}

