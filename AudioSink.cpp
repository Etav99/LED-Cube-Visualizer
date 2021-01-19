#include "AudioSink.h"
# define PI 3.14159265358979323846

AudioSink::AudioSink(int bufferSize, HANDLE * hNumPort)
{
	this->hNumPort = hNumPort;
	this->bufferSize = bufferSize;
	in = (double*)fftw_malloc(sizeof(double) * bufferSize);
	int n_out = ((bufferSize / 2) + 1);
	out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * n_out);
	position = 0;
	filled = 0;
}

HRESULT AudioSink::SetFormat(WAVEFORMATEX* pwfx)
{
	format = *pwfx;
	nFreqSamples = round(((double)(format.nSamplesPerSec)) /
		(format.nSamplesPerSec / ((double)bufferSize))) / 2;
	//nFreqSamples++;
	if (nFreqSamples > (bufferSize / 2)) {
		nFreqSamples = (bufferSize / 2);
	}
	x = new double[nFreqSamples];
	y = new double[nFreqSamples];
	y_prev = new double[nFreqSamples];
	memset(y_prev, 0, nFreqSamples);
	return 0;
}

HRESULT AudioSink::CopyData(BYTE* pData, UINT32 numFramesAvailable)
{
	if (pData != NULL && numFramesAvailable >= 0)
	{
		for (int i = 0; i < 2 * bufferSize && i < 2*numFramesAvailable; i += 2) {
			if(append(double(((float*)(pData))[i] + ((float*)(pData))[i + 1]))){
				FFT();
				break;
			}
		}
	}
	return 0;
}

int AudioSink::append(double val)
{
	in[position] = val;
	position++;
	if (position == bufferSize)
	{
		position = 0;
		filled = 1;
	}
	return filled;
}

void AudioSink::FFT()
{
	if (filled)
	{
		fftw_plan plan_forward;

		// Okno Hanna
		for (int i = 0; i < bufferSize; i++) {
			double multiplier = 0.5 * (1 - cos(2 * PI * i / (bufferSize - 1)));
			in[i] = multiplier * in[i];
		}

		// Liczby rzeczywiste na wejœciu, zespolone na wyjœciu
		plan_forward = fftw_plan_dft_r2c_1d(bufferSize, in, out, FFTW_ESTIMATE);

		// wykonaj fft
		fftw_execute(plan_forward);

		// Oblicz wartoœci z liczb zespolonych
		double yMax = 1E-10;
		int mul = (nFreqSamples+1) / 64;
 		for (int i = 0; i < nFreqSamples; i++)
		{
			int j = i / 4;
			if (i % mul == 0)
				y[i / mul] = 0;
			double a = out[i][0], b = out[i][1];
			y[j] += sqrt(sqrt(a * a + b * b)) / nFreqSamples;
			if (y[j] > yMax) yMax = y[j];
		}


		// Stwórz tablice i wyœlij do kostki
		//system("CLS");
		int shift;
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				shift = pow(((y[(i * 8 + j % 8)] + y_prev[(i * 8 + j % 8)]) / 2 / yMax), 0.75) * 8;
				cube[i * 8 + j % 8] = 0xFF00 << (8 - shift) >> 8;
				//for (int k = 0; k < shift; k++) {
				//	printf("*");
				//}
				//printf("\n");
			}
		}
		WriteFile(*hNumPort, (char*)(cube), 64, &bytesSent, 0);
		filled = 0;
		position = 0;

		memcpy(y_prev, y, nFreqSamples);
	}
}