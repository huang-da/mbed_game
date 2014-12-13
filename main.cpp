#include "std.h"
#include "sound.h"
#include "USBHostMSD.h"

DigitalOut p(LED1);

int main() {
	USBHostMSD msd("usb");
	while (!msd.connect()) {
		Thread::wait(500);
	}

	Output output(p18, 8000);
	// output.add(new RawFile("/usb/sound.raw"));
	output.add(new SquareWave());

	while (true) {
		p = !p;
		Thread::wait(400);
	}
	return 0;
}

