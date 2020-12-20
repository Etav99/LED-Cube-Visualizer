#pragma once
#include <audioclient.h>
#include <complex>
#include <iostream>
#include <fftw3.h>
#include <iomanip>
#include <windows.h>

using namespace std;
using namespace std::complex_literals;

class AudioSink
{
private:
	double* in;
	fftw_complex* out;
	double* x;
	double* y;
	int filled;
	int position;
	int nFreqSamples;
	int bufferSize;
	WAVEFORMATEX format;
	uint8_t cube[64] = {};
	uint8_t rec[64] = {};
	DCB dcb;
	HANDLE *hNumPort;
	DWORD RS_ile;

public:
	AudioSink(int bufferSize, HANDLE *hNumPort);
	HRESULT SetFormat(WAVEFORMATEX* pwfx);
	HRESULT CopyData(BYTE* pData, UINT32 numFramesAvailable);
	int append(double);
	void FFT();
	int getBufferSize() { return  bufferSize; };

};

