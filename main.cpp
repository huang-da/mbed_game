#include "std.h"
#include "sound.h"
#include "USBHostMSD.h"

DigitalOut p(LED1);

void memory() {
	// perform free memory check
	int blockSize = 16;
	int i = 1;
	printf("Checking memory with blocksize %d char ...\n", blockSize);
	while (true) {
		char *p = (char *) malloc(i * blockSize);
		if (p == NULL)
			break;
		free(p);
		++i;
	}
	printf("Ok for %d char\n", (i - 1) * blockSize);
}

struct s {
	const char *file;
	nat sample;
};

int main() {
	USBHostMSD msd("usb");
	while (!msd.connect()) {
		Thread::wait(500);
	}

	// s file = { "/usb/sound.raw", 8000 };
	s file = { "/usb/s44100.raw", 44100 };
	// s file = { "/usb/s22000.raw", 22000 };
	// s file = { "/usb/s11000.raw", 11000 };

	RawFile *f = new RawFile(file.file);

	memory();

	Output output(p18, file.sample);
	printf("Created output\n");
	output.add(f);

	memory();

	//output.add(new SquareWave());
	startWorker(&output);

	while (true) {
		p = !p;
		Thread::wait(400);
	}

	return 0;
}

