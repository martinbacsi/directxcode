#include <windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "ModelViewCamera.h"

using namespace DirectX;

// The vertex format
struct SimpleVertex
{
	DirectX::XMFLOAT3 Pos;	// Position
	DirectX::XMFLOAT3 Color; // color
};

// Constant buffer
struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};

ID3D11Device*			g_pd3dDevice		= NULL;
ID3D11DeviceContext*	g_pImmediateContext = NULL;
IDXGISwapChain*			g_pSwapChain		= NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11InputLayout*		g_pVertexLayout		= NULL;
ID3D11Buffer*			g_pVertexBuffer     = NULL;
ID3D11Buffer*			g_pIndexBuffer      = NULL;
ID3D11Buffer*			g_pConstantBuffer   = NULL;
ID3D11VertexShader*		g_pVertexShader		= NULL;
ID3D11PixelShader*		g_pPixelShader		= NULL;

// World, view and projection matrix
XMMATRIX				g_mWorld;
XMMATRIX				g_mView;
XMMATRIX                g_mProj;

bool					g_bActive			= true ; // Is window active?

ModelViewCamera*         g_Camera;

// This Macro used to release COM object
#define SAFE_RELEASE(P) if(P){ P->Release(); P = NULL;}

// Forward declaration
VOID CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
VOID InitVertexBuffer();
VOID InitIndexBuffer();
VOID InitConstantBuffer();
VOID InitVertexShader();
VOID InitPixelShader();
VOID InitWorldViewProjMatrix(HWND hWnd);

HRESULT InitD3D( HWND hWnd )
{
	// Get client size
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int width  = rect.right - rect.left;
	int height = rect.bottom - rect.top;

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
	vp.Width = width; // this should be similar with the back-buffer width, global it!
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports( 1, &vp );

	// Set camera window
	g_Camera = new ModelViewCamera();
	g_Camera->SetWindow(width, height);
	g_Camera->Reset();
	
	InitWorldViewProjMatrix(hWnd);
	InitVertexBuffer();
	InitIndexBuffer();
	InitConstantBuffer();
	InitVertexShader();
	InitPixelShader();
		                       
	return S_OK;
}

VOID InitVertexBuffer()
{
	
	// Create the vertex buffer
	SimpleVertex vertices[] = 
	{
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f) },
        { XMFLOAT3( 1.0f,  1.0f, -1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f) },
        { XMFLOAT3( 1.0f,  1.0f,  1.0f ), XMFLOAT3( 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f,  1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 1.0f, 1.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f,  1.0f ), XMFLOAT3( 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f,  1.0f ), XMFLOAT3( 0.0f, 0.0f, 0.0f) },
	};

	// Vertex Buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;		// Buffer usage
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// copy vertex buffer data
	D3D11_SUBRESOURCE_DATA initData;
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
	unsigned short cubeIndices[] = 
	{
		3,1,0,
        2,1,3,

        0,5,4,
        1,5,0,

        3,4,7,
        0,4,3,

        1,6,5,
        2,6,1,

        2,7,6,
        3,7,2,

        6,4,5,
        7,4,6,
	};

	// Index buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;		// Buffer usage
	bd.ByteWidth = sizeof(cubeIndices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;


	// Create index buffer
	D3D11_SUBRESOURCE_DATA indexBufferData = {0};
	indexBufferData.pSysMem = cubeIndices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);

	HRESULT hr = g_pd3dDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &g_pIndexBuffer);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create index buffer failed", L"Error", 0);
	}
}

VOID InitConstantBuffer()
{
	// Create constant buffer
	CD3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	HRESULT hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pConstantBuffer);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create constant buffer failed", L"Error", 0);
	}
}

VOID InitVertexShader()
{
	// Compile the vertex shader from file
	ID3DBlob*	pVSBlob = NULL;
	CompileShaderFromFile(L"arcball.fx", "VS", "vs_4_0", &pVSBlob);

	// Create vertex shader
	HRESULT hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
	if(FAILED(hr))
	{
		pVSBlob->Release();
		MessageBox(NULL, L"Create vertex shader failed", L"Error", 0);
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), 
													pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
}

VOID InitPixelShader()
{
	// Compile the pixel shader
	ID3DBlob*	pPSBlob = NULL;
	CompileShaderFromFile(L"arcball.fx", "PS", "ps_4_0", &pPSBlob);

	// Create the pixel shader
	HRESULT hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
	pPSBlob->Release();
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create pixel shader failed", L"Error", 0);
	}
}

VOID InitWorldViewProjMatrix(HWND hwnd)
{
	// Initialize world matrix
	XMMATRIX world = XMMatrixIdentity();
	g_Camera->SetWorldMatrix(&world);

	// Initialize view matrix
	XMFLOAT3 eyePoint = XMFLOAT3(0.0f, 0.0f, -1.0f);
	XMFLOAT3 lookAt   = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 Up       = XMFLOAT3(0.0f, 1.0f, 0.0f);
	g_Camera->SetViewParams(eyePoint, lookAt, Up);

	// Initialize projection matrix
	float fov = XM_PIDIV4;

	// Calculate aspect ratio
	RECT rc;
    GetClientRect(hwnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
	float aspectRatio = width / (float)height;
	g_Camera->SetProjParams(fov, aspectRatio, 1.0f, 1000.0f);
}

VOID CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	ID3DBlob*	pErrorBlob;

	hr = D3DCompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if(FAILED(hr))
	{
		if(pErrorBlob != NULL)
		{
			//OutputDebugStringA(pErrorBlob->GetBufferPointer());
			MessageBox(NULL, (WCHAR*)pErrorBlob->GetBufferPointer(), L"Error", 0);
			pErrorBlob->Release();
		}
	}

	if(pErrorBlob)
		pErrorBlob->Release();
}

VOID Cleanup()
{
	if(g_pImmediateContext) 
		g_pImmediateContext->ClearState();

	SAFE_RELEASE( g_pVertexBuffer);
	SAFE_RELEASE( g_pIndexBuffer);
	SAFE_RELEASE( g_pConstantBuffer);
	SAFE_RELEASE( g_pVertexLayout);
	SAFE_RELEASE( g_pVertexShader);
	SAFE_RELEASE( g_pPixelShader);
	SAFE_RELEASE( g_pRenderTargetView );
	SAFE_RELEASE( g_pSwapChain ) ;
	SAFE_RELEASE( g_pImmediateContext ) ;
	SAFE_RELEASE( g_pd3dDevice ) ;

	delete g_Camera;
	g_Camera = NULL;
}

VOID OnFrameMove()
{
	g_Camera->OnFrameMove() ;
}

VOID Render(float timeDelta)
{
	if (!g_bActive)
		Sleep(50) ;

	OnFrameMove();

	// Clear the back-buffer to a BLUE color
	float color[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, color);

	// Set up matrix
	ConstantBuffer cb;
	XMMATRIX world = *g_Camera->GetWorldMatrix();
	XMMATRIX view  = *g_Camera->GetViewMatrix();
	XMMATRIX proj  = *g_Camera->GetProjMatrix();

	cb.mWorld      = XMMatrixTranspose(world);
	cb.mView       = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(proj);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);

	// Set vertext shader, pixel shader and constant buffer
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// Set index buffer
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set constant buffer
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	// Set geometry type
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw
	g_pImmediateContext->DrawIndexed(36, 0, 0);

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

	g_Camera->HandleMessages(hWnd, msg, wParam, lParam) ;

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR szCmdLine, int iCmdShow)
{
	WNDCLASSEX winClass ;

	winClass.lpszClassName = L"ArcBall";
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
		L"ArcBall",				// window caption
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
