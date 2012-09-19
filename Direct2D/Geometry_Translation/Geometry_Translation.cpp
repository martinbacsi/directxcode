#include <windows.h>
#include <D2D1.h> 

#define SAFE_RELEASE(P) if(P){P->Release() ; P = NULL ;}

ID2D1Factory*				g_pD2DFactory			= NULL;	// Direct2D factory
ID2D1HwndRenderTarget*		g_pRenderTarget			= NULL;	// Render target
ID2D1SolidColorBrush*		g_pBlackBrush			= NULL;	// A black brush, reflect the line color
ID2D1RectangleGeometry*		g_pRectangleGeometry	= NULL;	// rectangle geometry
ID2D1TransformedGeometry*	g_pTransformedGeometry	= NULL;	// transformed geometry

float g_totalTime = 0.0f;

RECT rc ;		// Render area
HWND g_Hwnd ;	// Window handle

VOID CreateD2DResource(HWND hWnd)
{
	// This function was called in the DrawRectangle function which in turn called to response the
	// WM_PAINT Message, to avoid creating resource every time, we test the pointer to g_pRenderTarget
	// If the resource already create, skip the function
	if (!g_pRenderTarget)
	{
		HRESULT hr ;

#if defined(DEBUG) || defined(_DEBUG) // for debug build
		D2D1_FACTORY_OPTIONS options ;
		options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION ;
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, &g_pD2DFactory) ;
		if (FAILED(hr))
		{
			MessageBox(hWnd, "Create D2D factory failed!", "Error", 0) ;
			return ;
		}

#else	// for release build
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory) ;
		if (FAILED(hr))
		{
			MessageBox(hWnd, "Create D2D factory failed!", "Error", 0) ;
			return ;
		}
#endif
		// Obtain the size of the drawing area
		GetClientRect(hWnd, &rc) ;

		// Create a Direct2D render target
		hr = g_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
			hWnd, 
			D2D1::SizeU(rc.right - rc.left,rc.bottom - rc.top)
			), 
			&g_pRenderTarget
			) ;
		if (FAILED(hr))
		{
			MessageBox(hWnd, "Create render target failed!", "Error", 0) ;
			return ;
		}

		// Create a brush
		hr = g_pRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Black),
			&g_pBlackBrush
			) ;
		if (FAILED(hr))
		{
			MessageBox(hWnd, "Create brush failed!", "Error", 0) ;
			return ;
		}

		// Create a rectangle
		hr = g_pD2DFactory->CreateRectangleGeometry(
			D2D1::RectF(10, 10, 50, 50),
			&g_pRectangleGeometry
			);
		if (FAILED(hr))
		{
			MessageBox(hWnd, "Create rectangle geometry failed!", "Error", 0);
			return;
		}
	}
}

// Calculate the transform matrix
VOID CalculateTransform(D2D1_MATRIX_3X2_F* matrix)
{
	;
}

VOID DrawRectangle()
{
	// Calculate last time
	static float lastTime = (float)timeGetTime();

	// Calculate current time
	float currTime  = (float)timeGetTime();

	// Calculate time elapsed
	float timeDelta = (currTime - lastTime) * 0.001f;

	g_totalTime += timeDelta;

	CreateD2DResource(g_Hwnd) ;

	// Create a translation matrix based on the time elapsed
	D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Translation(g_totalTime, g_totalTime);

	// Create a transformed geometry
	HRESULT hr = g_pD2DFactory->CreateTransformedGeometry(
     g_pRectangleGeometry,
     matrix,
     &g_pTransformedGeometry
     );

	if (FAILED(hr))
	{
		MessageBox(NULL, "Create transformed geometry failed!", "Error", 0) ;
		return;
	}

	g_pRenderTarget->BeginDraw() ;

	// Clear background color to dark cyan
	g_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::DarkCyan));

	g_pRenderTarget->DrawGeometry(
		g_pTransformedGeometry,
		g_pBlackBrush
		);

	hr = g_pRenderTarget->EndDraw() ;
	if (FAILED(hr))
	{
		MessageBox(NULL, "Draw failed!", "Error", 0) ;

		return ;
	}
}

VOID Cleanup()
{
	SAFE_RELEASE(g_pRectangleGeometry);
	SAFE_RELEASE(g_pTransformedGeometry);
	SAFE_RELEASE(g_pRenderTarget) ;
	SAFE_RELEASE(g_pBlackBrush) ;
	SAFE_RELEASE(g_pD2DFactory) ;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)   
{
    switch (message)    
    {
	case   WM_PAINT:
		{
			DrawRectangle() ;
			return 0 ;
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
		Cleanup(); 
		PostQuitMessage( 0 ); 
		return 0; 
    }

	return DefWindowProc (hwnd, message, wParam, lParam) ;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow )
{
        
    WNDCLASSEX winClass ;

	winClass.lpszClassName = "Direct2D";
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
		MessageBox ( NULL, TEXT( "This program requires Windows NT!" ), "error", MB_ICONERROR) ;
		return 0 ;  
    }   
    
	g_Hwnd = CreateWindowEx(NULL,  
		"Direct2D",					// window class name
		"Draw Rectangle",			// window caption
		WS_OVERLAPPEDWINDOW, 		// window style
		CW_USEDEFAULT,				// initial x position
		CW_USEDEFAULT,				// initial y position
		600,						// initial x size
		600,						// initial y size
		NULL,						// parent window handle
		NULL,						// window menu handle
		hInstance,					// program instance handle
		NULL) ;						// creation parameters

        ShowWindow (g_Hwnd, iCmdShow) ;
		UpdateWindow (g_Hwnd) ;

		MSG    msg ;  
		ZeroMemory(&msg, sizeof(msg)) ;

		while (GetMessage (&msg, NULL, 0, 0))  
		{
			TranslateMessage (&msg) ;
			DispatchMessage (&msg) ;
		}

		return msg.wParam ;
}
