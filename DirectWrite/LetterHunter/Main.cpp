/*
Description:  This demo show you how to translate text in DirectWrite
Date:			2012-09-22
Author:			zdd(vckzdd@gmail.com)
Key steps:
		1. Create font from file
		2. Get font glyph and store it in path geometry
		3. Transform the path geometry.
*/

// Windows header
#include <time.h>

// DirectX header
#include <d2d1.h>
#include <dwrite.h>
#include <DirectXMath.h>
#include <D3D9Types.h>

// Customer header
#include "Text.h"

#define SAFE_RELEASE(P) if(P){P->Release() ; P = NULL ;}

// Globals 
ID2D1Factory*				g_pD2DFactory		= NULL;
ID2D1HwndRenderTarget*		g_pRenderTarget		= NULL;
IDWriteFactory*				g_pDWriteFactory	= NULL;

const int					g_TextCount			= 10;
Text*						g_pText[g_TextCount]	= { NULL };

/*
Create device independent resources, all DriectWrite resources are device independent
*/
VOID CreateDeviceIndependentResources()
{
	// Create Direct2D Factory
	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&g_pD2DFactory
		);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create Direct2D factory failed!", L"Error", 0);
		return;
	}

	// Create DirectWrite Factory
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED, 
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&g_pDWriteFactory)
		);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create DirectWrite factory failed!", L"Error", 0);
		return;
	}
}

// Generate random integer between start and end(inclusive)
int RandomInt(int start, int end)
{
	int k = rand() % (end - start + 1) + start;

	return k;
}

// Generate random lower case letter('a' - 'z')
char RandomLowerCaseLetter()
{
	int offset = RandomInt(0, 25);
	char letter = 'a' + offset;

	return letter;
}

// Generate random upper case letter('A' - 'Z')
char RandomUpperCaseLetter()
{
	int offset = RandomInt(0, 25);
	char letter = 'A' + offset;

	return letter;
}

// Generate a random RGB color
DWORD RandomColor()
{
	DWORD a = 0xff000000;
	int r = RandomInt(0, 255) << 16;
	int g = RandomInt(0, 255) << 8;
	int b = RandomInt(0, 255) ;

	DWORD color = a | r | g | b;
	return color;
}

VOID InitializeText()
{
	wchar_t* textBuffer[g_TextCount] = {
		L"A",
		L"B",
		L"C",
		L"D",
		L"E",
		L"F",
		L"G",
		L"H",
		L"I",
		L"J"
	};

	for(int i = 0; i < g_TextCount; ++i)
	{
		g_pText[i] = new Text(
			g_pD2DFactory,
			g_pRenderTarget,
			g_pDWriteFactory,
			textBuffer[i],
			D2D1::ColorF(D2D1::ColorF::Red),
			D2D1::ColorF(D2D1::ColorF::Black),
			10.0f,
			300
			);
	}
}

/*
Create device dependent resources, render target and brush are all device dependent resources
*/
VOID CreateDeviceResources(HWND Hwnd)
{
	// Get the client area size
	RECT rc;
	GetClientRect(Hwnd, &rc);

	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

	// Only creaste once
	if(!g_pRenderTarget)
	{
		// Create render target
		HRESULT hr = g_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(Hwnd, size),
			&g_pRenderTarget
		);
		if(FAILED(hr))
		{
			MessageBox(Hwnd, L"Create render target failed!", L"Error", 0);
			return;
		}

		InitializeText();
	}
}

// Cleanup the device dependent resources
VOID DiscardDeviceResources()
{
	SAFE_RELEASE(g_pDWriteFactory);
	SAFE_RELEASE(g_pRenderTarget);
	SAFE_RELEASE(g_pD2DFactory);
}

VOID CalculateTranslationMatrix(D2D1_MATRIX_3X2_F* matrix)
{
	static float totalTime = 0.0f;

	// Get start time
	static DWORD startTime = timeGetTime();

	// Get current time
	DWORD currentTime = timeGetTime();

	// Calculate time elapsed
	float timeElapsed = (currentTime - startTime) * 0.001f;

	// Accumulate total time elapsed
	totalTime += timeElapsed;

	// Build up the translation matrix
	matrix->_11 = 1.0f;
	matrix->_12 = 0.0f;
	matrix->_21 = 0.0f;
	matrix->_22 = 1.0f;
	matrix->_31 = 100;
	matrix->_32 = totalTime;
}

VOID Render(HWND hwnd)
{
	// Create device dependent resources
	CreateDeviceResources(hwnd);

	g_pRenderTarget->BeginDraw();
	g_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	///////////////////////////
	D2D1_MATRIX_3X2_F matrix;
	CalculateTranslationMatrix(&matrix);

	for(int i = 0; i < g_TextCount; ++i)
	{
		g_pText[i]->setTransform(matrix);

		// Set text color
		g_pText[i]->setFillColor(D2D1::ColorF(D2D1::ColorF::Green));
		g_pText[i]->setOutlineColor(D2D1::ColorF(D2D1::ColorF::Red));

		// Set outline width
		g_pText[i]->setOutlineWidth(30.0f);

		g_pText[i]->render();
	}
	//////////////////////////////

	g_pRenderTarget->EndDraw();
}

// When window size changed, we need to resize the render target as well
VOID OnResize(UINT width, UINT height)
{
	if(g_pRenderTarget)
	{
		D2D1_SIZE_U size = D2D1::SizeU(width, height);
		g_pRenderTarget->Resize(size);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)   
{
	switch (message)    
	{
	case   WM_PAINT:
		Render(hwnd);
		return 0 ;

	case WM_SIZE:
		{
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			OnResize(width, height);
			break;
		}

	case WM_KEYDOWN: 
		{ 
			switch( wParam ) 
			{ 
			case VK_ESCAPE: 
				SendMessage( hwnd, WM_CLOSE, 0, 0 ); 
				break ; 
			default: 
				break ; 
			} 
		} 
		break ; 

	case WM_DESTROY: 
		DiscardDeviceResources(); 
		PostQuitMessage( 0 ); 
		return 0; 
	}

	return DefWindowProc (hwnd, message, wParam, lParam) ;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow )
{
	srand((unsigned int)time(0));

	WNDCLASSEX winClass ;

	winClass.lpszClassName = L"DirectWrite Hello, World";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = WndProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon	       = NULL ;
	winClass.hIconSm	   = NULL ;
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = NULL ;
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	if (!RegisterClassEx (&winClass))   
	{
		MessageBox ( NULL, TEXT( "This program requires Windows NT!" ), L"error", MB_ICONERROR) ;
		return 0 ;  
	}   

	// Get screen resolution
	const HWND hDesktop = GetDesktopWindow();

	RECT desktopRect;
	GetWindowRect(hDesktop, &desktopRect);

	int windowWidth	 = desktopRect.right;
	int windowHeight = desktopRect.bottom;

	HWND hwnd = CreateWindowEx(NULL,  
		L"DirectWrite Hello, World",	// window class name
		L"Draw Text",					// window caption
		WS_OVERLAPPEDWINDOW, 			// window style
		CW_USEDEFAULT,					// initial x position
		CW_USEDEFAULT,					// initial y position
		windowWidth,					// window width
		windowHeight,					// window height 
		NULL,							// parent window handle
		NULL,							// window menu handle
		hInstance,						// program instance handle
		NULL) ;							// creation parameters

	CreateDeviceIndependentResources();

	ShowWindow (hwnd, iCmdShow) ;
	UpdateWindow (hwnd) ;

	MSG    msg ;  
	ZeroMemory(&msg, sizeof(msg)) ;

	while (GetMessage (&msg, NULL, 0, 0))  
	{
		TranslateMessage (&msg) ;
		DispatchMessage (&msg) ;
	}

	return msg.wParam ;
}