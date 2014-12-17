#include "std.h"
#include "memory.h"
#include "sound.h"
#include "mixer.h"
#include "USBHostMSD.h"

DigitalOut p(LED1);

struct s {
	const char *file;
	nat sample;
};

int main() {
	memory();

	USBHostMSD msd("usb");
	while (!msd.connect()) {
		Thread::wait(500);
	}

	// s file = { "/usb/sound.raw", 8000 };
	s file = { "/usb/h1.raw", 44100 };
	// s file = { "/usb/h2.raw", 44100 };
	// s file = { "/usb/s44100.raw", 44100 };
	// s file = { "/usb/s22000.raw", 22000 };
	// s file = { "/usb/s11000.raw", 11000 };

	RawFile *f = new RawFile(file.file);

	memory();

	AnalogOut out(p18);
	startMixer(out, file.sample);
	printf("Created output\n");
	play(f);

	memory();

	//output.add(new SquareWave());

	while (true) {
		p = !p;
		Thread::wait(400);
	}

	return 0;
}

