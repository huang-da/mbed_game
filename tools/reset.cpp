#include <Windows.h>
#include <iostream>
#include <conio.h>

using namespace std;

typedef unsigned int nat;

char kbchar() {
	if (_kbhit())
		return _getch();
	else
		return 0;
}

int main() {

	for (nat i = 1; i < 50; i++) {
		char name[] = "xx.xCOMNNN";
		sprintf(name, "\\\\.\\COM%u", i);
		HANDLE port = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (port != INVALID_HANDLE_VALUE) {
			DCB dcb;
			dcb.DCBlength = sizeof(dcb);
			ZeroMemory(&dcb, sizeof(dcb));

			GetCommState(port, &dcb);
			BuildCommDCB("baud=9600 parity=N data=8 stop=1", &dcb);

			if (!SetCommState(port, &dcb)) {
				cout << "Failed to set state." << endl;
			}

			COMMTIMEOUTS timeouts;
			GetCommTimeouts(port, &timeouts);
			timeouts.ReadIntervalTimeout = 20;
			timeouts.ReadTotalTimeoutMultiplier = 0;
			timeouts.ReadTotalTimeoutConstant = 50;
			SetCommTimeouts(port, &timeouts);

			SetCommBreak(port);
			Sleep(20);
			ClearCommBreak(port);
			Sleep(20);

			cout << "Reset signal sent on " << name << endl;
			cout << "Echoing data until <return> is pressed." << endl;

			const nat bufSz = 50;
			char buffer[bufSz + 1];
			while (kbchar() != 13) {
				DWORD read;
				if (!ReadFile(port, buffer, bufSz, &read, NULL)) {
					cout << "Failed to read from port!" << endl;
					break;
				}

				if (read) {
					buffer[read] = 0;
					cout << buffer;
				}
			}

			CloseHandle(port);
		}
	}

	return 0;
}
