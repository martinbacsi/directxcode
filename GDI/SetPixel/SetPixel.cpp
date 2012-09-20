/*
Description:	This demo show you how to use SetPixel function to draw pixels
Date:			2012-09-19
Author:			zdd(vckzdd@gmail.com)
*/

#include <windows.h>
#include <math.h>

VOID DrawPoint(HWND hwnd, int x, int y, COLORREF pixelColor)
{
	// Get the device context of the window
	HDC hdc = GetDC(hwnd);
	COLORREF color = pixelColor; // red
	SetPixel(hdc, x, y, color);
}

VOID DrawCircle(HWND hwnd, int centerX, int centerY, int radius)
{
	// Start from the most right point of the circle
	int startX = radius;
	int startY = 0;

	for (int angle = 0; angle < 100; ++angle)
	{
		int currentX = startX * cosf(angle) - startY * sinf(angle) + centerX;
		int currentY = startX * sinf(angle) + startY * cosf(angle) + centerY;
		DrawPoint(hwnd, currentX, currentY, 0x00ff0000);
	}
}
        
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)   
{
	HDC hdc ;
	PAINTSTRUCT ps ;
	RECT rect ;  

    switch (message)    
    {
	case   WM_PAINT:
		hdc = BeginPaint (hwnd, &ps) ;
		GetClientRect (hwnd, &rect) ;
		DrawText (hdc, TEXT ("Hello, World!"), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER) ;
		DrawCircle(hwnd, 500, 500, 100);
		EndPaint (hwnd, &ps) ;
		return 0 ;
	
	case   WM_DESTROY:
		PostQuitMessage (0) ;
		return 0 ;    
    }

	return DefWindowProc (hwnd, message, wParam, lParam) ;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow )
{
        
    WNDCLASSEX winClass ;

	winClass.lpszClassName = L"MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = WndProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon	       = NULL ;
	winClass.hIconSm	   = NULL ;
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;
	
	if (!RegisterClassEx (&winClass))   
	{
		MessageBox ( NULL, TEXT( "This program requires Windows NT!" ), L"error", MB_ICONERROR) ;
		return 0 ;  
    }   
    
	HWND   hwnd ; 
	hwnd = CreateWindowEx(NULL,  
		L"MY_WINDOWS_CLASS",        // window class name
		L"The Hello World Program",	// window caption
		WS_OVERLAPPEDWINDOW, 		// window style
		CW_USEDEFAULT,				// initial x position
		CW_USEDEFAULT,				// initial y position
		CW_USEDEFAULT,				// initial x size
		CW_USEDEFAULT,				// initial y size
		NULL,						// parent window handle
		NULL,						// window menu handle
		hInstance,					// program instance handle
		NULL) ;						// creation parameters

        ShowWindow (hwnd, iCmdShow) ;
		UpdateWindow (hwnd) ;

		MSG    msg ;    
		while (GetMessage (&msg, NULL, 0, 0))  
		{
			TranslateMessage (&msg) ;
			DispatchMessage (&msg) ;
		}

		return msg.wParam ;
}

/*
summery
1. why my window was not show up after running my program ?
The class name defined in CreateWindowEx function must be conform to the class name in WNDCLASSEX structure
otherwise, the window will fail to display

2. Why I got "This program need windows NT" message ?
because, you are using Vista OS, and you use CreateWindow function, try CreateWindowEx instead!

3. WS_OVERLAPPEDWINDOW this style contains the "close", "Maximize" and "Minimize" button.
*/