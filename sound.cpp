#include "sound.h"
#include <assert.h>

static const ushort zero = 0xFF/2;
static const nat largeNat = 0x80000000;
static const nat largeNatBits = 31;

Sound::Sound() : samplefreq(1), last(zero), done(false), bufferPos(0) {}

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

#if HQ_SOUND == 1
	repeatTargetInv = 1.0f / repeatTarget;
#elif HQ_SOUND == 2
	repeatTargetInv = largeNat / repeatTarget;
#endif

	// double repRate = double(repeatIncrease) / repeatTarget;
	// printf("Resampling %u us (%f Hz) to %u Hz:\n", rate, toFreq, samplefreq);
	// printf("Repeat when n*%u >= %u (every %f)\n", repeatIncrease, repeatTarget, repRate);
}

bool Sound::addResampled(ushort *to, nat count) {
	for (nat i = 0; i < count; i++) {
		if (repeatCounter >= repeatTarget) {
			to[i] += last;
			repeatCounter -= repeatTarget;
		} else {
#if HQ_SOUND == 1
			float frac = repeatTargetInv * repeatCounter;
			ushort now = next();
			to[i] += ushort(frac*last + (1.0f-frac)*now);
			last = now;
#elif HQ_SOUND == 2
			// ushort frac = (repeatCounter * 0x100) / repeatTarget;
			// ushort frac = (repeatCounter * repeatTargetInv) / (largeNat / 0x100);
			ushort frac = (repeatCounter * repeatTargetInv) >> (largeNatBits - 8);
			ushort now = next();
			ushort sample = frac*last + (0x100 - frac)*now;
			to[i] += ushort(sample >> 8);
			last = now;
#elif HQ_SOUND == 3
			to[i] += last = next();
#endif

			repeatCounter += repeatIncrease;
		}
	}

	return !done;
}

ushort Sound::next() {
	if (bufferPos >= bufferSize) {
		if (done || !data(buffer, bufferSize)) {
			for (nat i = 0; i < bufferSize; i++)
				buffer[i] = zero;
			done = true;
		}
		bufferPos = 0;
	}

	return buffer[bufferPos++];
}


SquareWave::SquareWave(nat freq, nat length) : pos(false), length(length) {
	samplefreq = freq * 2;
}

bool SquareWave::data(byte *to, nat size) {
	if (length == 0) {
		for (nat i = 0; i < size; i++) {
			to[i] = pos ? 0xFF : 0x00;
			pos = !pos;
		}
		return true;
	} else {
		for (nat i = 0; i < size; i++) {
			if (length > 0) {
				length--;
				to[i] = pos ? 0xFF : 0x00;
				pos = !pos;
			} else {
				to[i] = zero;
			}
		}
		return length == 0;
	}
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

bool RawFile::data(byte *to, nat size) {
	if (eof)
		return false;

	fileMutex.lock();
	nat r = fread(to, sizeof(*to), size, fp);
	fileMutex.unlock();
	if (r != size) {
		eof = true;
		for (nat i = r; i < size; i++)
			to[i] = zero;
	}
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

bool WavFile::data(byte *to, nat count) {
	if (pos == size)
		return false;

	fileMutex.lock();
	nat r = fread(to, sizeof(*to), count, fp);
	fileMutex.unlock();

	if (r < count) {
		for (nat i = r; i < count; i++)
			to[i] = zero;
		pos = size;
	} else {
		pos += r;
	}
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
