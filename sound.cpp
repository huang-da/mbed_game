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
	fileMutex.lock();
	fclose(fp);
	fileMutex.unlock();
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

WavFile::WavFile(const char *name) {
	fileMutex.lock();
	fp = fopen(name, "r");
	size = pos = 0;

	if (!loadFile()) {
		pos = size;
	}

	fileMutex.unlock();
}

WavFile::~WavFile() {
	fileMutex.lock();
	fclose(fp);
	fileMutex.unlock();
}

bool WavFile::data(byte *to) {
	if (pos == size)
		return false;

	fileMutex.lock();
	fread(to, sizeof(*to), 1, fp);
	fileMutex.unlock();
	pos++;
	return true;
}

void endian(nat &v) {
	nat t = (v & 0xFF) << 16;
	t |= (v & 0xFF00) << 8;
	t |= (v & 0xFF0000) >> 8;
	t |= (v & 0xFF000000) >> 16;
	// v = t;
}

void endian(short &v) {
	short t = (v & 0xFF) << 8;
	t |= (v & 0xFF00) >> 8;
	// v = t;
}

bool WavFile::loadFile() {
	char header[5];
	fread(header, 1, 4, fp);
	if (strcmp(header, "RIFF") != 0) {
		printf("Invalid header: %s\n", header);
		return false;
	}

	// Skip total file size.
	fread(header, 1, 4, fp);

	// Read WAVE letters.
	fread(header, 1, 4, fp);
	if (strcmp(header, "WAVE") != 0) {
		printf("Expected WAVE, got %s.\n", header);
		return false;
	}

	// Now for the fmt subchunk.
	fread(header, 1, 4, fp);
	if (strcmp(header, "fmt ") != 0) {
		printf("Expected fmt , got %s.\n", header);
		return false;
	}

	nat fmtSize;
	fread(&fmtSize, 4, 1, fp);
	endian(fmtSize);
	if (fmtSize != 16) {
		printf("Invalid size: %u\n", fmtSize);
		return false;
	}

	short fmt;
	fread(&fmt, 2, 1, fp);
	endian(fmt);
	if (fmt != 1) {
		printf("Only PCM is supported, got %d.\n", int(fmt));
		return false;
	}

	short channels;
	fread(&channels, 2, 1, fp);
	endian(channels);
	if (channels != 1) {
		printf("Currently only MONO is supported, tried to play %d channels.\n", int(channels));
		return false;
	}

	fread(&samplefreq, 4, 1, fp);
	endian(samplefreq);

	nat byteRate;
	fread(&byteRate, 4, 1, fp);
	endian(byteRate);

	short blockAlign;
	fread(&blockAlign, 2, 1, fp);
	endian(blockAlign);
	// No checks needed.

	short bitsPerSample;
	fread(&bitsPerSample, 2, 1, fp);
	endian(bitsPerSample);
	if (bitsPerSample != 8) {
		printf("Currently only 8 bits per sample is supported, not %d.\n", int(bitsPerSample));
		return false;
	}


	// Data chunk!
	fread(header, 1, 4, fp);
	if (strcmp(header, "data") != 0) {
		printf("Expected data, got %s.\n", header);
		return false;
	}

	// Size of data.
	fread(&size, 4, 1, fp);
	endian(size);

	return true;
}
