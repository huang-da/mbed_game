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

// Current playback volume (0-0x100)
static nat volume = 0x50;

// Interrupt timer used.
static Ticker ticker;

// Current worker thread (if any).
static Thread *workerThread = 0;

// Streams currently playing.
static vector<Sound *> streams;

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
	printf("Buffer part time: %u ms\n", bufferPartTime);

	clearBuffer();
	ticker.attach_us(interrupt, timestamp_t(period));
}

// Fill a part of the buffer.
static void fill(nat partOffset, nat size) {
	streamsLock.lock();

	if (streams.size() == 0) {
		clearPart(partOffset, size);
	} else {
		for (nat i = 0; i < streams.size(); i++) {
			Sound *s = streams[i];
			if (s->finished()) {
				streams.erase(streams.begin() + i);
				i--;
			} else {
				s->data(buffer + partOffset, size);
			}
		}
	}

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
	streams.push_back(sound);
	streamsLock.unlock();
}
