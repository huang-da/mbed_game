#include "mbed.h"

int main();

extern "C" int __real_main() {
	return main();
}
