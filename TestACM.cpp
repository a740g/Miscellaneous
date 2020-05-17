// Blade: A test program to learn about wave*, acm* and mmio* APIs
// mailto: v_2samg@hotmail.com

#include <windows.h>
#include <dsetup.h>
#include <dsound.h>
#include <mmreg.h>
#include <msacm.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <math.h>

#pragma comment(lib, "winmm")
#pragma comment(lib, "msacm32")

#define NUMBUFFERS 4				/* number of buffers */
#define BUFFERSIZE 100				/* buffer size in milliseconds */

HWAVEOUT hwaveout;
WAVEHDR header[NUMBUFFERS];
LPBYTE buffer[NUMBUFFERS];		/* pointers to buffers */
WORD buffersout;				/* number of buffers playing/about to be played */
WORD nextbuffer;				/* next buffer to be mixed */
ULONG buffersize;				/* buffer size in bytes */
LPBYTE lpSnd;					// the actual decompressed sound
DWORD dwOutSize = 0;			// total size of the output stream
DWORD dwLoc = 0;				// location of the sample waiting to be streamed to WinMM
DWORD md_mixfreq = 44100;
DWORD md_channels = 2;
DWORD md_bitspersample = 16;

/* WinMM callback */
void CALLBACK WinMM_CallBack(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
	if (uMsg == WOM_DONE) --buffersout;
}

/* Initialize WinMM */  
BOOL WinMM_Init(void) {
	WAVEFORMATEX	wfe;
	DWORD			samplesize;
	MMRESULT		mmr;
	int				n;

	// Check for wave output devices
	if (waveOutGetNumDevs() < 1) {
		return FALSE;
	}

	samplesize = 1;
	samplesize *= md_channels;
	samplesize *= (md_bitspersample / 8);

	wfe.wFormatTag = WAVE_FORMAT_PCM;
	wfe.nChannels = (WORD)md_channels;
	wfe.nSamplesPerSec = md_mixfreq;
	wfe.nAvgBytesPerSec = md_mixfreq * samplesize;
	wfe.nBlockAlign = (WORD)samplesize;
	wfe.wBitsPerSample = (WORD)md_bitspersample;
	wfe.cbSize = sizeof(wfe);

	mmr = waveOutOpen(&hwaveout, WAVE_MAPPER, &wfe, (DWORD)WinMM_CallBack, 0, CALLBACK_FUNCTION);
	if (mmr != MMSYSERR_NOERROR) {
		return FALSE;
	}

	// Blade: ceil() is here for a purpose!!! Fixes alignment problems.
	buffersize = (DWORD)ceil((double)(md_mixfreq * samplesize * BUFFERSIZE) / 1000.0);

	for (n = 0; n < NUMBUFFERS; n++) {
		buffer[n] = (LPBYTE)malloc(buffersize);
		header[n].lpData = (LPSTR)buffer[n];
		header[n].dwBufferLength = buffersize;
		mmr = waveOutPrepareHeader(hwaveout, &header[n], sizeof(WAVEHDR));
		if (!buffer[n] || mmr != MMSYSERR_NOERROR) {
			return FALSE;
		}
	}

	buffersout = nextbuffer = 0;

	return TRUE;
}

/* Shutdown WinMM */
void WinMM_Exit(void) {
	int n;

	if (hwaveout) {
		waveOutReset(hwaveout);
		for (n = 0; n < NUMBUFFERS; n++) {
			if (header[n].dwFlags & WHDR_PREPARED)
				waveOutUnprepareHeader(hwaveout, &header[n], sizeof(WAVEHDR));
			free(buffer[n]);
		}
		while (waveOutClose(hwaveout) == WAVERR_STILLPLAYING) Sleep(10);
		hwaveout = NULL;
	}
}

ULONG Sound_Render(LPBYTE buf, DWORD bufsize) {
	LPBYTE lpBuf;

	// Calculate the correct location in the decompressed sound buffer
	if (dwLoc >= dwOutSize) return 0;
	lpBuf = lpSnd + dwLoc;

	// Calculate how much we actually have to stream
	bufsize = min(bufsize, dwOutSize - dwLoc);

	// Copy the required chuck to stream
	memcpy(buf, lpBuf, bufsize);

	// Update buffer position correctly
	dwLoc += bufsize;

	// Return the number of SAMPLES sreamed
	return bufsize;
}

/* Poll and update WinMM playback */
void WinMM_Update(void) {
	ULONG done;

	while (buffersout < NUMBUFFERS) {
		done = Sound_Render(buffer[nextbuffer], buffersize);
		if (!done) break;
		header[nextbuffer].dwBufferLength = done;
		waveOutWrite(hwaveout, &header[nextbuffer], sizeof(WAVEHDR));
		if (++nextbuffer >= NUMBUFFERS) nextbuffer %= NUMBUFFERS;
		++buffersout;
	}
}

/* Since DirectSound only loops complete buffer, we totally ignore partial looping */
static LPBYTE WaveGetFormat(LPBYTE lpSrc, DWORD dwFileSize, LPWAVEFORMATEX *lpwfxInfo, LPDWORD lpdwDataSize, LPBOOL lpbLooping) {
	MMIOINFO mmioInf;
	HMMIO hmmio;
	MMCKINFO mmckinfoParent;
	MMCKINFO mmckinfoSubchunk;
	LPBYTE lpPos;

	/* Open the wave file in memory */
	memset(&mmioInf, 0, sizeof(mmioInf));
	mmioInf.pIOProc = NULL;
	mmioInf.fccIOProc = FOURCC_MEM;
	mmioInf.pchBuffer = (LPSTR)lpSrc;
	mmioInf.cchBuffer = dwFileSize;

	hmmio = mmioOpen(NULL, &mmioInf, MMIO_READ);
	if (hmmio == NULL) {
		puts("mmioOpen() failed!");
		return NULL;
	}

	/* Make sure it's a wave file */
	memset(&mmckinfoParent, 0, sizeof(mmckinfoParent));
	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if (mmioDescend(hmmio, &mmckinfoParent, NULL, MMIO_FINDRIFF)) {
		mmioClose(hmmio, 0);
		puts("not a wave file!");
		return NULL;
	}

	/* Find the format chunk */
	memset(&mmckinfoSubchunk, 0, sizeof(mmckinfoSubchunk));
	mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)) {
		mmioClose(hmmio, 0);
		puts("corrupt wave file!");
		return NULL;
	}

	/* Verify the format chunk */
	if (mmckinfoSubchunk.cksize < sizeof(PCMWAVEFORMAT)) {
		mmioClose(hmmio, 0);
		puts("corrupt wave file!");
		return NULL;
	}

	/* Allocate memory for the format chunk */
	*lpwfxInfo = (LPWAVEFORMATEX)malloc(mmckinfoSubchunk.cksize);
	if (*lpwfxInfo == NULL) {
		mmioClose(hmmio, 0);
		puts("malloc() failed!");
		return NULL;
	}

    /* Read the 'fmt ' chunk */
	/*
		Note: we can do this since structure packing is enabled in
		all Win32 header files.
	*/
    if (mmioRead(hmmio, (HPSTR)*lpwfxInfo, (LONG)mmckinfoSubchunk.cksize) != (LONG)mmckinfoSubchunk.cksize) {
		free(*lpwfxInfo);
		*lpwfxInfo = NULL;
		mmioClose(hmmio, 0);
		puts("mmioRead() failed!");
		return NULL;
	}

	/* Get out of 'fmt ' */
	if (mmioAscend(hmmio, &mmckinfoSubchunk, 0)) {
		free(*lpwfxInfo);
		*lpwfxInfo = NULL;
		mmioClose(hmmio, 0);
		puts("mmioAscend() failed!");
		return NULL;
	}

	/* Seek to the proper location */
    if (mmioSeek(hmmio, mmckinfoParent.dwDataOffset + sizeof(FOURCC), SEEK_SET) == -1) {
		free(*lpwfxInfo);
		*lpwfxInfo = NULL;
		mmioClose(hmmio, 0);
		puts("mmioSeek() failed!");
		return NULL;
	}

    /* Search the input file for for the 'data' chunk */
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)) {
		free(*lpwfxInfo);
		*lpwfxInfo = NULL;
		mmioClose(hmmio, 0);
		puts("mmioDescend() failed!");
		return NULL;
	}

	/* Verify the data size */
	if (mmckinfoSubchunk.cksize < DSBSIZE_MIN || mmckinfoSubchunk.cksize > DSBSIZE_MAX) {
		free(*lpwfxInfo);
		*lpwfxInfo = NULL;
		mmioClose(hmmio, 0);
		puts("sound data size too small or too large!");
		return NULL;
	}

	/* Return the data size */
	*lpdwDataSize = mmckinfoSubchunk.cksize;

	/* Return the memory location from where the data starts */
	lpPos = lpSrc + mmckinfoSubchunk.dwDataOffset;

	/* Find out if the sound is to be looped */
	*lpbLooping = FALSE;

	/* Get out of 'data' */
	if (mmioAscend(hmmio, &mmckinfoSubchunk, 0)) {
		mmioClose(hmmio, 0);
		return lpPos;
	}

	/* Seek to the proper location */
    if (mmioSeek(hmmio, mmckinfoParent.dwDataOffset + sizeof(FOURCC), SEEK_SET) == -1) {
		mmioClose(hmmio, 0);
		return lpPos;
	}

    /* Search the input file for for the 'cue ' chunk */
    mmckinfoSubchunk.ckid = mmioFOURCC('c', 'u', 'e', ' ');
    if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)) {
		mmioClose(hmmio, 0);
		return lpPos;
	}

	*lpbLooping = TRUE;
	mmioClose(hmmio, 0);
	return lpPos;
}

static LPBYTE WaveDecompress(LPBYTE lpSrc, LPWAVEFORMATEX lpwfxSrc, DWORD dwSizeSrc, LPWAVEFORMATEX lpwfxDst, LPDWORD lpdwSizeDst) {
	HACMSTREAM has;
	ACMSTREAMHEADER ash;
	LPBYTE lpDst;
	MMRESULT mmr;

	/* Setup the output format to the default PCM */
	memset(lpwfxDst, 0, sizeof(WAVEFORMATEX));
	lpwfxDst->wFormatTag = WAVE_FORMAT_PCM;

	/* Ask the ACM to suggest a compatible format */
	if ((mmr = acmFormatSuggest(NULL, lpwfxSrc, lpwfxDst, sizeof(WAVEFORMATEX), ACM_FORMATSUGGESTF_WFORMATTAG)) != 0) {
		switch (mmr) {
			case MMSYSERR_INVALFLAG:
				puts("acmFormatSuggest(): At least one flag is invalid!");
				break;
			case MMSYSERR_INVALHANDLE:
				puts("acmFormatSuggest(): The specified handle is invalid!");
				break;
			case MMSYSERR_INVALPARAM:
				puts("acmFormatSuggest(): At least one parameter is invalid!");
				break;
			default:
				puts("acmFormatSuggest() failed!");
		}
		return NULL;
	}

	/* Open an ACM data conversion stream */
	if ((mmr = acmStreamOpen(&has, NULL, lpwfxSrc, lpwfxDst, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME)) != 0) {
		switch (mmr) {
			case ACMERR_NOTPOSSIBLE:
				puts("acmStreamOpen(): The requested operation cannot be performed!");
				break;
			case MMSYSERR_INVALFLAG:
				puts("acmStreamOpen(): At least one flag is invalid!");
				break;
			case MMSYSERR_INVALHANDLE:
				puts("acmStreamOpen(): The specified handle is invalid!");
				break;
			case MMSYSERR_INVALPARAM:
				puts("acmStreamOpen(): At least one parameter is invalid!");
				break;
			case MMSYSERR_NOMEM:
				puts("acmStreamOpen(): The system is unable to allocate resources!");
				break;
			default:
				puts("acmStreamOpen() failed!");
		}
		return NULL;
	}

	/* Determine output buffer size */
	if (acmStreamSize(has, dwSizeSrc, lpdwSizeDst, ACM_STREAMSIZEF_SOURCE)) {
		acmStreamClose(has, 0);
		puts("acmStreamSize() failed!");
		return NULL;
	}

	/* Allocate the buffer */
	lpDst = (LPBYTE)malloc(*lpdwSizeDst);
	if (lpDst == NULL) {
		acmStreamClose(has, 0);
		puts("malloc() failed!");
		return NULL;
	}

	/* Prepare the stream header */
	memset(&ash, 0, sizeof(ash));
	ash.cbStruct = sizeof(ash);
	ash.pbSrc = lpSrc;
	ash.cbSrcLength = dwSizeSrc;
	ash.pbDst = lpDst;
	ash.cbDstLength = *lpdwSizeDst;
	
	if (acmStreamPrepareHeader(has, &ash, 0)) {
		free(lpDst);
		acmStreamClose(has, 0);
		puts("acmStreamPrepareHeader() failed!");
		return NULL;
	}

	/* Start converting the data */
	if (acmStreamConvert(has, &ash, 0)) {
		acmStreamUnprepareHeader(has, &ash, 0);
		free(lpDst);
		acmStreamClose(has, 0);
		puts("acmStreamConvert() failed!");
		return NULL;
	}

	/* Unprepare the header */
	acmStreamUnprepareHeader(has, &ash, 0);

	/* Close the ACM stream */
	acmStreamClose(has, 0);

	/* Return the exact destination buffer usage size */
	*lpdwSizeDst = ash.cbDstLengthUsed;

	return lpDst;
}

int main(int argc, char **argv) {
	FILE *f;
	LPBYTE lpBuf;
	LPBYTE lpStart;
	DWORD dwSize, dwDataSize;
	LPWAVEFORMATEX lpwfx;
	WAVEFORMATEX wfxD;
	BOOL bLooping, bPaused = FALSE;
	CHAR c;

	if (argc < 2) {
		puts("syntax: testapi [filename]");
		return EXIT_FAILURE;
	}

	f = fopen(argv[1], "rb");
	if (f == NULL) {
		puts("failed to open file!");
		return EXIT_FAILURE;
	}

	dwSize = filelength(fileno(f));

	if (dwSize < DSBSIZE_MIN || dwSize > DSBSIZE_MAX) {
		puts("file too small or too large!");
		fclose(f);
		return EXIT_FAILURE;
	}

	lpBuf = (LPBYTE)malloc(dwSize);
	if (lpBuf == NULL) {
		fclose(f);
		puts("malloc() failed!");
		return EXIT_FAILURE;
	}

	if (fread(lpBuf, sizeof(BYTE), dwSize, f) != dwSize) {
		fclose(f);
		free(lpBuf);
		puts("fread() failed!");
		return EXIT_FAILURE;
	}

	lpStart = WaveGetFormat(lpBuf, dwSize, &lpwfx, &dwDataSize, &bLooping);
	if (lpStart == NULL) {
		puts("WaveGetFormat() failed!");
		free(lpBuf);
		fclose(f);
		return EXIT_FAILURE;
	}

	printf("file name: %s\n", argv[1]);
	printf("file size = %i\n", dwSize);
	printf("data start = %i\n", (int)(lpStart - lpBuf));
	printf("data length = %i\n", dwDataSize);
	printf("data format = %i\n", (int)lpwfx->wFormatTag);
	printf("data bits/sample = %i\n", (int)lpwfx->wBitsPerSample);
	printf("data channels = %i\n", (int)lpwfx->nChannels);
	printf("data sample rate = %i\n", (int)lpwfx->nSamplesPerSec);
	printf("data extra format information = %i\n", (int)lpwfx->cbSize);
	printf("data average bytes/sec = %i\n", (int)lpwfx->nAvgBytesPerSec);
	printf("data block alingment = %i\n", (int)lpwfx->nBlockAlign);
	printf("data looping: %s\n\n", (bLooping ? "True" : "False"));

	lpSnd = WaveDecompress(lpStart, lpwfx, dwDataSize, &wfxD, &dwOutSize);
	if (lpSnd == NULL) {
		free(lpwfx);
		free(lpBuf);
		fclose(f);
		puts("WaveDecompress() failed!");
		return EXIT_FAILURE;
	}

	printf("decompressed sound size = %i\n", dwOutSize);
	printf("data format = %i\n", (int)wfxD.wFormatTag);
	printf("data bits/sample = %i\n", (int)wfxD.wBitsPerSample);
	printf("data channels = %i\n", (int)wfxD.nChannels);
	printf("data sample rate = %i\n", (int)wfxD.nSamplesPerSec);
	printf("data format extension = %i\n", (int)wfxD.cbSize);
	printf("data average bytes/sec = %i\n\n", (int)wfxD.nAvgBytesPerSec);

	// Free all resources not required
	free(lpwfx);
	free(lpBuf);
	fclose(f);

	// Setup WinMM
	md_channels = wfxD.nChannels;
	md_bitspersample = wfxD.wBitsPerSample;
	md_mixfreq = wfxD.nSamplesPerSec;

	if (!WinMM_Init()) {
		puts("Failed to initialize WinMM device!");
		free(lpSnd);
		return EXIT_FAILURE;
	}

	printf("Playing %lu-bit %s audio @ %lu Hz\n\n",
			md_bitspersample,
			(md_channels == 1) ? "mono" : "stereo",
			md_mixfreq);

	// Loop if we have more than 1 paramater
	if (argc > 2) bLooping = TRUE;

	while (dwLoc < dwOutSize) {
		c = (CHAR)(kbhit() ? getch() : 0);

		if (c == 'p' || c == 'P')
			bPaused = !bPaused;
		else if (c == 0x1b) {
			break;
		}

		if (!bPaused) WinMM_Update();

		// Looping support
		if (bLooping && dwLoc >= dwOutSize) dwLoc = 0;

		Sleep(10);

		printf("\rPosition: %lu of %lu...", dwLoc, dwOutSize);
	}

	Sleep(1000);
	WinMM_Exit();
	free(lpSnd);

	return EXIT_SUCCESS;
}
