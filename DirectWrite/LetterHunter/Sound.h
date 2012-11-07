#ifndef __SOUND_H__
#define __SOUND_H__

#include <Windows.h>

// Use the one in DirectX SDK folder for Win7, use the one in Windows SDK folder for Win8
#include "XAudio2.h"
#include "Utilities.h"

// Little-Endian for Windows, XBOX use big-endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

class Sound
{
public:
	Sound(void);
	~Sound(void);

public:
	void initialize(wchar_t* audioFile);
	void play();

private:
	HRESULT findChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
	void readChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset);

	IXAudio2*				XAudio2_;
	IXAudio2MasteringVoice* masterVoice_;	
	IXAudio2SourceVoice*	sourceVoice_;

	XAUDIO2_BUFFER buffer_;

};

#endif // end __SOUND_H__

