// OS_c.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include <iostream>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <stdlib.h>
#include "AudioSink.h"
#include "Streamer.h"

using namespace std;

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient); 

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

HRESULT RecordAudioStream(AudioSink* pMySink);

volatile bool consoleExit = false;
Streamer *str;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
	case CTRL_C_EVENT:
		consoleExit = true;
		return FALSE;

	case CTRL_CLOSE_EVENT:
		consoleExit = true;
		return FALSE;
		break;

	case CTRL_SHUTDOWN_EVENT:
		consoleExit = true;
		return FALSE;

	default:
		return FALSE;
	}
}

int main()
{
	if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
		printf("\nConsole handler error!");
		return 1;
	}

	char port[5] = "COM ";
	char x;
	cout << "Wprowadz numer portu COM, do ktorego podlaczona jest kostka: ";
	cin >> port[3];
	WCHAR    wPort[5];
	MultiByteToWideChar(0, 0, port, 5, wPort, 5);

	DCB dcb;
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = CBR_115200;
	dcb.fParity = TRUE;
	dcb.Parity = EVENPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.ByteSize = 8;
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fRtsControl = RTS_CONTROL_ENABLE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fAbortOnError = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	dcb.fErrorChar = FALSE;
	dcb.fNull = FALSE;

	HANDLE hNumPort;
	hNumPort = CreateFile(wPort, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	cout << wPort;
	cout << "\n\nLaczenie z kostka przez port " << port << "\n\n";
	while (!SetCommState(hNumPort, &dcb) && !consoleExit);
	cout << "Polaczono!";

	CoInitialize(0);
	AudioSink sink(512, &hNumPort);
	Streamer streamer(&sink);
	str = &streamer;
	streamer.Initialize();
	while (!consoleExit) {
		streamer.recordSamples();
	}
	streamer.stop();
	uint8_t zeros[64] = {};
	WriteFile(hNumPort, (char*)(zeros), 64, NULL, 0);
	CloseHandle(hNumPort);
	return 0;
}
