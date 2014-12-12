#include <Windows.h>
#include <iostream>

using namespace std;

typedef unsigned int nat;

int main() {

	for (nat i = 1; i < 50; i++) {
		char name[] = "xx.xCOMNNN";
		sprintf(name, "\\\\.\\COM%u", i);
		HANDLE port = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (port != INVALID_HANDLE_VALUE) {
			SetCommBreak(port);
			Sleep(20);
			ClearCommBreak(port);
			Sleep(20);
			// DCB dcb;
			// dcb.DCBlength = sizeof(dcb);
			// ZeroMemory(&dcb, sizeof(dcb));

			// GetCommState(port, &dcb);
			// BuildCommDCB("baud=9600 parity=N data=8 stop=1", &dcb);

			// if (!SetCommState(port, &dcb)) {
			// 	cout << "Failed to set state." << endl;
			// }

			// char buffer = 0;
			// DWORD written;
			// WriteFile(port, &buffer, 1, &written, NULL);

			CloseHandle(port);

			cout << "Reset signal sent on " << name << endl;
		}
	}

	return 0;
}
