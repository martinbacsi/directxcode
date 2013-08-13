#include "D3D10.h"

D3D10::D3D10(void)
    : d3ddevice_(NULL),
	  swapchain_(NULL),
	  rendertaregtview_(NULL),
	  is_fullscreen_(false)
{
	camera = new Camera();
}

D3D10::~D3D10(void)
{
	delete camera;
	camera = NULL;

	// Release Direct3D Device
	if (d3ddevice_ != NULL)
	{
		d3ddevice_->Release();
		d3ddevice_ = NULL;
	}

	// Release swap chain
	if (swapchain_ != NULL)
	{
		swapchain_->Release();
		swapchain_ = NULL;
	}
}

void D3D10::InitD3D10(HWND hWnd)
{
	this->hWnd = hWnd;

	// Create DXGI factory
	IDXGIFactory* pFactory = NULL;
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Create DXGI factory failed", L"Error", 0);
	}

	// Enumerate adapter
	// NOTE: I assume the system only has one adapter here, if the system has mutiple video cards
	// the code need update.
	IDXGIAdapter* pAdapter = NULL;
	pFactory->EnumAdapters(0, &pAdapter);

	// Get output of the adapter
	// NOTE: I assume the adapter only has one output here, if the adpater has more than one output,
	// for example multiple monitors, then the code need update.
	IDXGIOutput* pOutput = NULL;
	pAdapter->EnumOutputs(0, &pOutput);

	// Get output description
	DXGI_OUTPUT_DESC outputDesc;
	pOutput->GetDesc(&outputDesc);

	// Get the screen size
	screen_width_  = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
	screen_height_ = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;

	// Get client area size of the render window
	RECT rc;
    GetClientRect(hWnd, &rc);
    UINT window_width = rc.right - rc.left;
    UINT window_height = rc.bottom - rc.top;

	// Setup a DXGI swap chain descriptor
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );

	if (is_fullscreen_)
	{
		sd.Windowed = false;
		sd.BufferDesc.Width = screen_width_;
		sd.BufferDesc.Height = screen_height_;
	}
	else
	{
		sd.Windowed = true;
		sd.BufferDesc.Width = window_width;
		sd.BufferDesc.Height = window_height;
	}

	sd.BufferCount = 1; // number of buffer
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
		&swapchain_,
		&d3ddevice_)))
	{
		MessageBox(NULL, L"Create device and swap chain failed", L"Error", 0);
	}

	// Create render target and bind the back-buffer
	ID3D10Texture2D* pBackBuffer;

	// Get a pointer to the back buffer
	hr = swapchain_->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID* )&pBackBuffer);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Get backbuffer failed", L"Error", 0);
	}

	// Create a render-target view
	d3ddevice_->CreateRenderTargetView(pBackBuffer, NULL, &rendertaregtview_);

	// Bind the view
	d3ddevice_->OMSetRenderTargets(1, &rendertaregtview_, NULL);

	// Setup the viewport
	D3D10_VIEWPORT vp;

	if (is_fullscreen_)
	{
		vp.Width  = screen_width_;
		vp.Height = screen_height_;
	}
	else
	{
		vp.Width  = window_width;
		vp.Height = window_height;
	}

	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	d3ddevice_->RSSetViewports(1, &vp);
}

LPDIRECT3DTEXTURE9 D3D10::CreateTexture(int texWidth, int texHeight, D3DCOLOR color)
{
	// Create a texture Description and fill it.
	D3D10_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));

	texDesc.ArraySize		= 1;				// Number of textures
	texDesc.Usage			= D3D10_USAGE_DYNAMIC; 
	texDesc.BindFlags		= D3D10_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags	= D3D10_CPU_ACCESS_WRITE;	// CPU will write this resource
	texDesc.Format			= DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Width			= texWidth;
	texDesc.Height			= texHeight;
	texDesc.MipLevels		= 1;
	texDesc.MiscFlags		= 0;

	DXGI_SAMPLE_DESC sampleDes;
	ZeroMemory(&sampleDes, sizeof(sampleDes));
	sampleDes.Count = 1;
	sampleDes.Quality = 0;
	texDesc.SampleDesc = sampleDes;

	// Create sub resource
	D3D10_SUBRESOURCE_DATA texData;
	ZeroMemory(&texData, sizeof(texData));
	texData.pSysMem = 0;
	texData.SysMemPitch = 0;
	texData.SysMemSlicePitch = 0;

	// Create texture
	ID3D10Texture2D* texture2d = NULL;
	HRESULT hr = d3ddevice_->CreateTexture2D(&texDesc, NULL, &texture2d);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Create texture in memory failed", L"Error", 0);
	}

	// Lock texture and fill in colors
	D3D10_MAPPED_TEXTURE2D mappedTex;
	ZeroMemory(&mappedTex, sizeof(mappedTex));

	texture2d->Map(0, D3D10_MAP_WRITE_DISCARD, 0, &mappedTex);

	// Fill in colors
	//============================= BE CAREFUL OF THE COLOR ORDER R8G8B8A8=====================

	// Calculate number of rows
	// total bytes = texture_width * texture_height * bytes_in_texel
	// We use DXGI_FORMAT_R8G8B8A8_UINT, so each texel is 4 bytes
	// RowPitch is number of bytes in one row.
	int numRows = texWidth * texHeight * 4 / mappedTex.RowPitch;
	for (int i = 0; i < texWidth; ++i)
	{
		for (int j = 0; j < texHeight; ++j)
		{
			int index = i * numRows + j;
			int* pData = (int*)mappedTex.pData;
			memcpy(&pData[index], &color, 4);
		}
	}

	// Unlock texture
	texture2d->Unmap(0);

	// Create shader resource view to bind the texture
	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;

	D3D10_TEXTURE2D_DESC desc;
	texture2d->GetDesc(&desc);
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = desc.MipLevels - 1;

	ID3D10ShaderResourceView* textureRV = NULL;
	hr = d3ddevice_->CreateShaderResourceView(texture2d, &srvDesc, &textureRV);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Create shader resource view failed", L"Error", 0);
	}

	// Bind to shader variables
	g_pDiffuseVariable->SetResource(textureRV);
}

LPDIRECT3DTEXTURE9 D3D10::CreateInnerTexture(int texWidth, int texHeight, D3DCOLOR color)
{
	// Create a texture Description and fill it.
	D3D10_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));

	texDesc.ArraySize		= 1;				// Number of textures
	texDesc.Usage			= D3D10_USAGE_DYNAMIC; 
	texDesc.BindFlags		= D3D10_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags	= D3D10_CPU_ACCESS_WRITE;	// CPU will write this resource
	texDesc.Format			= DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Width			= texWidth;
	texDesc.Height			= texHeight;
	texDesc.MipLevels		= 1;
	texDesc.MiscFlags		= 0;

	DXGI_SAMPLE_DESC sampleDes;
	ZeroMemory(&sampleDes, sizeof(sampleDes));
	sampleDes.Count = 1;
	sampleDes.Quality = 0;
	texDesc.SampleDesc = sampleDes;

	// Create sub resource
	D3D10_SUBRESOURCE_DATA texData;
	ZeroMemory(&texData, sizeof(texData));
	texData.pSysMem = 0;
	texData.SysMemPitch = 0;
	texData.SysMemSlicePitch = 0;

	// Create texture
	ID3D10Texture2D* texture2d = NULL;
	HRESULT hr = d3ddevice_->CreateTexture2D(&texDesc, NULL, &texture2d);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Create texture in memory failed", L"Error", 0);
	}

	// Lock texture and fill in colors
	D3D10_MAPPED_TEXTURE2D mappedTex;
	ZeroMemory(&mappedTex, sizeof(mappedTex));

	texture2d->Map(0, D3D10_MAP_WRITE_DISCARD, 0, &mappedTex);

	// Fill in colors
	//============================= BE CAREFUL OF THE COLOR ORDER R8G8B8A8=====================

	// Calculate number of rows
	// total bytes = texture_width * texture_height * bytes_in_texel
	// We use DXGI_FORMAT_R8G8B8A8_UINT, so each texel is 4 bytes
	// RowPitch is number of bytes in one row.
	int numRows = texWidth * texHeight * 4 / mappedTex.RowPitch;
	for (int i = 0; i < texWidth; ++i)
	{
		for (int j = 0; j < texHeight; ++j)
		{
			int index = i * numRows + j;
			int* pData = (int*)mappedTex.pData;
			memcpy(&pData[index], &color, 4);
		}
	}

	// Unlock texture
	texture2d->Unmap(0);

	// Create shader resource view to bind the texture
	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;

	D3D10_TEXTURE2D_DESC desc;
	texture2d->GetDesc(&desc);
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = desc.MipLevels - 1;

	ID3D10ShaderResourceView* textureRV = NULL;
	hr = d3ddevice_->CreateShaderResourceView(texture2d, &srvDesc, &textureRV);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Create shader resource view failed", L"Error", 0);
	}

	// Bind to shader variables
	g_pDiffuseVariable->SetResource(textureRV);
}

void D3D10::SetupMatrix()
{
	// View matrix
	D3DXMATRIX matView = camera->GetViewMatrix() ;
	d3ddevice_->SetTransform(D3DTS_VIEW, &matView) ;

	// Projection matrix
	D3DXMATRIX matProj = camera->GetProjMatrix() ;
	d3ddevice_->SetTransform(D3DTS_PROJECTION, &matProj) ;
}

void D3D10::SetupLight()
{
	// Create light
	D3DLIGHT9 pointLight ;

	// Light color
	D3DXCOLOR color = D3DCOLOR_XRGB(255, 255, 255) ;

	// The light position is always same as the camera eye point
	// so no matter how you rotate the camera, the cube will keep the same brightness
	 D3DXVECTOR3 position = camera->GetEyePoint() ;

	// Light type, we use point light here
	pointLight.Type			= D3DLIGHT_POINT ;

	// Light attributes
	pointLight.Ambient		= color * 0.6f;
	pointLight.Diffuse		= color;
	pointLight.Specular		= color * 0.6f;
	pointLight.Position		= position;
	pointLight.Range		= 320.0f;
	pointLight.Falloff		= 1.0f;
	pointLight.Attenuation0	= 1.0f;
	pointLight.Attenuation1	= 0.0f;
	pointLight.Attenuation2	= 0.0f;

	// Set material
	D3DMATERIAL9 material ;
	material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0) ;
	material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0);
	material.Specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0);
	material.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
	material.Power = 2.0f ;
	d3ddevice_->SetMaterial(&material) ;

	// Enable light
	d3ddevice_->SetLight(0, &pointLight) ;		
	d3ddevice_->LightEnable(0, true) ;		
}

void D3D10::ResizeD3DScene(int width, int height)
{
	if (height == 0)				// Prevent A Divide By Zero By
		height = 1;					// Making Height Equal One

	// Compute aspect ratio
	float fAspectRatio = width / (FLOAT)height;

	// Setup Projection matrix
	camera->SetProjParams(D3DX_PI / 4, fAspectRatio, 1.0f, 1000.0f);

	camera->SetWindow(width, height);
}

void D3D10::FrameMove()
{
	camera->OnFrameMove() ;
}

// Calculate the picking ray and transform it to model space 
// x and y are the screen coordinates when left button down
Ray D3D10::CalculatePickingRay(int x, int y)
{
	float px = 0.0f;
	float py = 0.0f;

	// Get viewport
	D3D10_VIEWPORT vp;
	d3ddevice_->RSGetViewports(NULL, &vp);

	// Get Projection matrix
	D3DXMATRIX proj;
	d3ddevice_->
	d3ddevice_->GetTransform(D3DTS_PROJECTION, &proj);

	px = ((( 2.0f*x) / vp.Width)  - 1.0f) / proj(0, 0);
	py = (((-2.0f*y) / vp.Height) + 1.0f) / proj(1, 1);

	Ray ray;
	ray.origin    = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	ray.direction = D3DXVECTOR3(px, py, 1.0f); 

	// Get view matrix
	D3DXMATRIX view;
	d3ddevice_->GetTransform(D3DTS_VIEW, &view);

	// Get world matrix
	D3DXMATRIX world;
	d3ddevice_->GetTransform(D3DTS_WORLD, &world);

	// Concatinate them in to single matrix
	D3DXMATRIX WorldView = world * view;

	// inverse it
	D3DXMATRIX worldviewInverse;
	D3DXMatrixInverse(&worldviewInverse, 0, &WorldView);


	// Transform the ray to model space
	D3DXVec3TransformCoord(&ray.origin, &ray.origin, &worldviewInverse) ;
	D3DXVec3TransformNormal(&ray.direction, &ray.direction, &worldviewInverse) ;

	// Normalize the direction
	D3DXVec3Normalize(&ray.direction, &ray.direction) ;

	return ray;
}

// Transform the screen point to vector in model space
D3DXVECTOR3 D3D10::ScreenToVector3(int x, int y)
{
	D3DXVECTOR3 vector3 ;

	// Get viewport
	D3D10_VIEWPORT vp;
	d3ddevice_->RSGetViewports(NULL, &vp);

	// Get Projection matrix
	D3DXMATRIX proj;
	d3ddevice_->GetTransform(D3DTS_PROJECTION, &proj);

	vector3.x = ((( 2.0f*x) / vp.Width)  - 1.0f) / proj(0, 0);
	vector3.y = (((-2.0f*y) / vp.Height) + 1.0f) / proj(1, 1);
	vector3.z = 1.0f ;

	// Get view matrix
	D3DXMATRIX view;
	d3ddevice_->GetTransform(D3DTS_VIEW, &view);

	// Get world matrix
	D3DXMATRIX world;
	d3ddevice_->GetTransform(D3DTS_WORLD, &world);

	// Concatinate them in to single matrix
	D3DXMATRIX WorldView = world * view;

	// inverse it
	D3DXMATRIX worldviewInverse;
	D3DXMatrixInverse(&worldviewInverse, 0, &WorldView);

	D3DXVec3TransformCoord(&vector3, &vector3, &worldviewInverse) ;

	D3DXVec3Normalize(&vector3, &vector3) ;

	return vector3 ;
}

LRESULT D3D10::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return camera->HandleMessages(hWnd, uMsg, wParam, lParam);
}

ID3D10Device* D3D10::GetD3DDevice() const
{
	return d3ddevice_;
}

D3DPRESENT_PARAMETERS D3D10::GetD3Dpp() const
{
	return d3dpp_;
}

void D3D10::SetBackBufferWidth(int width)
{
	d3dpp_.BackBufferWidth = width;
}

void D3D10::SetBackBufferHeight(int height)
{
	d3dpp_.BackBufferHeight = height;
}

void D3D10::SetIsFullScreen(bool is_fullscreen)
{
	is_fullscreen_ = is_fullscreen;
	d3dpp_.Windowed = !is_fullscreen;
}

bool D3D10::GetIsFullScreen() const
{
	return is_fullscreen_;
}

int	D3D10::GetLastWindowWidth() const
{
	return last_window_width_;
}

void D3D10::SetLastWindowWidth(int windowWidth)
{
	last_window_width_ = windowWidth;
}

int	D3D10::GetLastWindowHeight() const
{
	return last_window_height_;
}

void D3D10::SetLastWindowHeight(int windowHeight)
{
	last_window_height_ = windowHeight;
}

int	D3D10::GetScreenWidth() const
{
	return screen_width_;
}

int D3D10::GetScreenHeight() const
{
	return screen_height_;
}
