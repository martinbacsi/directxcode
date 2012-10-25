#include "Sound.h"
#include <Windows.h>

Sound::Sound(void):XAudio2_(NULL), masterVoice_(NULL), sourceVoice_(NULL)
{
	// This line is needed for OS oldder Windows 8
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// Create an instance of XAudio2 engine
	HRESULT hr = XAudio2Create(&XAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Failed to create XAudio2 engine instance", L"Error", 0);
	}

	// Create a mastering voice
	hr = XAudio2_->CreateMasteringVoice(&masterVoice_);
	if ( FAILED(hr) )
	{
		MessageBox(NULL, L"Failed to create mastering voice", L"Error", 0);
	}

	ZeroMemory(&buffer_, sizeof(buffer_));
}

Sound::~Sound(void)
{
	SAFE_RELEASE(XAudio2_);
}

HRESULT Sound::findChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
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

void Sound::readChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset)
{
    HRESULT hr = S_OK;
    hr = SetFilePointer( hFile, bufferoffset, NULL, FILE_BEGIN );
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Failed to set file pointer", L"Error", 0);
	}

    DWORD dwRead;
    hr = ReadFile( hFile, buffer, buffersize, &dwRead, NULL );
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Failed to read file", L"Error", 0);
	}
}

void Sound::initialize(wchar_t* audioFile)
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
	findChunk(hFile,fourccRIFF,dwChunkSize, dwChunkPosition );
	DWORD filetype;
	readChunkData(hFile,&filetype,sizeof(DWORD),dwChunkPosition);
	if (filetype != fourccWAVE)
	{
		MessageBox(NULL, L"File type was not WAVE", L"Error", 0);
	}

	WAVEFORMATEXTENSIBLE wfx = {0};
	findChunk(hFile,fourccFMT, dwChunkSize, dwChunkPosition );
	readChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition );

	//fill out the audio data buffer with the contents of the fourccDATA chunk
	findChunk(hFile,fourccDATA,dwChunkSize, dwChunkPosition );
	BYTE * pDataBuffer = new BYTE[dwChunkSize];
	readChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

	buffer_.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
	buffer_.pAudioData = pDataBuffer;  //buffer containing audio data
	buffer_.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	// Create an instance of XAudio2 engine
	HRESULT hr = XAudio2Create(&XAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Failed to create XAudio2 engine instance", L"Error", 0);
	}

	// Create a mastering voice
	hr = XAudio2_->CreateMasteringVoice(&masterVoice_);
	if ( FAILED(hr) )
	{
		MessageBox(NULL, L"Failed to create mastering voice", L"Error", 0);
	}

	// Create a source voice
	hr = XAudio2_->CreateSourceVoice(&sourceVoice_, (WAVEFORMATEX*)&wfx);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create source voice failed", L"Error", 0);
	}
}

void Sound::play()
{
	// Sub mit source buffer
	// *** This line will failed if user pressed keyboard all the time without released, why?
	HRESULT hr = sourceVoice_->SubmitSourceBuffer(&buffer_);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Submit to source buffer failed", L"Error", 0);
	}

	// Start play current sound
	hr = sourceVoice_->Start();
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Play sound failed", L"Error", 0);
	}
}