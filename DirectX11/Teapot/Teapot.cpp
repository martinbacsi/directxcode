#include <Windows.h>
#include <mmsyscom.h>
#include <D3DX11.h>
#include <D3DX10math.h>

ID3D11Device*			g_pd3dDevice		= NULL;
ID3D11DeviceContext*	g_pImmediateContext = NULL;
IDXGISwapChain*			g_pSwapChain		= NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;

bool					g_bActive			= true ; // Is window active?

#define SAFE_RELEASE(P) if(P){ P->Release(); P = NULL;}

HRESULT InitD3D( HWND hWnd )
{
	// Setup a DXGI swap chain descriptor
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );

	sd.BufferCount = 1; // number of buffer
	sd.BufferDesc.Width = 640; // buffer width, can we set it to the screen width?
	sd.BufferDesc.Height = 480; // buffer height, can we set it to the screen height?
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // buffer format, 32 bit color with alpha(RGBA)
	sd.BufferDesc.RefreshRate.Numerator = 60; // refresh rate?
	sd.BufferDesc.RefreshRate.Denominator = 1; // WHAT'S THIS?
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // use buffer as render target
	sd.OutputWindow = hWnd; // output window handle
	sd.SampleDesc.Count = 1; // WHAT'S THIS?
	sd.SampleDesc.Quality = 0; // WHAT'S THIS?
	sd.Windowed = FALSE; // window mode

	// Create device and swap chain
	D3D_FEATURE_LEVEL FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0; // Use d3d11
	UINT              numLevelsRequested = 1; // Number of levels 
	D3D_FEATURE_LEVEL FeatureLevelsSupported;

	HRESULT hr;
	if (FAILED (hr = D3D11CreateDeviceAndSwapChain( NULL, 
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&FeatureLevelsRequested,
		numLevelsRequested,
		D3D11_SDK_VERSION,
		&sd, 
		&g_pSwapChain,
		&g_pd3dDevice,
		&FeatureLevelsSupported,
		&g_pImmediateContext )))
	{
		return hr;
	}

	// Create render target and bind the back-buffer
	ID3D11Texture2D* pBackBuffer;

	// Get a pointer to the back buffer
	hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );

	// Create a render-target view
	g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView);

	// Bind the view
	g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, NULL ); // WHAT'S OM here mean?

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = 640; // this should be similar with the back-buffer width, global it!
	vp.Height = 480;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports( 1, &vp );
		                       
	return S_OK;
}

VOID Cleanup()
{
	SAFE_RELEASE( g_pd3dDevice ) ;
	SAFE_RELEASE( g_pImmediateContext ) ;
	SAFE_RELEASE( g_pSwapChain ) ;
	SAFE_RELEASE( g_pRenderTargetView );
}

void SetupMatrix()
{
	// translate model to origin
	D3DXMATRIX world ;
	D3DXMatrixTranslation(&world, 0.0f, 0.0f, 0.0f) ;

	// set view
	D3DXVECTOR3 eyePt(0.0f, 0.0f, -10.0f) ;
	D3DXVECTOR3 upVec(0.0f, 1.0f, 0.0f) ;
	D3DXVECTOR3 lookCenter(0.0f, 0.0f, 0.0f) ;

	D3DXMATRIX view ;
	D3DXMatrixLookAtLH(&view, &eyePt, &lookCenter, &upVec) ;

	// set projection
	D3DXMATRIX proj ;
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 4, 1.0f, 1.0f, 1000.0f) ;
}

void Render(float timeDelta)
{
	if (!g_bActive)
	{
		Sleep(50) ;
	}

	SetupMatrix() ;

	// Clear the back-buffer to a RED color
	float color[4];
	color[0] = 1.0f; // red
	color[1] = 0.0f; // green
	color[2] = 0.0f; // blue
	color[3] = 0.0f; // alpha

	g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, color );

	// Draw
	g_pSwapChain->Present(0, 0);
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_KEYDOWN:
		{
			switch( wParam )
			{
			case VK_ESCAPE:
				SendMessage( hWnd, WM_CLOSE, 0, 0 );
				break ;
			default:
				break ;
			}
		}
		return 0 ;

	case WM_SIZE:
		if(wParam == SIZE_MAXHIDE || wParam == SIZE_MINIMIZED)
			g_bActive = false;
		else
			g_bActive = true;
		return 0;

	case WM_ACTIVATEAPP:
		if(wParam == TRUE)
			g_bActive = true ;
		else
			g_bActive = false ;
		return 0 ;

	case WM_DESTROY:
		Cleanup();
		PostQuitMessage( 0 );
		return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR szCmdLine, int iCmdShow)
{
	WNDCLASSEX winClass ;

	winClass.lpszClassName = "Teapot";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = MsgProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon	       = NULL ;
	winClass.hIconSm	   = NULL ;
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW) ;
	winClass.hbrBackground = NULL ;
	winClass.lpszMenuName  = NULL ;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	RegisterClassEx (&winClass) ;  

	HWND hWnd = CreateWindowEx(NULL,  
		winClass.lpszClassName,		// window class name
		"Teapot",					// window caption
		WS_OVERLAPPEDWINDOW, 		// window style
		32,							// initial x position
		32,							// initial y position
		600,						// initial window width
		600,						// initial window height
		NULL,						// parent window handle
		NULL,						// window menu handle
		hInstance,					// program instance handle
		NULL) ;						// creation parameters

	// Create window failed
	if(hWnd == NULL)
	{
		MessageBoxA(hWnd, "Create Window failed!", "Error", 0) ;
		return -1 ;
	}

	// Initialize Direct3D
	if( SUCCEEDED(InitD3D(hWnd)))
	{ 
		// Show the window
		ShowWindow( hWnd, SW_SHOWDEFAULT );
		UpdateWindow( hWnd );

		MSG msg ; 
		ZeroMemory( &msg, sizeof(msg) );
		PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

		// Get last time
		static DWORD lastTime = timeGetTime();

		while (msg.message != WM_QUIT)  
		{
			if(PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) != 0)
			{
				TranslateMessage (&msg) ;
				DispatchMessage (&msg) ;
			}
			else // Render the game if there is no message to process
			{
				// Get current time
				DWORD currTime  = timeGetTime();

				// Calculate time elapsed
				float timeDelta = (currTime - lastTime) * 0.001f;

				// Render
				Render(timeDelta) ;

				// Update last time to current time for next loop
				lastTime = currTime;
			}
		}
	}

	UnregisterClass(winClass.lpszClassName, hInstance) ;
	return 0;
}
