/*
Description:  This demo show you how to translate text in DirectWrite
Date:			2012-09-22
Author:			zdd(vckzdd@gmail.com)
Key steps:
		1. Create font from file
		2. Get font glyph and store it in path geometry
		3. Transform the path geometry.
*/

#include <Windows.h>
#include <MMSystem.h>
#include "LetterHunter.h"
#include "Utilities.h"

LetterHunter* g_pletterHunter = NULL;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)   
{
	switch (message)    
	{
	case   WM_PAINT:
		{
			static DWORD lastTime = timeGetTime();
			DWORD currentTime = timeGetTime();
			float timeDelta = (currentTime - lastTime) * 0.001f;

			g_pletterHunter->run(timeDelta);

			lastTime = currentTime;

			return 0 ;
		}

	case WM_SIZE:
		{
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			g_pletterHunter->resize(width, height);
			break;
		}

	case WM_KEYDOWN: 
		{ 
			switch( wParam ) 
			{ 
			case VK_ESCAPE: 
				SendMessage( hwnd, WM_CLOSE, 0, 0 ); 
				break ; 
			} 
		} 
		break ; 

	case WM_DESTROY: 
		SAFE_DELETE(g_pletterHunter);
		PostQuitMessage( 0 ); 
		return 0; 
	}

	return DefWindowProc (hwnd, message, wParam, lParam) ;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow )
{
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
		L"LetterObject Hunter",				// window caption
		WS_OVERLAPPEDWINDOW, 			// window style
		CW_USEDEFAULT,					// initial x position
		CW_USEDEFAULT,					// initial y position
		windowWidth,					// window width
		windowHeight,					// window height 
		NULL,							// parent window handle
		NULL,							// window menu handle
		hInstance,						// program instance handle
		NULL) ;							// creation parameters

	g_pletterHunter = new LetterHunter(hwnd, 10);
	g_pletterHunter->initialize();

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