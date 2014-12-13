#include "sound.h"

Sound::~Sound() {}

SquareWave::SquareWave() : pos(false) {}

bool SquareWave::finished() { return false; }

void SquareWave::data(byte *to, nat size) {
	for (nat i = 0; i < size; i++) {
		to[i] = pos ? 0xFF : 0x00;
		pos = !pos;
	}
}

RawFile::RawFile(const char *name) {
	fp = fopen(name, "r");
}

RawFile::~RawFile() {
	fclose(fp);
}

void RawFile::data(byte *to, nat size) {
	nat pos = fread(to, sizeof(*to), size, fp);
	for (; pos < size; pos++)
		to[pos] = 0;
}

bool RawFile::finished() {
	if (feof(fp))
		return true;
	return false;
}

// Size of the playback buffer.
static const nat playbackBuffer = 1024 * 10;
static const nat bufferChunks = 2;
static const nat volume = 0x20; // 0x100 is max.

void startWorker(void const *p) {
	Output *o = (Output *)p;
	o->fillThread();
}

Output::Output(PinName pin, nat samplerate) : output(pin), samplerate(samplerate) {
	createBuffer();
	startInterrupt();

	workerThread = new Thread(startWorker, this, osPriorityRealtime);
}

Output::~Output() {
	ticker.detach();
	workerThread->terminate();

	for (nat i = 0; i < src.size(); i++)
		delete src[i];
	delete workerThread;
	delete []buffer;
}

void Output::add(Sound *s) {
	srcMutex.lock();
	src.push_back(s);
	srcMutex.unlock();
}

void Output::startInterrupt() {
	nat period = 1000000 / samplerate;
	ticker.attach_us(this, &Output::sample, timestamp_t(period));
}

void Output::createBuffer() {
	bufferSize = playbackBuffer;
	nat period = 1000 / samplerate;
	bufferTime = bufferSize * period;
	buffer = new byte[bufferSize];
	playPos = 0;
	for (nat i = 0; i < bufferSize; i++) {
		buffer[i] = 0;
	}
}

void Output::sample() {
	unsigned short v = buffer[playPos];
	v *= volume;
	// v |= v << 8;
	output.write_u16(v);

	if (++playPos >= bufferSize)
	 	playPos = 0;
}

void Output::fillThread() {
	nat pos = 0;
	nat stride = bufferSize / bufferChunks;

	while (true) {
		nat play = playPos;
		if (play < pos || play >= pos + stride) {
			fill(buffer + pos, stride);
			pos += stride;
			if (pos >= bufferSize)
				pos -= bufferSize;
		}

		// Thread::wait(bufferTime / 4);
		Thread::wait(400);
	}
}

void Output::fill(byte *start, nat bytes) {
	srcMutex.lock();

	for (nat i = 0; i < src.size(); i++) {
		Sound *s = src[i];
		if (s->finished()) {
			src.erase(src.begin() + i);
			i--;
			continue;
		}

		s->data(start, bytes);
	}

	srcMutex.unlock();
}
