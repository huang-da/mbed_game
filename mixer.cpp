#include "mixer.h"
#include <assert.h>

using namespace mixer;

// Static buffer for playback.
static byte buffer[bufferSize];

// Total time for the current samplerate for one part of the buffer (ms).
static nat bufferPartTime;

// Stack space for playback.
static byte stackBuf[stackSize];

// Output used.
static AnalogOut *output = 0;

// Volume input.
static AnalogIn *vol = 0;

// Current playback position.
static nat playPos = 0;
static nat playPartPos = 0;
static nat playPartId = 0;

// Buffer used when mixing.
static ushort mixBuffer[bufferPartSize];

// Current playback volume (0-0x100)
static nat volume = 0x100;

// Interrupt timer used.
static Ticker ticker;

// Ticker interval (us).
static nat tickerInterval;

// Current worker thread (if any).
static Thread *workerThread = 0;

// Streams currently playing.
static Sound *stream[streams];
static bool paused[streams] = { false };

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
	if (++playPartPos >= bufferPartSize) {
		playPartPos = 0;
		workerThread->signal_set(1 << playPartId);
		if (++playPartId >= bufferParts)
			playPartId = 0;
	}
}

// Start the interrupt generating sound.
static void startInterrupt(nat samplerate) {
	playPos = 0;
	playPartPos = 0;
	playPartId = 0;
	nat period = 1000000 / samplerate;
	if (period < 10) {
		printf("This high samplerate is probably not a good idea!\n");
		assert(false);
	}
	tickerInterval = period;
	bufferPartTime = period * bufferPartSize / 1000;
	printf("Sample period: %u us\n", period);
	printf("Sample frequency: %f Hz\n", 1000000.0 / period);
	printf("Buffer part time: %u ms\n", bufferPartTime);

	clearBuffer();
	ticker.attach_us(interrupt, timestamp_t(period));
}

// Fill the mix buffer with new data.
static void fillMix() {
	// clear
	for (nat i = 0; i < bufferPartSize; i++) {
		mixBuffer[i] = 0;
	}

	streamsLock.lock();

	nat used = 0;
	for (nat i = 0; i < streams; i++) {
		if (paused[i])
			continue;

		Sound *s = stream[i];
		if (!s)
			continue;

		used++;
		if (!s->addResampled(mixBuffer, bufferPartSize)) {
			stream[i] = 0;
			delete s;
		}
	}

	streamsLock.unlock();

	// Normalize.
	if (used >= 2) {
		ushort dec = (used - 1) * 0xFF / 2;
		for (nat i = 0; i < bufferPartSize; i++) {
			ushort &v = mixBuffer[i];

			if (v > dec)
				v -= dec;
			else
				v = 0;

			if (v > 0xFF)
				v = 0xFF;
			//mixBuffer[i] /= used;
		}
	}

}

// Copy the mix buffer into the new place.
static void fill(nat partOffset) {
	for (nat i = 0; i < bufferPartSize; i++)
		buffer[partOffset + i] = byte(mixBuffer[i]);
}

// The thread keeping the buffer filled.
static void fillThread(void const *p) {
	while (true) {
		for (nat pos = 0; pos < bufferParts; pos++) {
			fillMix();
			Thread::signal_wait(1 << pos);
			fill(pos * bufferPartSize);

			// Update volume.
			if (vol)
				volume = vol->read_u16() >> 8;
		}
	}
}

// Start.
void startMixer(AnalogOut &out, nat samplerate) {
	output = &out;
	workerThread = new Thread(fillThread, 0, osPriorityRealtime, stackSize, stackBuf);
	startInterrupt(samplerate);
}

// Set volume.
void setVolume(AnalogIn &v) {
	vol = &v;
}

// Stop.
void stopMixer() {
	ticker.detach();
	workerThread->terminate();
}

void play(Sound *sound) {
	bool added = false;
	sound->resampleTo(tickerInterval);
	streamsLock.lock();
	for (nat i = 0; i < streams; i++) {
		if (!stream[i]) {
			stream[i] = sound;
			paused[i] = false;
			added = true;
			break;
		}
	}
	streamsLock.unlock();
	if (!added)
		delete sound;
}

void stop(Sound *sound) {
	bool found = false;
	streamsLock.lock();
	for (nat i = 0; i < streams; i++) {
		if (stream[i] == sound) {
			found = true;
			stream[i] = 0;
		}
	}
	streamsLock.unlock();
	if (found)
		delete sound;
}

bool playing(Sound *sound) {
	if (sound == 0)
		return false;
	for (nat i = 0; i < streams; i++)
		if (stream[i] == sound)
			return true;
	return false;
}

void pause(Sound *sound, bool pause) {
	for (nat i = 0; i < streams; i++) {
		if (stream[i] == sound)
			paused[i] = pause;
	}
}
