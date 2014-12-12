#include "std.h"
// #include "sound.h"

DigitalOut p(LED1);

int main() {
	//Output output(LED1, 8000);
	while (true) {
		p = !p;
		wait(0.4);
	}
	return 0;
}

