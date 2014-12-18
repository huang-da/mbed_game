#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <ShlWapi.h>

#pragma comment (lib, "shlwapi.lib")

using namespace std;

typedef unsigned int nat;

char kbchar() {
	if (_kbhit())
		return _getch();
	else
		return 0;
}

const nat progressWidth = 70;

DWORD CALLBACK progress(LARGE_INTEGER total,
			LARGE_INTEGER transferred,
			LARGE_INTEGER streamSz,
			LARGE_INTEGER streamTransferred,
			DWORD streamNr,
			DWORD callbackReason,
			HANDLE src,
			HANDLE dest,
			LPVOID data) {

	nat filled = nat((transferred.QuadPart * progressWidth) / total.QuadPart);
	char progress[progressWidth + 3];
	progress[0] = '[';
	progress[progressWidth + 1] = ']';
	progress[progressWidth + 2] = 0;
	for (nat i = 0; i < progressWidth; i++)
		if (i < filled)
			progress[i + 1] = '=';
		else if (i > filled)
			progress[i + 1] = ' ';
		else
			progress[i + 1] = '>';

	printf("\r%s", progress);

	return PROGRESS_CONTINUE;
}

int main(int argc, const char **argv) {

	if (argc == 2) {
		char from[MAX_PATH];
		for (nat i = 0; argv[1][i]; i++) {
			from[i] = argv[1][i];
			from[i+1] = 0;
		}
		for (nat i = 0; from[i]; i++)
			if (from[i] == '/')
				from[i] = '\\';

		for (char d = 'a'; d <= 'z'; d++) {
			char path[] = "?:\\";
			path[0] = d;

			if (GetDriveType(path) == DRIVE_REMOVABLE) {
				char check[] = "?:\\MBED.HTM";
				check[0] = d;
				if (PathFileExists(check)) {
					char to[] = "?:\\prog.bin";
					to[0] = d;
					printf("Copying to %s\n", to);
					if (!CopyFileEx(from, to, progress, 0, 0, 0)) {
						printf("Copy failed! %d\n", GetLastError());
					}
					printf("\n");
				}
			}
		}
	}

	Sleep(1000);

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
			cout << "Echoing data until <return> is pressed." << endl << endl;

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
