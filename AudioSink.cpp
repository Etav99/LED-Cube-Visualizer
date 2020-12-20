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
	nFreqSamples++;
	if (nFreqSamples > (bufferSize / 2)) {
		nFreqSamples = (bufferSize / 2);
	}
	x = new double[nFreqSamples];
	y = new double[nFreqSamples];
	return 0;
}

HRESULT AudioSink::CopyData(BYTE* pData, UINT32 numFramesAvailable)
{
	if (pData != NULL && numFramesAvailable >= bufferSize*2)
	{
		for (int i = 0; i < 2 * bufferSize; i += 2) {
			append(double(((float*)(pData))[i] + ((float*)(pData))[i + 1]));
			if (filled) {
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
		for (int i = 0; i < nFreqSamples; i++)
		{
			double a = out[i][0], b = out[i][1];
			y[i] = (sqrt(sqrt(sqrt(a * a + b * b))) / nFreqSamples);
			if ((y[i] > yMax) && (i > 0)) yMax = y[i];
		}
		y[0] = 0.0;

		// Stwórz tablice i wyœlij do kostki
		int shift;
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				shift = (y[1 + (i * 8 + j % 8)] / yMax) * 8;
				if (shift == 8) {
					i = i;
				}
				cube[i * 8 + j % 8] = 0xFF00 << (8 - shift) >> 8;
			}
		}
		WriteFile(*hNumPort, (char*)(cube), 64, &RS_ile, 0);
		filled = 0;
		position = 0;
	}
}