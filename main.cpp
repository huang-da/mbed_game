#include "std.h"
#include "cpu.h"
#include "memory.h"
#include "sound.h"
#include "mixer.h"
#include "USBHostMSD.h"

DigitalOut p(LED1);
DigitalIn sound(p14);

struct s {
	const char *file;
	nat sample;
};

int main() {
	measureCpu();

	// memory();

	USBHostMSD msd("usb");
	while (!msd.connect()) {
		Thread::wait(500);
	}

	// s file = { "/usb/sound.raw", 8000 };
	// s file = { "/usb/h1.raw", 44100 };
	// s file = { "/usb/h2.raw", 44100 };
	// s file = { "/usb/h1_2.raw", 20000 };
	// s file = { "/usb/h2_2.raw", 20000 };
	// s file = { "/usb/s44100.raw", 44100 };
	// s file = { "/usb/s22000.raw", 22000 };
	// s file = { "/usb/s11000.raw", 11000 };
	// RawFile *f = new RawFile(file.file, file.sample);

	// WavFile *f = new WavFile("/usb/h1.wav");
	// WavFile *f = new WavFile("/usb/h1_2.wav");
	// WavFile *f = new WavFile("/usb/h2.wav");
	// WavFile *f = new WavFile("/usb/h2_2.wav");
	// WavFile *f = new WavFile("/usb/staple.wav");
	// WavFile *f = new WavFile("/usb/s44100.wav");
	// WavFile *f = new WavFile("/usb/s22000.wav");
	// WavFile *f = new WavFile("/usb/s11000.wav");
	//WavFile *f = new WavFile("/usb/sound.wav", true);
	// WavFile *f = new WavFile("/usb/sfx1_3.wav", true);
	// SquareWave *f = new SquareWave(440);

	WavFile *f = new WavFile("/usb/smg2.wav", true);
	// memory();

	AnalogIn vol(p19);
	AnalogOut out(p18);
	setVolume(vol);
	startMixer(out, f->samplefreq);
	printf("Created output\n");
	play(f);

	// memory();

	//output.add(new SquareWave());

	bool last = false;
	bool now = false;
	while (true) {
		p = !p;
		Thread::wait(400);

		last = now;
		now = sound;
		if (now && !last) {
			play(new WavFile("/usb/sfx1_3.wav"));
			// play(new WavFile("/usb/sound.wav"));
		}
	}

	return 0;
}

