#include "RubikCube.h"
#include "DXErr.h"
#include <time.h>

RubikCube::RubikCube(void)
	: d3ddevice_(NULL),
      swap_chain_(NULL),
	  immediate_context_(NULL),
	  vertex_shader_(NULL),
	  pixel_shader_(NULL),
	  output_(NULL),
	  input_layout_(NULL),
	  rendertarget_view_(NULL),
	  depth_stencil_view_(NULL),
	  is_fullscreen_(false),
	  kNumLayers(3),
      kNumCubes(kNumLayers * kNumLayers * kNumLayers),
	  kNumFaces(6),
      gap_between_layers_(0.15f),
	  total_rotate_angle_(0),
	  rotate_speed_(1.0f),
	  is_hit_(false),
	  hit_layer_(-1),
	  is_cubes_selected_(false),
	  rotate_finish_(true),
	  window_active_(false),
	  init_window_x_(0),
	  init_window_y_(0),
	  init_window_width_(1000),
	  init_window_height_(1000),
	  screen_width_(0),
	  screen_height_(0),
	  current_window_width_(init_window_width_),
	  current_window_height_(init_window_height_),
	  last_window_width_(current_window_width_),
	  last_window_height_(current_window_height_)
{
	world_arcball_ = new ArcBall();

	camera_ = new Camera();

	// Create 27 unit cubes
	cubes = new Cube[kNumCubes];

	// Create 6 faces
	faces = new Rect[kNumFaces];

	// Calculate face length and half face length which will used later to determine unit cube layer.
	float cube_length = cubes[0].GetLength();
	face_length_ = kNumLayers * cube_length + (kNumLayers - 1) * gap_between_layers_;
	float half_face_length = face_length_ / 2;

	// in order to format codes, use short temp variables here.
	float length = cube_length;
	float gap = gap_between_layers_;

	// Calculate the coordinates of the 8 corner points on Rubik Cube, we use them to mark the Face coordinates later.
	XMVECTOR A = XMVectorSet(-half_face_length,  half_face_length, -half_face_length, 0); // The front-top-left corner
	XMVECTOR B = XMVectorSet( half_face_length,  half_face_length, -half_face_length, 0);
	XMVECTOR C = XMVectorSet( half_face_length, -half_face_length, -half_face_length, 0);
	XMVECTOR D = XMVectorSet(-half_face_length, -half_face_length, -half_face_length, 0);
	XMVECTOR E = XMVectorSet(-half_face_length,  half_face_length,  half_face_length, 0); // The back-top-left corner
	XMVECTOR F = XMVectorSet( half_face_length,  half_face_length,  half_face_length, 0);
	XMVECTOR G = XMVectorSet( half_face_length, -half_face_length,  half_face_length, 0);
	XMVECTOR H = XMVectorSet(-half_face_length, -half_face_length,  half_face_length, 0);

	// Initialize the 6 faces of Rubik Cube, faces used later in Ray-Cube hit test.
	Rect  FrontFace(A, B, C, D) ; 
	Rect   BackFace(E, F, G, H) ;
	Rect   LeftFace(E, A, D, H) ;
	Rect  RightFace(B, F, G, C) ;
	Rect    TopFace(E, F, B, A) ;
	Rect BottomFace(G, H, D, C) ;

	faces[0] = FrontFace;
	faces[1] = BackFace;
	faces[2] = LeftFace;
	faces[3] = RightFace;
	faces[4] = TopFace;
	faces[5] = BottomFace;
}

RubikCube::~RubikCube(void)
{
	// Delete world arcball
	delete world_arcball_;
	world_arcball_ = NULL;

	// Delete camera
	delete camera_;
	camera_ = NULL;

	// Delete cubes
	delete []cubes;
	cubes = NULL;

	// Delete faces
	delete []faces;
	faces = NULL;

	if (input_layout_ != NULL)
	{
		input_layout_->Release();
		input_layout_ = NULL;
	}

	if (vertex_shader_ != NULL)
	{
		vertex_shader_->Release();
		vertex_shader_ = NULL;
	}

	if (pixel_shader_ != NULL)
	{
		pixel_shader_->Release();
		pixel_shader_ = NULL;
	}

	if (immediate_context_ != NULL)
	{
		immediate_context_->Release();
		immediate_context_ = NULL;
	}

	if(d3ddevice_ != NULL)
	{
		d3ddevice_->Release();
		d3ddevice_ = NULL;
	}
}

void RubikCube::Initialize(HWND hWnd)
{
	hWnd_ = hWnd;

	InitD3D11(hWnd);

	InitCubes();

	InitVertexShader();

	InitPixelShader();

	ResetTextures();

	ResetLayerIds();
}

/*
The layer id was count from X-axis first, from left to right, 0, 1, 2, ...
Then from Y-axis, kNumLayers, kNumLayers + 1, ...
Then from Z-axis, 2 * kNumLayers, 2 * kNumLayers + 1, ....
*/
void RubikCube::ResetLayerIds()
{
	float length = cubes[0].GetLength();
	float gap    = gap_between_layers_;
	float half_face_length = face_length_ / 2;

	for (int i = 0; i < kNumCubes; ++i)
	{
		float center_x = XMVectorGetX(cubes[i].GetCenter()) + half_face_length;
		float center_y = XMVectorGetY(cubes[i].GetCenter()) + half_face_length;
		float center_z = XMVectorGetZ(cubes[i].GetCenter()) + half_face_length;

		for (int j = 0; j < kNumLayers; ++j)
		{
			if (center_x >= j * (length + gap) 
				&& center_x <= (j + 1) * (length + gap) - gap)
			{
				cubes[i].SetLayerIdX(j);
			}

			if (center_y >= j * (length + gap)
				&& center_y <= (j + 1) * (length + gap) - gap)
			{
				cubes[i].SetLayerIdY(j + kNumLayers);
			}

			if (center_z >= j * (length + gap)
				&& center_z <= (j + 1) * (length + gap) - gap)
			{
				cubes[i].SetLayerIdZ(j + 2 * kNumLayers);
			}
		}
	}
}

void RubikCube::ResetTextures()
{
	float half_face_length = face_length_ / 2;
	float float_epsilon = 0.0001f; 

	// Set texture for each face of Rubik Cube
	for (int i = 0; i < kNumCubes; ++i)
	{ 
		//Front face
		if (fabs(XMVectorGetZ(cubes[i].GetMinPoint()) + half_face_length) < float_epsilon)
		{
			cubes[i].SetTextureId(0, 0);
		}

		// Back face
		if (fabs(XMVectorGetZ(cubes[i].GetMaxPoint()) - half_face_length) < float_epsilon)
		{
			cubes[i].SetTextureId(1, 1);
		}

		// Left face
		if (fabs(XMVectorGetX(cubes[i].GetMinPoint()) + half_face_length) < float_epsilon)
		{
			cubes[i].SetTextureId(2, 2);
		}

		// Right face
		if (fabs(XMVectorGetX(cubes[i].GetMaxPoint()) - half_face_length) < float_epsilon)
		{
			cubes[i].SetTextureId(3, 3);
		}

		// Top face
		if (fabs(XMVectorGetY(cubes[i].GetMaxPoint()) - half_face_length) < float_epsilon)
		{
			cubes[i].SetTextureId(4, 4);
		}

		// Bottom face
		if (fabs(XMVectorGetY(cubes[i].GetMinPoint()) + half_face_length) < float_epsilon)
		{
			cubes[i].SetTextureId(5, 5);
		}
	}
}

D3DXVECTOR2 RubikCube::GetMaxScreenResolution()
{
	// Return value
	int  max_screen_width = 0;
	int max_screen_height = 0;

	// Create DXGI factory
	IDXGIFactory* pFactory = NULL;
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Create DXGI factory failed", L"Error", 0);
	}

	// Enumerate all adapters in current system
	IDXGIAdapter* pAdapter = NULL;
	for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		// Enumerate all output in current adapters
		IDXGIOutput* pOutput = NULL;
		for (UINT i = 0; pAdapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			// Fetch Output infomation
			DXGI_OUTPUT_DESC outputDesc;
			pOutput->GetDesc(&outputDesc);

			// Calculate desktop resolution for current output
			int current_screen_width  = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
			int current_screen_height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;

			// Get the maximum screen resolution
			if (max_screen_width < current_screen_width)
			{
				output_ = pOutput;
				max_screen_width  = current_screen_width;

				// Screen height was updated based on screen width, we don't compare screen width to get the 
				// maximum value here, this will cause error sometimes, take a look at the following two resolutions
				// 1600 * 900 and 1360 * 1024, 1600 > 1360, but 900 < 1024.
				max_screen_height = current_screen_height;
			}
		}
	}

	// When pFactory->EnumAdapters return DXGI_NOT_FOUND, pAdapter was set to NULL
	// So only call Release when pAdapter was not NULL.
	if (pAdapter != NULL)
	{
		pAdapter->Release();
		pAdapter = NULL;
	}

	pFactory->Release();
	pFactory = NULL;

	return D3DXVECTOR2((float)max_screen_width, (float)max_screen_height);
}

void RubikCube::Render()
{
	// Window was inactive(minimized or hidden by other apps), yields 25ms to other program
	if(!window_active_)
	{
		Sleep(25) ;
	}

	// Update frame
	camera_->OnFrameMove();

	// Get view and projection matrix;
	XMMATRIX view_matrix = camera_->GetViewMatrix();
	XMMATRIX proj_matrix = camera_->GetProjMatrix();

	// Clear the back-buffer to a BLUE color
	float color[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	immediate_context_->ClearRenderTargetView(rendertarget_view_, color);

	// Clear the depth-stencil buffer
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 120);

	// Set vertex laytou
	immediate_context_->IASetInputLayout(input_layout_);

	// Set vertex shader
	immediate_context_->VSSetShader(vertex_shader_, NULL, 0);

	// Set pixel shader
	immediate_context_->PSSetShader(pixel_shader_, NULL, 0);

	// Store old world matrix
	XMMATRIX world_matrix = camera_->GetWorldMatrix() ;

	//draw all unit cubes to build the Rubik cube
	for(int i = 0; i < kNumCubes - 25; i++)
	{
		cubes[i].Draw(immediate_context_, view_matrix, proj_matrix);
	}

	// Restore world matrix since the Draw function in class Cube has set the world matrix for each cube
	camera_->SetWorldMatrix(world_matrix);

	// Present the sence from back buffer to front buffer
	swap_chain_->Present(0, 0);
}

void RubikCube::Shuffle()
{
	// If another rotatioin was in progress, return.
	// This prevent the Rubik Cube from being distort when user drag the left button while pressing the S key.
	if(!rotate_finish_)
		return ;

	// Block other rotations 
	rotate_finish_ = false ;

	// Set the random seed
	srand((unsigned int)time(0));

	// Calculate total layers, a n x n x n Rubik Cube has 3 x n layers totally.
	int total_layers = kNumLayers * 3;

	// Rotate 20 times
	for (int i = 0; i < 20; ++i)
	{
		// Generate a random layer
		int layer_id = rand() % total_layers;

		// Generate rotate axis based on rotate layer
		XMVECTOR axis;

		if (layer_id >= 0 && layer_id <= kNumLayers - 1)
			axis = XMVectorSet(1, 0, 0, 0);
		else if (layer_id >= kNumLayers && layer_id <= 2 * kNumLayers - 1)
			axis = XMVectorSet(0, 1, 0, 0);
		else // layer_id >= 2 * kNumLayers && layer_id <= 3 * kNumLayers - 1
			axis = XMVectorSet(0, 0, 1, 0);

		RotateLayer(layer_id, axis, (float)D3DX_PI / 2);

		for (int i = 0; i < kNumCubes; ++i)
		{
			if (cubes[i].InLayer(layer_id))
			{
				cubes[i].UpdateMinMaxPoints(axis, 1);
				cubes[i].UpdateCenter();
			}
		}

		ResetLayerIds();
	}

	// Release other rotations
	rotate_finish_ = true ;
}

// Restore Rubik Cube,make it in complete state
void RubikCube::Restore()
{
	InitCubes();
	ResetLayerIds();
}

// Switch between window mode and full-screen mode
void RubikCube::ToggleFullScreen()
{
	// Window -> Full-screen
	if (is_fullscreen_ == false)
	{
		is_fullscreen_ = true;
		ResizeD3DScene(screen_width_, screen_height_);
		//swap_chain_->SetFullscreenState(true, NULL);
	}
	
	// Full-screen -> Window
	else 
	{
		is_fullscreen_ = false;
		ResizeD3DScene(screen_width_, screen_height_);
		swap_chain_->SetFullscreenState(false, NULL);
	}
}

void RubikCube::OnLeftButtonDown(int x, int y)
{
	if(!rotate_finish_) // another rotate is in process, return directly
		return ;
	rotate_finish_ = false ; // Prevent the other rotation during this rotate

	// Clear total angle
	total_rotate_angle_ = 0;

	Ray ray = CalculatePickingRay(x, y) ;

	// Get Projection matrix
	XMMATRIX proj = camera_->GetProjMatrix();

	previous_vector_ = ScreenToVector3(x, y);

	XMVECTOR currentHitPoint;	// hit point on the face
	float maxDist = 100000.0f ;

	// Select the face nearest to the camera
	for(int i = 0; i < kNumFaces; i++)
	{
		if(RayRectIntersection(ray, faces[i], currentHitPoint))
		{
			is_hit_ = true ;

			// distance from the origin of the ray and to the hit point
			float distance = SquareDistance(ray.origin, currentHitPoint);

			if(distance < maxDist)
			{
				maxDist = distance ;
				previous_hitpoint_ = currentHitPoint ;
			}
		}
	}

	// no action if the picking ray is not intersection with cube 
	if(!is_hit_)
		return ;

	// if the ray intersect with either of the two triangles, then it intersect with the rectangle
	world_arcball_->OnBegin(x, y) ;
}

void RubikCube::OnMouseMove(int x, int y)
{
	if (!is_hit_)
		return;

	world_arcball_->OnMove(x, y) ;

	current_vector_ = ScreenToVector3(x, y);

	// Get the picked face
	Face face = GetPickedFace(previous_hitpoint_);

	Ray picking_ray = CalculatePickingRay(x, y);
	RayRectIntersection(picking_ray, faces[face], current_hitpoint_);

	D3DXPLANE plane;
	
	int layer = -1;

	if (!is_cubes_selected_)
	{
		is_cubes_selected_ = true;

		// Calculate picking plane.
		plane = GeneratePlane(face, previous_hitpoint_, current_hitpoint_);
	
		rotate_axis_ = GetRotateAxis(face, previous_hitpoint_, current_hitpoint_);
		hit_layer_ = GetHitLayer(face, rotate_axis_, previous_hitpoint_);
	}

	float angle = CalculateRotateAngle();

	rotate_direction_ = GetRotateDirection(face, rotate_axis_, previous_vector_, current_vector_);

	// Flip the angle if the direction is counter-clockwise
	// the positive direction is clockwise around the rotate axis when look through the axis toward the origin.
	if (rotate_direction_ == kCounterClockWise)
		angle = -angle;
	total_rotate_angle_ += angle;

	// Rotate
	RotateLayer(hit_layer_, rotate_axis_, angle);

	// Update previous_hitpoint_
	previous_vector_ = current_vector_;
}

// When Left button up, complete the rotation of the left angle to align the cube and update the layer info
void RubikCube::OnLeftButtonUp()
{
	is_hit_ = false ;

	world_arcball_->OnEnd();

	float left_angle = 0.0f ;	// the angle need to rotate when mouse is up
	int   num_half_PI = 0;

	if (total_rotate_angle_ > 0)
	{
		while (total_rotate_angle_ >= (float)D3DX_PI)
		{
			total_rotate_angle_ -= (float)D3DX_PI / 2;
			++num_half_PI;
		}

		if ((total_rotate_angle_ >= 0) && (total_rotate_angle_ <= (float)D3DX_PI / 4))
		{
			left_angle = -total_rotate_angle_;
		}

		else // ((total_rotate_angle_ > D3DX_PI / 4) && (total_rotate_angle_ < D3DX_PI / 2))
		{
			++num_half_PI;
			left_angle = (float)D3DX_PI / 2 - total_rotate_angle_;
		}

	}
	else // total_rotate_angle_ < 0
	{
		while (total_rotate_angle_ <= -(float)D3DX_PI / 2)
		{
			total_rotate_angle_ += (float)D3DX_PI / 2;
			--num_half_PI;
		}

		if ((total_rotate_angle_ >= -(float)D3DX_PI / 4) && (total_rotate_angle_ <= 0))
		{
			left_angle = -total_rotate_angle_;
		}

		else // ((total_rotate_angle_ > -D3DX_PI / 2) && (total_rotate_angle_ < -D3DX_PI / 4))
		{
			--num_half_PI;
			left_angle = -(float)D3DX_PI / 2 - total_rotate_angle_;
		}
	}

	RotateLayer(hit_layer_, rotate_axis_, left_angle);

	// Make num_rotate_half_PI > 0, since we will mode 4 later
	// so add it 4 each time, -1 = 3, -2 = 2, -3 = 1
	// because - (pi / 2) = 3 * pi /2, -pi / 2 = pi / 2, - 3 * pi / 2 = pi / 2
	while (num_half_PI < 0)
		num_half_PI += 4;

	num_half_PI %= 4;

	for (int i = 0; i < kNumCubes; ++i)
	{
		if (cubes[i].InLayer(hit_layer_))
		{
			cubes[i].UpdateMinMaxPoints(rotate_axis_, num_half_PI);
			cubes[i].UpdateCenter();
		}
	}

	ResetLayerIds();

	// When mouse up, one rotation was finished, no cube was selected
	is_cubes_selected_ = false;

	// Enable next rotation.
	rotate_finish_ = true ;
}

LRESULT RubikCube::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)    
	{
	case WM_CREATE:
		break;

	case WM_ACTIVATE:
		if(LOWORD(wParam) == WA_INACTIVE)
			window_active_ = false ;
		break ;

	case WM_LBUTTONDOWN:
		{	
			// Set current window to capture mouse event, so even if the mouse was release outside the window
			// the message still can be correctly processed.
			SetCapture(hWnd) ;
			int iMouseX = ( short )LOWORD( lParam );
			int iMouseY = ( short )HIWORD( lParam );
			OnLeftButtonDown(iMouseX, iMouseY);
		}
		break ;

	case WM_LBUTTONUP:
		{
			OnLeftButtonUp();
			ReleaseCapture();
		}
		break ;

	case WM_MOUSEMOVE:
		{
			int iMouseX = ( short )LOWORD( lParam );
			int iMouseY = ( short )HIWORD( lParam );
			OnMouseMove(iMouseX, iMouseY) ;
		}
		break ;

	case WM_PAINT: 
		Render();
		break ;

	case WM_KEYDOWN:
		{
			switch( wParam )
			{
			case 'R':
				Restore();
				break;
			case 'S':
				Shuffle();
				break;
			case 'F':
				ToggleFullScreen() ;
				break;
			case VK_ESCAPE:
				SendMessage(hWnd, WM_CLOSE, 0, 0);
				break ;
			default:
				break ;
			}
		}
		break ;

	case WM_SIZE: // why not use WM_EXITSIZEMOVE?
		{
			if (!swap_chain_)
				return 0;

			// inactive the app when window is minimized
			if(wParam == SIZE_MINIMIZED)
				window_active_ = false ;

			else if (wParam == SIZE_MAXIMIZED)
			{
				// Get current window size
				current_window_width_ = ( short )LOWORD( lParam );
				current_window_height_ = ( short )HIWORD( lParam );

				if(current_window_width_ != last_window_width_ || current_window_height_ != last_window_height_)
				{
					// Update back buffer to desktop resolution
					sd_.BufferDesc.Width = current_window_width_;
					sd_.BufferDesc.Height = current_window_height_;
					ResizeD3DScene(current_window_width_, current_window_height_);

					last_window_width_ = current_window_width_ ;
					last_window_height_ = current_window_height_ ;
				}
			}

			else if (wParam == SIZE_RESTORED)
			{
				window_active_ = true ;

				// Maximized -> Full Screen
				if (is_fullscreen_ == true)
				{
					// Update back buffer to desktop resolution
					sd_.BufferDesc.Width = screen_width_;
					sd_.BufferDesc.Height = screen_height_;

					// resize scene
					ResizeD3DScene(screen_width_, screen_width_);
				}
				else
				{
					// Get current window size
					current_window_width_ = ( short )LOWORD( lParam );
					current_window_height_ = ( short )HIWORD( lParam );

					// Reset device
					sd_.BufferDesc.Width = current_window_width_;
					sd_.BufferDesc.Height = current_window_height_;
					ResizeD3DScene(current_window_width_, current_window_height_);

					last_window_width_ = current_window_width_ ;
					last_window_height_ = current_window_height_ ;
				}
			}
		}
		break ;

	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage (0) ;
		return 0 ;    
	}

	return camera_->HandleMessages(hWnd, uMsg, wParam, lParam);
}

void RubikCube::InitD3D11(HWND hWnd)
{
	this->hWnd = hWnd;

	RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

	// Setup a DXGI swap chain descriptor
	ZeroMemory( &sd_, sizeof( sd_ ) );

	if(is_fullscreen_)
	{
		sd_.Windowed = FALSE;
		sd_.BufferDesc.Width  = 1980; // here need update
		sd_.BufferDesc.Height = 1280;	// here need update
	}
	else
	{
		sd_.Windowed = TRUE;

		// Get current window size and set it as back buffer size
		RECT rect;
		GetClientRect(hWnd, &rect);
		sd_.BufferDesc.Width  = rect.right - rect.left;
		sd_.BufferDesc.Height = rect.bottom - rect.top;
	}

	sd_.BufferCount = 1; // number of buffer
	sd_.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // buffer format, 32 bit color with alpha(RGBA)
	sd_.BufferDesc.RefreshRate.Numerator = 60; // refresh rate?
	sd_.BufferDesc.RefreshRate.Denominator = 1; // WHAT'S THIS?
	sd_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // use buffer as render target
	sd_.OutputWindow = hWnd; // output window handle
	sd_.SampleDesc.Count = 1; // WHAT'S THIS?
	sd_.SampleDesc.Quality = 0; // WHAT'S THIS?
	sd_.Windowed = TRUE; // full-screen mode

	// Create device and swap chain
	HRESULT hr;
	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif 

	// Create device and swap chain
	D3D_FEATURE_LEVEL FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0; // Use d3d11
	UINT              numLevelsRequested = 1; // Number of levels 
	D3D_FEATURE_LEVEL FeatureLevelsSupported;

	if (FAILED (hr = D3D11CreateDeviceAndSwapChain( NULL, 
	    D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&FeatureLevelsRequested,
		numLevelsRequested,
		D3D11_SDK_VERSION,
		&sd_, 
		&swap_chain_,
		&d3ddevice_,
		&FeatureLevelsSupported,
		&immediate_context_)))
	{
		MessageBox(hWnd, L"Create device and swap chain failed!", L"Error", 0);
	}


	// Create depth-stencil resource
	D3D11_TEXTURE2D_DESC tex_desc;
	ZeroMemory(&tex_desc, sizeof(tex_desc));
	tex_desc.Width = sd_.BufferDesc.Width;
	tex_desc.Height = sd_.BufferDesc.Height;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tex_desc.SampleDesc = sd_.SampleDesc;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	ID3D11Texture2D* depth_stencil = NULL;
	hr = d3ddevice_->CreateTexture2D(&tex_desc, NULL, &depth_stencil);
	if (FAILED(hr))
	{
		MessageBox(hWnd, L"Create depth and stencil buffer failed!", L"Error", 0);
	}

	// Create depth-stencil state
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	ID3D11DepthStencilState* pDSState = NULL;
	hr = d3ddevice_->CreateDepthStencilState(&dsDesc, &pDSState);
	if (FAILED(hr))
	{
		MessageBox(hWnd, L"Create depth and stencil state failed!", L"Error", 0);
	}

	// Bind depth-stencil state.
	immediate_context_->OMSetDepthStencilState(pDSState, 1);

	// Bind the depth_stencil resource using a view.
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	hr = d3ddevice_->CreateDepthStencilView(
		depth_stencil,
		&descDSV,
		&depth_stencil_view_);
	if (FAILED(hr))
	{
		MessageBox(hWnd, L"Create depth and stencil view failed!", L"Error", 0);
	}

	// Create render target and bind the back-buffer
	ID3D11Texture2D* pBackBuffer;

	// Get a pointer to the back buffer
	hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID* )&pBackBuffer);
	if (FAILED(hr))
	{
		MessageBox(hWnd, L"Get buffer failed!", L"Error", 0);
	}

	// Create a render-target view
	hr = d3ddevice_->CreateRenderTargetView(pBackBuffer, NULL, &rendertarget_view_);
	if (FAILED(hr))
	{
		MessageBox(hWnd, L"Create render target view failed!", L"Error", 0);
	}

	// Release temporary back buffer
	pBackBuffer->Release();

	// Bind the view
	immediate_context_->OMSetRenderTargets(1, &rendertarget_view_, depth_stencil_view_); 

	// Setup the viewport
	vp_.Width    = (float)width; // this should be similar with the back-buffer width, global it!
	vp_.Height   = (float)height;
	vp_.MinDepth = 0.0f;
	vp_.MaxDepth = 1.0f;
	vp_.TopLeftX = 0;
	vp_.TopLeftY = 0;
	immediate_context_->RSSetViewports(1, &vp_);

	// Get maximum screen resolution
	D3DXVECTOR2 max_screen_resolution = GetMaxScreenResolution();
	screen_width_  = (int)max_screen_resolution.x;
	screen_height_ = (int)max_screen_resolution.y;

	// Setup view matrix
	XMVECTOR vecEye = XMVectorSet(0.0f, 0.0f, -20.0f, 0.0f);
	XMVECTOR vecAt  = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR vecUp  = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) ;
	camera_->SetViewParams(vecEye, vecAt, vecUp);
	
	// Setup projection matrix
	float aspectRatio = (float)sd_.BufferDesc.Width / (float)sd_.BufferDesc.Height ;
	camera_->SetProjParams((float)D3DX_PI / 4, aspectRatio, 1.0f, 1000.0f) ;
}

void RubikCube::ResizeD3DScene(int width, int height)
{
	if (height == 0)				// Prevent A Divide By Zero By
		height = 1;					// Making Height Equal One

	// Compute aspect ratio
	float fAspectRatio = width / (FLOAT)height;

	// Setup Projection matrix
	camera_->SetProjParams((float)D3DX_PI / 4, fAspectRatio, 1.0f, 1000.0f);

	camera_->SetWindow(width, height);

	// Release all references to swap chain
	rendertarget_view_->Release();

	// Resize back buffer
	swap_chain_->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	// Create render target and bind the back-buffer
	ID3D11Texture2D* pBackBuffer;

	// Get a pointer to the back buffer
	HRESULT hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID* )&pBackBuffer);
	if (FAILED(hr))
	{
		MessageBox(hWnd, L"Get buffer failed!", L"Error", 0);
	}

	// Create a render-target view
	d3ddevice_->CreateRenderTargetView(pBackBuffer, NULL, &rendertarget_view_);

	// Release temporary back buffer
	pBackBuffer->Release();

	// Bind the view
	immediate_context_->OMSetRenderTargets(1, &rendertarget_view_, NULL); 

	// Setup the viewport
	vp_.Width = (float)width; // this should be similar with the back-buffer width, global it!
	vp_.Height = (float)height;
	vp_.MinDepth = 0.0f;
	vp_.MaxDepth = 1.0f;
	vp_.TopLeftX = 0;
	vp_.TopLeftY = 0;
	immediate_context_->RSSetViewports(1, &vp_);
}

void RubikCube::InitCubes()
{
	// Set device for unit cubes
	for (int i = 0; i < kNumCubes; ++i)
	{
		cubes[i].SetDevice(d3ddevice_);
	}

	// Get unit cube length and gaps between layers
	float cube_length = cubes[0].GetLength();
	float gap = gap_between_layers_;

	// Calculate half face length
	float half_face_length = face_length_ / 2;

	/* Initialize the top-front-left corner of each unit cubes, we use (kNumLayers)^3 unit cubes
	   to build up a Rubik Cube

	   The Cube was labeled by the following rule, suppose a 3 x 3 x 3 Rubik Cube
	   front layer		middle layer      back layer
	   6   7   8		15  16  17		  24  25  26
	   3   4   5 		12  13  14		  21  22  23
	   0   1   2		 9  10  11		  18  19  20
	*/
	for (int i = 0; i < kNumLayers; ++i)
	{
		for (int j = 0; j < kNumLayers; ++j)
		{
			for (int k = 0; k < kNumLayers; ++k)
			{
				// calculate the front-bottom-left corner coodinates for current cube
				// The Rubik Cube's center was the coordinate center, but the calculation assume the front-bottom-left corner
				// of the Rubik Cube was in the coodinates center, so move half_face_length for each coordinates component.
				float x = i * (cube_length + gap) - half_face_length;
				float y = j * (cube_length + gap) - half_face_length;
				float z = k * (cube_length + gap) - half_face_length;

				// calculate the unit cube index in inti_pos
				int n = i + (j * kNumLayers) + (k * kNumLayers * kNumLayers);

				// Initiliaze cube n
				cubes[n].Init(XMVectorSet(x, y, z, 0));
			}
		}
	}

	// Reset world matrix to Identity matrix for each unit cube
	XMMATRIX world_matrix = XMMatrixIdentity();

	for (int i = 0; i < kNumCubes; ++i)
	{
		cubes[i].SetWorldMatrix(world_matrix);
	}
}

void RubikCube::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	ID3DBlob*	pErrorBlob;

#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3D11_SHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

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

void RubikCube::InitVertexShader()
{
	// Compile the vertex shader from file
	ID3DBlob*	pVSBlob = NULL;
	CompileShaderFromFile(L"rubik_cube.fx", "VS", "vs_4_0", &pVSBlob);

	// Create vertex shader
	HRESULT hr = d3ddevice_->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &vertex_shader_);
	if(FAILED(hr))
	{
		pVSBlob->Release();
		MessageBox(NULL, L"Create vertex shader failed", L"Error", 0);
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	UINT numElements = ARRAYSIZE(layout);
	hr = d3ddevice_->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), 
													pVSBlob->GetBufferSize(), &input_layout_);
	pVSBlob->Release();

}

void RubikCube::InitPixelShader() 
{
	// Compile the pixel shader
	ID3DBlob*	pPSBlob = NULL;
	CompileShaderFromFile(L"rubik_cube.fx", "PS", "ps_4_0", &pPSBlob);

	// Create the pixel shader
	ID3D11InputLayout*		g_pVertexLayout		= NULL;
	HRESULT hr = d3ddevice_->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pixel_shader_);
	pPSBlob->Release();
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create pixel shader failed", L"Error", 0);
	}
}

int RubikCube::GetWindowPosX() const
{
	return init_window_x_;
}

int RubikCube::GetWindowPosY() const
{
	return init_window_y_;
}

int RubikCube::GetWindowWidth() const
{
	return init_window_width_;
}

int RubikCube::GetWindowHeight() const
{
	return init_window_height_;
}

XMVECTOR RubikCube::ScreenToVector3(int x, int y)
{
	// Get Projection matrix
	XMMATRIX proj = camera_->GetProjMatrix();

	XMVECTOR first_row = proj.r[0];
	float proj00 = XMVectorGetX(first_row);

	XMVECTOR second_row = proj.r[1];
	float proj11 = XMVectorGetY(second_row);
	
	float vector_x  = ((( 2.0f*x) / vp_.Width)  - 1.0f) / proj00;
	float vector_y = (((-2.0f*y) / vp_.Height) + 1.0f) / proj11;
	float vector_z = 1.0f ;

	XMVECTOR vector3 = XMVectorSet(vector_x, vector_y, vector_z, 0);

	// Get world matrix
	XMMATRIX world = camera_->GetWorldMatrix();

	// Get view matrix
	XMMATRIX view = camera_->GetViewMatrix();

	// Concatinate them in to single matrix
	XMMATRIX WorldView = world * view;

	// inverse it
	XMMATRIX worldviewInverse = XMMatrixInverse(0, WorldView);
	vector3 = XMVector2TransformCoord(vector3, worldviewInverse);
	vector3 = XMVector3Normalize(vector3);

	return vector3 ;
}

// Calculate the picking ray and transform it to model space 
// x and y are the screen coordinates when left button down
Ray RubikCube::CalculatePickingRay(int x, int y)
{
	float px = 0.0f;
	float py = 0.0f;

	// Get Projection matrix
	XMMATRIX proj = camera_->GetProjMatrix();

	XMVECTOR first_row = proj.r[0];
	float proj00 = XMVectorGetX(first_row);

	XMVECTOR second_row = proj.r[1];
	float proj11 = XMVectorGetY(second_row);

	px = ((( 2.0f*x) / vp_.Width)  - 1.0f) / proj00;
	py = (((-2.0f*y) / vp_.Height) + 1.0f) / proj11;

	Ray ray;
	ray.origin    = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	ray.direction = XMVectorSet(px, py, 1.0f, 0.0f); 

	// Get view matrix
	XMMATRIX view = camera_->GetViewMatrix();

	// Get world matrix
	XMMATRIX world = camera_->GetWorldMatrix();
	// d3ddevice_->GetTransform(D3DTS_WORLD, &world);

	// Concatinate them in to single matrix
	XMMATRIX WorldView = world * view;

	// inverse it
	XMMATRIX worldviewInverse = XMMatrixInverse(0, WorldView);

	// Transform the ray to model space
	ray.origin = XMVector2TransformCoord(ray.origin, worldviewInverse) ;
	ray.direction = XMVector2TransformCoord(ray.direction, worldviewInverse) ;

	// Normalize the direction
	ray.direction = XMVector3Normalize(ray.direction) ;

	return ray;
}

/*
We get the picked face by check the hit_point's coordinates(x, y, z), suppose the length of the small cube
is cube_length, and the gap length between two layers is gaps, and the Rubik cube's center is coordiates origin.
then, if the left face was picked, then, x == -(1.5 * cube_length + gaps), if the top face was picked, then 
y == 1.5 * length + gaps
*/
Face RubikCube::GetPickedFace(XMVECTOR hit_point) const
{
	float float_epsilon = 0.001f;
	float cube_length = cubes[0].GetLength();
	float face_length = kNumLayers * cube_length + (kNumLayers - 1) * gap_between_layers_;
	float half_face_length = face_length / 2;

	if (fabs(XMVectorGetZ(hit_point) + half_face_length) < float_epsilon) { return kFrontFace;  }
	if (fabs(XMVectorGetZ(hit_point) - half_face_length) < float_epsilon) { return kBackFace;   }
	if (fabs(XMVectorGetX(hit_point) + half_face_length) < float_epsilon) { return kLeftFace;   }
	if (fabs(XMVectorGetX(hit_point) - half_face_length) < float_epsilon) { return kRightFace;  }
	if (fabs(XMVectorGetY(hit_point) - half_face_length) < float_epsilon) { return kTopFace;    }
	if (fabs(XMVectorGetY(hit_point) + half_face_length) < float_epsilon) { return kBottomFace; }

	return kUnknownFace;
}

D3DXPLANE RubikCube::GeneratePlane(Face face, XMVECTOR& previous_point, XMVECTOR& current_point)
{
	float abs_diff_x = fabs(XMVectorGetX(previous_point) - XMVectorGetX(current_point));
	float abs_diff_y = fabs(XMVectorGetY(previous_point) - XMVectorGetY(current_point));
	float abs_diff_z = fabs(XMVectorGetZ(previous_point) - XMVectorGetZ(current_point));

	switch (face)
	{
	case kFrontFace:
	case kBackFace:
		if (abs_diff_x < abs_diff_y)
			return D3DXPLANE(1, 0, 0, -XMVectorGetX(previous_point));
		else
			return D3DXPLANE(0, 1, 0, -XMVectorGetY(previous_point));
		break;

	case kLeftFace:
	case kRightFace:
		if (abs_diff_y < abs_diff_z)
			return D3DXPLANE(0, 1, 0, -XMVectorGetY(previous_point));
		else
			return D3DXPLANE(0, 0, 1, -XMVectorGetZ(previous_point));
		break;

	case kTopFace:
	case kBottomFace:
		if (abs_diff_x < abs_diff_z)
			return D3DXPLANE(1, 0, 0, -XMVectorGetX(previous_point));
		else
			return D3DXPLANE(0, 0, 1, -XMVectorGetZ(previous_point));
		break;

	default:
		return D3DXPLANE(0, 0, 0, 0);
	}
}

XMVECTOR RubikCube::GetRotateAxis(Face face, XMVECTOR& previous_point, XMVECTOR& current_point)
{
	float abs_diff_x = fabs(XMVectorGetX(previous_point) - XMVectorGetX(current_point));
	float abs_diff_y = fabs(XMVectorGetY(previous_point) - XMVectorGetY(current_point));
	float abs_diff_z = fabs(XMVectorGetZ(previous_point) - XMVectorGetZ(current_point));

	switch (face)
	{
	case kFrontFace:
	case kBackFace:
		if (abs_diff_x < abs_diff_y)
			return XMVectorSet(1, 0, 0, 0);
		else
			return XMVectorSet(0, 1, 0, 0);
		break;

	case kLeftFace:
	case kRightFace:
		if (abs_diff_y < abs_diff_z)
			return XMVectorSet(0, 1, 0, 0);
		else
			return XMVectorSet(0, 0, 1, 0);
		break;

	case kTopFace:
	case kBottomFace:
		if (abs_diff_x < abs_diff_z)
			return XMVectorSet(1, 0, 0, 0);
		else
			return XMVectorSet(0, 0, 1, 0);
		break;

	default:
		return XMVectorSet(0, 0, 0, 0);
	}
}

RotateDirection RubikCube::GetRotateDirection(Face face, XMVECTOR& axis, XMVECTOR& previous_vector, XMVECTOR& current_vector)
{
	float delta_x = XMVectorGetX(previous_vector) - XMVectorGetX(current_vector);
	float delta_y = XMVectorGetY(previous_vector) - XMVectorGetY(current_vector);
	float delta_z = XMVectorGetZ(previous_vector) - XMVectorGetZ(current_vector);

	// Rotate around x-axis
	if (XMVectorGetX(axis) != 0)
	{
		switch (face)
		{
			case kFrontFace:
				if (delta_y > 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			case kBackFace:
				if (delta_y < 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			case kTopFace:
				if (delta_z > 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			case kBottomFace:
				if (delta_z < 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;
			default:
				return kUnknownDirection;
		}
	}

	// Rotate around y-axis
	else if (XMVectorGetY(axis) != 0)
	{
		switch (face)
		{
			case kFrontFace:
				if (delta_x < 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			case kBackFace:
				if (delta_x > 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			case kLeftFace:
				if (delta_z > 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			case kRightFace:
				if (delta_z < 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			default:
				return kUnknownDirection;
		}
	}

	// Rotate around z-axis
	else // axis.z != 0
	{
		switch (face)
		{
			case kLeftFace:
				if (delta_y < 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			case kRightFace:
				if (delta_y > 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			case kTopFace:
				if (delta_x < 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			case kBottomFace:
				if (delta_x > 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			default:
				return kUnknownDirection;
		}
	}
}

float RubikCube::CalculateRotateAngle()
{
	// Get the rotation increment
	XMVECTOR quat = world_arcball_->GetRotationQuatIncreament();

	//extract rotation angle from quaternion
	float angle = 2.0f * acosf(XMVectorGetW(quat)) * rotate_speed_ ;

	return angle;
}

int  RubikCube::GetHitLayer(Face face, XMVECTOR& rotate_axis, XMVECTOR& hit_point)
{
	float length = cubes[0].GetLength();
	float gap    = gap_between_layers_;

	float half_face_length = face_length_ / 2;
	float float_epsilon = 0.0001f; 

	// X-Axis 
	if (XMVectorGetX(rotate_axis) != 0)
	{
		switch (face)
		{
		case kFrontFace:
		case kBackFace:
		case kTopFace:
		case kBottomFace:
			{
				for (int i = 0; i < kNumLayers; ++i)
				{
					if (XMVectorGetX(hit_point) + half_face_length >= i * (length + gap) 
					    && XMVectorGetX(hit_point) + half_face_length <= (i + 1) * (length + gap) - gap)
						return i;
				}
			}
		break;
		}
	}

	// Y-Axis
	else if (XMVectorGetY(rotate_axis) != 0)
	{
		switch (face)
		{
		case kLeftFace:
		case kRightFace:
		case kFrontFace:
		case kBackFace:
			{
				for (int i = 0; i < kNumLayers; ++i)
				{
					if (XMVectorGetY(hit_point) + half_face_length >= i * (length + gap)
						&& XMVectorGetY(hit_point) + half_face_length <= (i + 1) * (length + gap) - gap)
						return i + kNumLayers;
				}
			}
		break;
		}
	}

	// Z-Axis
	else 
	{
		switch (face)
		{
		case kLeftFace:
		case kRightFace:
		case kTopFace:
		case kBottomFace:
			{
				for (int i = 0; i < kNumLayers; ++i)
				{
					if (XMVectorGetZ(hit_point) + half_face_length >= i * (length + gap)
						&& XMVectorGetZ(hit_point) + half_face_length <= (i + 1) * (length + gap) - gap)
						return i + 2 * kNumLayers;
				}
			}
			break;
		}
	}

	return -1;
}

void RubikCube::RotateLayer(int layer, XMVECTOR& axis, float angle)
{
	for(int i = 0; i < kNumCubes; ++i)
	{
		if (cubes[i].InLayer(layer))
		{
			cubes[i].Rotate(axis, angle);
		}
	}
}
