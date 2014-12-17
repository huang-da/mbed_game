#include "mixer.h"

using namespace mixer;

// Static buffer for playback.
static byte buffer[bufferSize];

// Total time for the current samplerate for one part of the buffer (ms).
static nat bufferPartTime;

// Stack space for playback.
static byte stackBuf[stackSize];

// Output used.
static AnalogOut *output = 0;

// Current playback position.
static nat playPos = 0;

// Counters to skip ticks when needed. For example,
// a samplerate of 44100Hz yields a period of 22us, when it should
// be 22.67us. To compensate for this, we ignore some ticks.
// The difference between the actual samplerate and the
// desired samplerate can be expressed as: z/s, where 1000000 = u * s + z (s is the samplerate).
// From this we can conclude that we want to skip every n = u / (z/s) samples. To count
// in whole numbers we can write it like this: n * z = u * s, where n * z can be a counter
// that is incremented by z each sample and a sample is then skipped whenever the counter
// is larger than u * s.
static nat skipCounter = 0;
static nat skipIncrease = 0;
static nat skipTarget = 0;


// Current playback volume (0-0x100)
static nat volume = 0x50;

// Interrupt timer used.
static Ticker ticker;

// Current worker thread (if any).
static Thread *workerThread = 0;

// Streams currently playing.
static Sound *stream[streams];

// Lock for the 'streams' above.
static Mutex streamsLock;

// Clear a part of the buffer.
static void clearPart(nat offset, nat size) {
	for (nat i = 0; i < size; i++)
		buffer[offset + i] = 0;
}

// Initialize the buffer.
static void clearBuffer() {
	clearPart(0, bufferSize);
}

// Interrupt routine.
static void interrupt() {
	skipCounter += skipIncrease;
	if (skipCounter >= skipTarget) {
		skipCounter -= skipTarget;
		return;
	}

	unsigned short v = buffer[playPos];
	v *= volume;
	output->write_u16(v);

	if (++playPos >= bufferSize)
		playPos = 0;
}

// Start the interrupt generating sound.
static void startInterrupt(nat samplerate) {
	playPos = 0;
	nat period = 1000000 / samplerate;
	bufferPartTime = period * bufferPartSize / 1000;

	nat remainder = 1000000 - period * samplerate;
	skipTarget = period * samplerate;
	skipIncrease = remainder;
	skipCounter = 0;

	printf("Remainder: %u\n", remainder);
	printf("Period: %u us\n", period);
	printf("Skip: +%u, to %u (skip every %f sample)\n", skipIncrease, skipTarget, skipTarget / float(skipIncrease));
	printf("Buffer part time: %u ms\n", bufferPartTime);

	clearBuffer();
	ticker.attach_us(interrupt, timestamp_t(period));
}

// Fill a part of the buffer.
static void fill(nat partOffset, nat size) {
	streamsLock.lock();

	bool any = false;

	for (nat i = 0; i < streams; i++) {
		Sound *s = stream[i];
		if (!s) {
		} else if (s->finished()) {
			stream[i] = 0;
			delete s;
			i--;
		} else {
			s->data(buffer + partOffset, size);
			any = true;
		}
	}

	if (!any)
		clearPart(partOffset, size);

	streamsLock.unlock();
}

// The thread keeping the buffer filled.
static void fillThread(void const *p) {
	nat pos = 0;

	while (true) {
		nat play = playPos;
		if (play < pos || play >= pos + bufferPartSize) {
			fill(pos, bufferPartSize);
			pos += bufferPartSize;
			if (pos >= bufferSize)
				pos -= bufferSize;
		}

		Thread::wait(bufferPartTime / 2);
	}
}

// Start.
void startMixer(AnalogOut &out, nat samplerate) {
	output = &out;
	startInterrupt(samplerate);
	workerThread = new Thread(fillThread, 0, osPriorityNormal, stackSize, stackBuf);
}

// Stop.
void stopMixer() {
	ticker.detach();
	workerThread->terminate();
}

void play(Sound *sound) {
	streamsLock.lock();
	for (nat i = 0; i < streams; i++) {
		if (!stream[i]) {
			stream[i] = sound;
			break;
		}
	}
	streamsLock.unlock();
}
