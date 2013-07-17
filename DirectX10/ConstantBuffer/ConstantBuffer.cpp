/*
Description:  This demo show you how to use constant buffer in DirectX 10, we only use constant buffer in shader file
but not in program, it seems direct3d 10 does not support to update shader variables as DirectX 11 did as below
	// Set up matrix
	ConstantBuffer cb;
	cb.mWorld      = XMMatrixTranspose(g_mWorld);
	cb.mView       = XMMatrixTranspose(g_mView);
	cb.mProjection = XMMatrixTranspose(g_mProj);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);

Date: 2013-07-10
Author: zdd
*/

#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>
#include <d3dcompiler.h>

// DirectX Globals
ID3D10Device*			g_pd3dDevice		= NULL;
IDXGISwapChain*			g_pSwapChain		= NULL;
ID3D10RenderTargetView* g_pRenderTargetView = NULL;
ID3D10InputLayout*		g_pVertexLayout		= NULL;
ID3D10Buffer*			g_pVertexBuffer     = NULL;
ID3D10Buffer*			g_pIndexBuffer      = NULL;
ID3D10Buffer*			g_pConstantBuffer	= NULL;
ID3D10Effect*           g_pEffect			= NULL;
ID3D10EffectTechnique*  g_pTechnique		= NULL;

// Effect variables
ID3D10EffectMatrixVariable* g_pWorldVariable = NULL;
ID3D10EffectMatrixVariable* g_pViewVariable = NULL;
ID3D10EffectMatrixVariable* g_pProjVariable = NULL;

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
VOID InitVertexBuffer();
VOID InitIndexBuffer();
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
	UINT flags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;

#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3D10_CREATE_DEVICE_DEBUG;
#endif 

	if (FAILED (hr = D3D10CreateDeviceAndSwapChain( NULL, 
	    D3D10_DRIVER_TYPE_HARDWARE,
		NULL,
		flags,
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

	InitWorldViewProjMatrix(hWnd);
	InitVertexBuffer();
	InitIndexBuffer();
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
	rasterDesc.CullMode = D3D10_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D10_FILL_WIREFRAME;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	g_pd3dDevice->CreateRasterizerState(&rasterDesc, &g_pRasterizerState);
}

VOID InitVertexBuffer()
{
	// Create the vertex buffer
	SimpleVertex vertices[] = 
	{
		D3DXVECTOR3(-1.0f, -1.0f, -1.0f), // 0
		D3DXVECTOR3(-1.0f,  1.0f, -1.0f), // 1
		D3DXVECTOR3( 1.0f,  1.0f, -1.0f), // 2
		D3DXVECTOR3( 1.0f, -1.0f, -1.0f), // 3

		D3DXVECTOR3( 1.0f, -1.0f,  1.0f), // 4
		D3DXVECTOR3( 1.0f,  1.0f,  1.0f), // 5
		D3DXVECTOR3(-1.0f,  1.0f,  1.0f), // 6
		D3DXVECTOR3(-1.0f, -1.0f,  1.0f), // 7
	};

	// Vertex Buffer
	D3D10_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D10_USAGE_DEFAULT;		// Buffer usage
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// copy vertex buffer data
	D3D10_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;
	HRESULT	hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pVertexBuffer);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create vertex buffer failed", L"Error", 0);
	}
}

VOID InitIndexBuffer()
{
	// Indices array
	DWORD indices[] =
	{
		0, 1, 2, 0, 2, 3, // front face
		4, 5, 6, 4, 6, 7, // back face
		7, 6, 1, 7, 1, 0, // left face
		3, 2, 5, 3, 5, 4, // right face
		1, 6, 5, 1, 5, 2, // top face
		3, 4, 7, 3, 7, 0, // bottom face
	};

	D3D10_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(indices);
	bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// Copy index buffer data
	D3D10_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = indices;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;
	HRESULT hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pIndexBuffer);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Create index buffer failed", L"Error", 0);
	}
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

	//// Compile the effects file
    HRESULT hr = D3DX10CreateEffectFromFile( L"constant_buffer.fx", NULL, NULL, "fx_4_0", dwShaderFlags, 0,
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
	g_pViewVariable  = g_pEffect->GetVariableByName("View")->AsMatrix();
	g_pProjVariable  = g_pEffect->GetVariableByName("Projection")->AsMatrix();

    // Define the input layout
    D3D10_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = sizeof(layout) / sizeof(layout[0]);

    // Create the input layout
    D3D10_PASS_DESC PassDesc;
    g_pTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
    hr = g_pd3dDevice->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
                                          PassDesc.IAInputSignatureSize, &g_pVertexLayout);
    if(FAILED(hr))
	{
		MessageBox(NULL, L"Create input layout failed", L"Error", 0);
	}
}

VOID Cleanup()
{
	SAFE_RELEASE( g_pVertexBuffer);
	SAFE_RELEASE( g_pIndexBuffer);
	SAFE_RELEASE( g_pConstantBuffer);
	SAFE_RELEASE( g_pVertexLayout);
	SAFE_RELEASE( g_pRenderTargetView );
	SAFE_RELEASE( g_pSwapChain ) ;
	SAFE_RELEASE( g_pd3dDevice ) ;
}

VOID SetupMatrix(float timeDelta)
{
	g_pWorldVariable->SetMatrix(g_mWorld);
	g_pViewVariable->SetMatrix(g_mView);
	g_pProjVariable->SetMatrix(g_mProj);
}

VOID Render(float timeDelta)
{
	if (!g_bActive)
		Sleep(50) ;

	SetupMatrix(timeDelta);

	// Clear the back-buffer to a BLUE color
	float color[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	g_pd3dDevice->ClearRenderTargetView(g_pRenderTargetView, color);

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pd3dDevice->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// Set index buffer
	g_pd3dDevice->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set constant buffer
	g_pd3dDevice->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	// Set the input layout
	g_pd3dDevice->IASetInputLayout(g_pVertexLayout);

	// Set geometry type
	g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set rasterazition state
	g_pd3dDevice->RSSetState(g_pRasterizerState);

	// Apply each pass in technique and draw triangle.
	D3D10_TECHNIQUE_DESC techDesc;
	g_pTechnique->GetDesc(&techDesc);
	for (unsigned int i = 0; i < techDesc.Passes; ++i)
	{
		g_pTechnique->GetPassByIndex(i)->Apply(0);
		g_pd3dDevice->DrawIndexed(36, 0, 0);
	}

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

	winClass.lpszClassName = L"ConstantBuffer";
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
		L"ConstantBuffer",				// window caption
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
