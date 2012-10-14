#include <C:\Program Files\Microsoft DirectX SDK (June 2010)\Include\XAudio2.h>
#include <Windows.h>
#include <iostream>

// Little-Endian for Windows, XBOX use big-endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
    HRESULT hr = S_OK;
    if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, 0, NULL, FILE_BEGIN ) )
        return HRESULT_FROM_WIN32( GetLastError() );

    DWORD dwChunkType;
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;

    while (hr == S_OK)
    {
        DWORD dwRead;

		// Get chunk type
        if( 0 == ReadFile( hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL ) )
            hr = HRESULT_FROM_WIN32( GetLastError() );

		// Get chunk data type
        if( 0 == ReadFile( hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL ) )
            hr = HRESULT_FROM_WIN32( GetLastError() );

        switch (dwChunkType)
        {
        case fourccRIFF:
            dwRIFFDataSize = dwChunkDataSize;
            dwChunkDataSize = 4;
            if( 0 == ReadFile( hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL ) )
                hr = HRESULT_FROM_WIN32( GetLastError() );
            break;

        default:
            if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, dwChunkDataSize, NULL, FILE_CURRENT ) )
            return HRESULT_FROM_WIN32( GetLastError() );            
        }

        dwOffset += sizeof(DWORD) * 2;
        
        if (dwChunkType == fourcc)
        {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;
        
        if (bytesRead >= dwRIFFDataSize) return S_FALSE;

    }

    return S_OK;
    
}

void ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset)
{
    HRESULT hr = S_OK;
    hr = SetFilePointer( hFile, bufferoffset, NULL, FILE_BEGIN );
	if(FAILED(hr))
	{
		MessageBox(NULL, "Failed to set file pointer", "Error", 0);
	}

    DWORD dwRead;
    hr = ReadFile( hFile, buffer, buffersize, &dwRead, NULL );
	if(FAILED(hr))
	{
		MessageBox(NULL, "Failed to read file", "Error", 0);
	}
}

void PlayAudioFile(char* audioFile)
{
	// Open the file
	HANDLE hFile = CreateFile(
		audioFile,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL );


	// Set file pointer to the begining of file
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	DWORD dwChunkSize;
	DWORD dwChunkPosition;

	//check the file type, should be fourccWAVE or 'XWMA'
	FindChunk(hFile,fourccRIFF,dwChunkSize, dwChunkPosition );
	DWORD filetype;
	ReadChunkData(hFile,&filetype,sizeof(DWORD),dwChunkPosition);
	if (filetype != fourccWAVE)
	{
		MessageBox(NULL, "File type was not WAVE", "Error", 0);
	}

	WAVEFORMATEXTENSIBLE wfx = {0};
	FindChunk(hFile,fourccFMT, dwChunkSize, dwChunkPosition );
	ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition );

	//fill out the audio data buffer with the contents of the fourccDATA chunk
	FindChunk(hFile,fourccDATA,dwChunkSize, dwChunkPosition );
	BYTE * pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

	XAUDIO2_BUFFER buffer = {0};
	buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
	buffer.pAudioData = pDataBuffer;  //buffer containing audio data
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	// This line is needed for OS oldder Windows 8
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// Create an instance of XAudio2 engine
	IXAudio2* pXAudio2 = NULL;
	HRESULT hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Failed to create XAudio2 engine instance", "Error", 0);
	}

	// Create a mastering voice
	IXAudio2MasteringVoice* pMasterVoice	= NULL;	
	hr = pXAudio2->CreateMasteringVoice(&pMasterVoice);
	if ( FAILED(hr) )
	{
		MessageBox(NULL, "Failed to create mastering voice", "Error", 0);
	}

	// Create a source voice
	IXAudio2SourceVoice* pSourceVoice  = NULL;
	hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx);
	if(FAILED(hr))
	{
		MessageBox(NULL, "Create source voice failed", "Error", 0);
	}

	// Sub mit source buffer
	hr = pSourceVoice->SubmitSourceBuffer(&buffer);
	if(FAILED(hr))
	{
		MessageBox(NULL, "Submit to source buffer failed", "Error", 0);
	}

	// Play
	hr = pSourceVoice->Start(0);
	if(FAILED(hr))
	{
		MessageBox(NULL, "Play sound failed", "Error", 0);
	}
}

int main(void)
{
	char * audioFile = "inmysong.wav";
	PlayAudioFile(audioFile);

	getchar();
	return 0;
}