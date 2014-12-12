#include "sound.h"

// Size of the playback buffer.
static const nat playbackBuffer = 1024;

Output::Output(PinName pin, nat samplerate) : output(pin), samplerate(samplerate) {
	createBuffer();
	startInterrupt();
}

void Output::startInterrupt() {
	nat period = 1000000 / samplerate;
	ticker.attach_us(this, &Output::sample);
}

void Output::createBuffer() {
	bufferSize = playbackBuffer;
	buffer = new byte[bufferSize];
	playPos = 0;
	for (nat i = 0; i < bufferSize; i++) {
		buffer[i] = 0;
	}
}

void Ouput::sample() {
	unsigned short v = buffer[playPos];
	v |= v << 8;
	output.write_u16(v);
	if (++playPos == bufferSize)
		playPos = 0;
}
