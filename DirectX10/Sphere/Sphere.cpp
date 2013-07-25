/*
Description:  This demo show you how to rotate a cube in DirectX10
Date: 2013-07-09
Author: zdd
*/

#include <windows.h>
#include <d3d10_1.h>
#include <d3dx10.h>
#include <d3dcompiler.h>
#include "DXUTShapes.h"

// DirectX Globals
ID3D10Device*			g_pd3dDevice		= NULL;
IDXGISwapChain*			g_pSwapChain		= NULL;
ID3D10RenderTargetView* g_pRenderTargetView = NULL;
ID3D10InputLayout*		g_pVertexLayout		= NULL;
ID3D10Effect*           g_pEffect			= NULL;
ID3D10EffectTechnique*  g_pTechnique		= NULL;
ID3DX10Mesh*			g_pSphere			= NULL;

ID3D10EffectMatrixVariable* g_pWorldVariable = NULL;
ID3D10EffectMatrixVariable* g_pViewVariable = NULL;
ID3D10EffectMatrixVariable* g_pProjectionVariable = NULL;

D3D10_RASTERIZER_DESC rasterDesc;
ID3D10RasterizerState*	g_pRasterizerState	= NULL;

// Is window active?
bool g_bActive = true ; 

// The vertex format
struct SimpleVertex
{
	D3DXVECTOR3 Pos;
};

D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

// Safe release a COM object
#define SAFE_RELEASE(P) if(P){ P->Release(); P = NULL;}

// Forward declaration
VOID CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
VOID InitWorldViewProjMatrix(HWND hwnd);
VOID InitEffects();
VOID InitRasterState();

HRESULT InitD3D( HWND hWnd )
{
	RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

	// Setup a DXGI swap chain descriptor
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );

	sd.BufferCount = 1; // number of buffer
	sd.BufferDesc.Width = width; // buffer width, can we set it to the screen width?
	sd.BufferDesc.Height = height; // buffer height, can we set it to the screen height?
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // buffer format, 32 bit color with alpha(RGBA)
	sd.BufferDesc.RefreshRate.Numerator = 60; // refresh rate?
	sd.BufferDesc.RefreshRate.Denominator = 1; // WHAT'S THIS?
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // use buffer as render target
	sd.OutputWindow = hWnd; // output window handle
	sd.SampleDesc.Count = 1; // WHAT'S THIS?
	sd.SampleDesc.Quality = 0; // WHAT'S THIS?
	sd.Windowed = TRUE; // full-screen mode

	// Create device and swap chain
	HRESULT hr;
	if (FAILED (hr = D3D10CreateDeviceAndSwapChain( NULL, 
	    D3D10_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		D3D10_SDK_VERSION,
		&sd, 
		&g_pSwapChain,
		&g_pd3dDevice)))
	{
		return hr;
	}

	// Create render target and bind the back-buffer
	ID3D10Texture2D* pBackBuffer;

	// Get a pointer to the back buffer
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID* )&pBackBuffer);
	if (FAILED(hr))
		return hr;

	// Create a render-target view
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);

	// Bind the view
	g_pd3dDevice->OMSetRenderTargets(1, &g_pRenderTargetView, NULL); // WHAT'S OM here mean?

	// Setup the viewport
	D3D10_VIEWPORT vp;
	vp.Width = width; // this should be similar with the back-buffer width, global it!
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pd3dDevice->RSSetViewports(1, &vp);

	// Create sphere
	DXUTCreateSphere(g_pd3dDevice, 3.0f, 20, 20, &g_pSphere);	

	InitWorldViewProjMatrix(hWnd);
	InitEffects();
	InitRasterState();
		                       
	return S_OK;
}

VOID InitWorldViewProjMatrix(HWND hwnd)
{
	// Initialize world matrix
	D3DXMatrixIdentity(&g_mWorld);

	// Initialize view matrix
	D3DXVECTOR3 eyePoint(5.0f, 3.0f, -5.0f);
	D3DXVECTOR3 lookAt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 Up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &eyePoint, &lookAt, &Up);

	// Initialize projection matrix
	float fov = (float)(D3DX_PI / 4);

	// Calculate aspect ratio
	RECT rc;
    GetClientRect(hwnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
	float aspectRatio = width / (float)height;

	D3DXMatrixPerspectiveFovLH(&g_mProj, fov, aspectRatio, 1.0f, 1000.0f);
}

VOID InitRasterState()
{
	rasterDesc.AntialiasedLineEnable = true;
	rasterDesc.CullMode = D3D10_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D10_FILL_WIREFRAME;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	g_pd3dDevice->CreateRasterizerState(&rasterDesc, &g_pRasterizerState);
}

VOID InitEffects()
{
	ID3DBlob* pErrorBlob;

	// Create the effect
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3D10_SHADER_DEBUG;
    #endif

	// Compile the effects file
    HRESULT hr = D3DX10CreateEffectFromFile( L"sphere.fx", NULL, NULL, "fx_4_0", dwShaderFlags, 0,
                                         g_pd3dDevice, NULL, NULL, &g_pEffect, &pErrorBlob, NULL);

    // Output the error message if compile failed
	if(FAILED(hr))
    {
        if(pErrorBlob != NULL)
		{
			OutputDebugStringA((CHAR*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
    }

	// Release the Blob
	if(pErrorBlob)
		pErrorBlob->Release();

    // Obtain the technique
    g_pTechnique = g_pEffect->GetTechniqueByName("Render");

	 // Obtain the variables
    g_pWorldVariable = g_pEffect->GetVariableByName("World")->AsMatrix();
    g_pViewVariable = g_pEffect->GetVariableByName("View")->AsMatrix();
    g_pProjectionVariable = g_pEffect->GetVariableByName("Projection")->AsMatrix();

    // Define the input layout
    D3D10_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = sizeof(layout) / sizeof(layout[0]);

    // Create the input layout
    D3D10_PASS_DESC PassDesc;
    g_pTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
    hr = g_pd3dDevice->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
                                          PassDesc.IAInputSignatureSize, &g_pVertexLayout);
    if(FAILED(hr))
	{
		MessageBox(NULL, L"Create input layout failed", L"Error", 0);
	}
}

VOID Cleanup()
{
	SAFE_RELEASE( g_pSphere );
	SAFE_RELEASE( g_pRasterizerState);
	SAFE_RELEASE( g_pVertexLayout);
	SAFE_RELEASE( g_pRenderTargetView );
	SAFE_RELEASE( g_pSwapChain ) ;
	SAFE_RELEASE( g_pd3dDevice ) ;
}

VOID SetupMatrix(float timeDelta)
{
	static float timeElapsed = 0.0f;
	timeElapsed += timeDelta;

	D3DXMatrixRotationY(&g_mWorld, timeElapsed);

	g_pWorldVariable->SetMatrix((float*)&g_mWorld);
    g_pViewVariable->SetMatrix((float*)&g_mView);
    g_pProjectionVariable->SetMatrix((float*)&g_mProj);
}

VOID Render(float timeDelta)
{
	if (!g_bActive)
		Sleep(50) ;

	SetupMatrix(timeDelta);

	// Clear the back-buffer to a BLUE color
	float color[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	g_pd3dDevice->ClearRenderTargetView(g_pRenderTargetView, color);

	// Set the input layout
	g_pd3dDevice->IASetInputLayout(g_pVertexLayout);

	// Set rasterazition state
	g_pd3dDevice->RSSetState(g_pRasterizerState);

	// Apply each pass in technique and draw triangle.
	D3D10_TECHNIQUE_DESC techDesc;
	g_pTechnique->GetDesc(&techDesc);
	for (unsigned int i = 0; i < techDesc.Passes; ++i)
	{
		g_pTechnique->GetPassByIndex(i)->Apply(0);
	}

	// Draw sphere
	g_pSphere->DrawSubset(0);

	// Present the sence from back buffer to front buffer
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

	winClass.lpszClassName = L"Sphere";
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
		L"Sphere",				// window caption
		WS_OVERLAPPEDWINDOW, 		// window style
		32,							// initial x position
		32,							// initial y position
		800,						// initial window width
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