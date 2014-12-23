#include "cpu.h"
#include "std.h"
#include "rtos.h"

static const nat threadStackSize = 1024;
static byte threadStack[threadStackSize];
static Thread *thread = 0;
static const nat outputSize = 50;

static nat cyclesPerSec() {
	Timer t;
	t.start();
	nat cycles = 0;
	while (t.read_ms() <= 1000) {
		Thread::yield();
		cycles++;
	}
	t.stop();
	return cycles;
}

static void cpuMeasure(const void *) {
	nat baseline = cyclesPerSec();
	char ascii[outputSize + 3];
	ascii[0] = '[';
	ascii[outputSize + 1] = ']';
	ascii[outputSize + 2] = 0;

	while (true) {
		nat n = cyclesPerSec();

		nat remaining = n * outputSize / baseline;
		if (remaining > outputSize) remaining = 0;
		nat used = outputSize - remaining;

		for (nat i = 0; i < outputSize; i++) {
			if (used > i)
				ascii[i + 1] = '=';
			if (used == i)
				ascii[i + 1] = '|';
			if (used < i)
				ascii[i + 1] = ' ';
		}

		printf("\r%s %10u/%10u", ascii, n, baseline);
		fflush(stdout);
	}
}

void measureCpu() {
	if (thread)
		return;

	thread = new Thread(cpuMeasure, 0, osPriorityIdle, threadStackSize, threadStack);

	// Let the measuring thread measure the baseline.
	Thread::wait(1200);
}
