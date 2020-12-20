#pragma once
#include <audioclient.h>
#include <mmdeviceapi.h>
#include "AudioSink.h"

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000
class Streamer
{
private:
	AudioSink* pMySink;
	HRESULT hr;
	REFERENCE_TIME hnsActualDuration;
	UINT32 bufferFrameCount;
	UINT32 numFramesAvailable;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioCaptureClient* pCaptureClient = NULL;
	WAVEFORMATEX* pwfx = NULL;
	UINT32 packetLength = 0;
	BYTE* pData;
	DWORD flags;
public:
	bool exit = 0;

	Streamer(AudioSink* pMySink);
	void Initialize();
	void recordSamples();
	void stop();


};

