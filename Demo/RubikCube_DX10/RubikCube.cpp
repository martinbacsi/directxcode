#include "RubikCube.h"
#include "DXErr.h"
#include <time.h>

RubikCube::RubikCube(void)
	: d3ddevice_(NULL),
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
	  current_window_width_(init_window_width_),
	  current_window_height_(init_window_height_),
	  last_window_width_(current_window_width_),
	  last_window_height_(current_window_height_),
	  texture_width_(128),
	  texture_height_(128),
	  inner_textures_(NULL)
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
	D3DXVECTOR3 A(-half_face_length,  half_face_length, -half_face_length); // The front-top-left corner
	D3DXVECTOR3 B( half_face_length,  half_face_length, -half_face_length);
	D3DXVECTOR3 C( half_face_length, -half_face_length, -half_face_length);
	D3DXVECTOR3 D(-half_face_length, -half_face_length, -half_face_length);

	D3DXVECTOR3 E(-half_face_length,  half_face_length,  half_face_length); // The back-top-left corner
	D3DXVECTOR3 F( half_face_length,  half_face_length,  half_face_length);
	D3DXVECTOR3 G( half_face_length, -half_face_length,  half_face_length);
	D3DXVECTOR3 H(-half_face_length, -half_face_length,  half_face_length);

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

	texture_id_ = new int[kNumFaces];
	face_textures_ = new ID3D10Texture2D*[kNumFaces];

	for(int i = 0; i < kNumFaces; ++i)
	{
		texture_id_[i] = -1;
		face_textures_[i] = NULL;
	}
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

	delete []texture_id_;
	texture_id_ = NULL;

	// Release face textures
	for(int i = 0; i < kNumFaces; ++i)
	{
		if(face_textures_[i] != NULL)
		{
			face_textures_[i]->Release();
			face_textures_[i] = NULL;
		}
	}

	// Release inner texture
	if (inner_textures_ != NULL)
	{
		inner_textures_->Release();
		inner_textures_ = NULL;
	}
	
	// Release Direct3D Device
	if(d3ddevice_ != NULL)
	{
		d3ddevice_->Release();
		d3ddevice_ = NULL;
	}

	// Release Direct3D object
	if(d3d_ != NULL)
	{
		d3d_->Release();
		d3d_ = NULL;
	}
}

void RubikCube::Initialize(HWND hWnd)
{
	hWnd_ = hWnd;

	InitD3D10(hWnd);

	InitTextures();

	InitCubes();

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
		float center_x = cubes[i].GetCenter().x + half_face_length;
		float center_y = cubes[i].GetCenter().y + half_face_length;
		float center_z = cubes[i].GetCenter().z + half_face_length;

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
		if (fabs(cubes[i].GetMinPoint().z + half_face_length) < float_epsilon)
		{
			cubes[i].SetTextureId(0, 0);
		}

		// Back face
		if (fabs(cubes[i].GetMaxPoint().z - half_face_length) < float_epsilon)
		{
			cubes[i].SetTextureId(1, 1);
		}

		// Left face
		if (fabs(cubes[i].GetMinPoint().x + half_face_length) < float_epsilon)
		{
			cubes[i].SetTextureId(2, 2);
		}

		// Right face
		if (fabs(cubes[i].GetMaxPoint().x - half_face_length) < float_epsilon)
		{
			cubes[i].SetTextureId(3, 3);
		}

		// Top face
		if (fabs(cubes[i].GetMaxPoint().y - half_face_length) < float_epsilon)
		{
			cubes[i].SetTextureId(4, 4);
		}

		// Bottom face
		if (fabs(cubes[i].GetMinPoint().y + half_face_length) < float_epsilon)
		{
			cubes[i].SetTextureId(5, 5);
		}
	}
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
	D3DXMATRIX view_matrix = camera_->GetViewMatrix();
	D3DXMATRIX proj_matrix = camera_->GetProjMatrix();
	D3DXVECTOR3 eye_pos    = camera_->GetEyePoint();

	// Clear the back-buffer to a BLUE color
	float color[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	d3ddevice_->ClearRenderTargetView(rendertarget_view_, color);

	//draw all unit cubes to build the Rubik cube
	for(int i = 0; i < kNumCubes; i++)
	{
		cubes[i].Draw(view_matrix, proj_matrix, eye_pos);
	}

	// Restore world matrix since the Draw function in class Cube has set the world matrix for each cube
	D3DXMATRIX matWorld = camera_->GetWorldMatrix() ;
	d3ddevice_->SetTransform(D3DTS_WORLD, &matWorld) ;

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
		D3DXVECTOR3 axis;

		if (layer_id >= 0 && layer_id <= kNumLayers - 1)
			axis = D3DXVECTOR3(1, 0, 0);
		else if (layer_id >= kNumLayers && layer_id <= 2 * kNumLayers - 1)
			axis = D3DXVECTOR3(0, 1, 0);
		else // layer_id >= 2 * kNumLayers && layer_id <= 3 * kNumLayers - 1
			axis = D3DXVECTOR3(0, 0, 1);

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

// Switch from window mode and full-screen mode
void RubikCube::ToggleFullScreen()
{
	wp.length = sizeof(WINDOWPLACEMENT) ;

	// Window -> Full-Screen
	if(is_fullscreen_ == false)
	{
		is_fullscreen_ = true;

		// Get and save window placement
		GetWindowPlacement(hWnd_, &wp) ;

		// Update back buffer to desktop resolution
		sd_.BufferDesc.Width = current_window_width_;
		sd_.BufferDesc.Height = current_window_height_;
	}
	else // Full-Screen -> Window
	{
		is_fullscreen_ = false;

		// Update back buffer size
		sd_.BufferDesc.Width = current_window_width_;
		sd_.BufferDesc.Height = current_window_height_;


		// When swith from Full-Screen mode to window mode and the wp structe was not initialized
		// The window position and size was unavailable, this will happened when the app start as full-screen mode
		// give a defaul value of it.
		//

		// Restore window placement
		SetWindowPlacement(hWnd_, &wp) ;
	}

	// Display mode changed, we need to reset device
	ResetDevice() ;
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
	D3DXMATRIX proj = camera_->GetProjMatrix();

	previous_vector_ = ScreenToVector3(x, y);

	D3DXVECTOR3 currentHitPoint;	// hit point on the face
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

	/*case WM_CAPTURECHANGED:
		OnLeftButtonUp();
		ReleaseCapture();
		break;*/

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
					ResetDevice();

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
					sd_.BufferDesc.Width = current_window_width_;
					sd_.BufferDesc.Height = current_window_height_;

					// Reset device
					ResetDevice();
				}
				else
				{
					// Get current window size
					current_window_width_ = ( short )LOWORD( lParam );
					current_window_height_ = ( short )HIWORD( lParam );

					// Reset device
					sd_.BufferDesc.Width = current_window_width_;
					sd_.BufferDesc.Height = current_window_height_;
					ResetDevice();

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

void RubikCube::InitD3D10(HWND hWnd)
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
	UINT flags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;

#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3D10_CREATE_DEVICE_DEBUG;
#endif 

	if (FAILED (hr = D3D10CreateDeviceAndSwapChain( NULL, 
	    D3D10_DRIVER_TYPE_HARDWARE,
		NULL,
		flags,
		D3D10_SDK_VERSION,
		&sd_, 
		&swap_chain_,
		&d3ddevice_)))
	{
		MessageBox(hWnd, L"Create device and swap chain failed!", L"Error", 0);
	}

	// Create render target and bind the back-buffer
	ID3D10Texture2D* pBackBuffer;

	// Get a pointer to the back buffer
	hr = swap_chain_->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID* )&pBackBuffer);
	if (FAILED(hr))
	{
		MessageBox(hWnd, L"Get buffer failed!", L"Error", 0);
	}

	// Create a render-target view
	d3ddevice_->CreateRenderTargetView(pBackBuffer, NULL, &rendertarget_view_);

	// Bind the view
	d3ddevice_->OMSetRenderTargets(1, &rendertarget_view_, NULL); // WHAT'S OM here mean?

	// Setup the viewport
	vp_.Width = width; // this should be similar with the back-buffer width, global it!
	vp_.Height = height;
	vp_.MinDepth = 0.0f;
	vp_.MaxDepth = 1.0f;
	vp_.TopLeftX = 0;
	vp_.TopLeftY = 0;
	d3ddevice_->RSSetViewports(1, &vp);

	// Setup view matrix
	D3DXVECTOR3 vecEye(0.0f, 0.0f, -10.0f);
	D3DXVECTOR3 vecAt (0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vecUp (0.0f, 1.0f, 0.0f) ;
	camera_->SetViewParams(vecEye, vecAt, vecUp);
	
	// Setup projection matrix
	float aspectRatio = (float)sd_.BufferDesc.Width / (float)sd_.BufferDesc.Height ;
	camera_->SetProjParams((float)D3DX_PI / 4, aspectRatio, 1.0f, 1000.0f) ;
}

void RubikCube::ResizeD3D9Scene(int width, int height)
{
	if (height == 0)				// Prevent A Divide By Zero By
		height = 1;					// Making Height Equal One

	// Compute aspect ratio
	float fAspectRatio = width / (FLOAT)height;

	// Setup Projection matrix
	camera_->SetProjParams((float)D3DX_PI / 4, fAspectRatio, 1.0f, 1000.0f);

	camera_->SetWindow(width, height);
}

void RubikCube::InitTextures()
{
	DWORD colors[] = 
	{
		0xffffffff, // White,   front face
		0xffffff00, // Yellow,	back face
		0xffff0000, // Red,		left face
		0xffffa500,	// Orange,	right face
		0xff00ff00, // Green,	top face
		0xff0000ff, // Blue,	bottom face
	};

	// Create face textures
	for(int i = 0; i < kNumFaces; ++i)
	{
		face_textures_[i] = CreateTexture(texture_width_, texture_height_, colors[i]);
	}

	// Create inner texture
	inner_textures_ = CreateInnerTexture(texture_width_, texture_height_, 0xffffffff);

	Cube::SetFaceTexture(face_textures_, kNumFaces);
	Cube::SetInnerTexture(inner_textures_);
}

LPDIRECT3DTEXTURE9 RubikCube::CreateTexture(int texWidth, int texHeight, D3DCOLOR color)
{
	LPDIRECT3DTEXTURE9 pTexture;

	HRESULT hr = D3DXCreateTexture(d3ddevice_, 
		texWidth, 
		texHeight, 
		0, 
		0, 
		D3DFMT_A8R8G8B8,  // 4 bytes for a pixel 
		D3DPOOL_MANAGED, 
		&pTexture);

	if (FAILED(hr))
	{
		MessageBox(NULL, L"Create texture failed", L"Error", 0);
	}

	// Lock the texture and fill in color
	D3DLOCKED_RECT lockedRect;
	hr = pTexture->LockRect(0, &lockedRect, NULL, 0);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Lock texture failed!", L"Error", 0);
	}

	DWORD sideColor = 0xff000000; // the side line color

	int side_width = 10;

	// Calculate number of rows in the locked Rect
	int rowCount = (texWidth * texHeight * 4 ) / lockedRect.Pitch;

	for (int i = 0; i < texWidth; ++i)
	{
		for (int j = 0; j < texHeight; ++j)
		{
			int index = i * rowCount + j;

			int* pData = (int*)lockedRect.pBits;

			if (i <= side_width || i >= texWidth - side_width 
				|| j <= side_width || j >= texHeight - side_width)
			{
				memcpy(&pData[index], &sideColor, 4);
			}
			else
			{
				memcpy(&pData[index], &color, 4);
			}
		}
	}

	pTexture->UnlockRect(0);

	return pTexture;
}

ID3D10Texture2D* RubikCube::CreateInnerTexture(int texWidth, int texHeight, D3DCOLOR color)
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

	ID3D10Texture2D* pTexture = NULL;
	// Create texture
	HRESULT hr = d3ddevice_->CreateTexture2D(&texDesc, NULL, &pTexture);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Create texture in memory failed", L"Error", 0);
	}

	// Lock texture and fill in colors
	D3D10_MAPPED_TEXTURE2D mappedTex;
	ZeroMemory(&mappedTex, sizeof(mappedTex));

	pTexture->Map(0, D3D10_MAP_WRITE_DISCARD, 0, &mappedTex);

	// Fill in colors
	//============================= BE CAREFUL OF THE COLOR ORDER R8G8B8A8=====================

	DWORD sideColor = 0xff121212; // the side line color

	// Calculate number of rows in the locked Rect
	int rowCount = (texWidth * texHeight * 4 ) / lockedRect.Pitch;

	for (int i = 0; i < texWidth; ++i)
	{
		for (int j = 0; j < texHeight; ++j)
		{
			int index = i * rowCount + j;

			int* pData = (int*)lockedRect.pBits;
		
			memcpy(&pData[index], &color, 4);
		}
	}

	// Unlock texture
	pTexture->Unmap(0);

	// Create shader resource view to bind the texture
	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ID3D10Texture2D* pTexture2D = pTexture;

	D3D10_TEXTURE2D_DESC desc;
	pTexture->GetDesc(&desc);
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = desc.MipLevels - 1;

	hr = d3ddevice_->CreateShaderResourceView(pTexture, &srvDesc, &g_pTextureRV);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Create shader resource view failed", L"Error", 0);
	}

	// Bind to shader variables
	g_pDiffuseVariable->SetResource(g_pTextureRV);
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
				cubes[n].Init(D3DXVECTOR3(x, y, z));
			}
		}
	}

	// Reset world matrix to Identity matrix for each unit cube
	D3DXMATRIX world_matrix;
	D3DXMatrixIdentity(&world_matrix);

	for (int i = 0; i < kNumCubes; ++i)
	{
		cubes[i].SetWorldMatrix(world_matrix);
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

D3DXVECTOR3 RubikCube::ScreenToVector3(int x, int y)
{
	D3DXVECTOR3 vector3 ;

	// Get Projection matrix
	D3DXMATRIX proj = camera_->GetProjMatrix();

	vector3.x = ((( 2.0f*x) / vp_.Width)  - 1.0f) / proj(0, 0);
	vector3.y = (((-2.0f*y) / vp_.Height) + 1.0f) / proj(1, 1);
	vector3.z = 1.0f ;

	// Get world matrix
	D3DXMATRIX world = camera_->GetWorldMatrix();

	// Get view matrix
	D3DXMATRIX view = camera_->GetViewMatrix();

	// Concatinate them in to single matrix
	D3DXMATRIX WorldView = world * view;

	// inverse it
	D3DXMATRIX worldviewInverse;
	D3DXMatrixInverse(&worldviewInverse, 0, &WorldView);

	D3DXVec3TransformCoord(&vector3, &vector3, &worldviewInverse) ;

	D3DXVec3Normalize(&vector3, &vector3) ;

	return vector3 ;
}

// Calculate the picking ray and transform it to model space 
// x and y are the screen coordinates when left button down
Ray RubikCube::CalculatePickingRay(int x, int y)
{
	float px = 0.0f;
	float py = 0.0f;

	// Get Projection matrix
	D3DXMATRIX proj = camera_->GetProjMatrix();

	px = ((( 2.0f*x) / vp_.Width)  - 1.0f) / proj(0, 0);
	py = (((-2.0f*y) / vp_.Height) + 1.0f) / proj(1, 1);

	Ray ray;
	ray.origin    = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	ray.direction = D3DXVECTOR3(px, py, 1.0f); 

	// Get view matrix
	D3DXMATRIX view = camera_->GetViewMatrix();

	// Get world matrix
	D3DXMATRIX world = camera_->GetWorldMatrix();
	// d3ddevice_->GetTransform(D3DTS_WORLD, &world);

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

/*
We get the picked face by check the hit_point's coordinates(x, y, z), suppose the length of the small cube
is cube_length, and the gap length between two layers is gaps, and the Rubik cube's center is coordiates origin.
then, if the left face was picked, then, x == -(1.5 * cube_length + gaps), if the top face was picked, then 
y == 1.5 * length + gaps
*/
Face RubikCube::GetPickedFace(D3DXVECTOR3 hit_point) const
{
	float float_epsilon = 0.001f;
	float cube_length = cubes[0].GetLength();
	float face_length = kNumLayers * cube_length + (kNumLayers - 1) * gap_between_layers_;
	float half_face_length = face_length / 2;

	if (fabs(hit_point.z + half_face_length) < float_epsilon) { return kFrontFace;  }
	if (fabs(hit_point.z - half_face_length) < float_epsilon) { return kBackFace;   }
	if (fabs(hit_point.x + half_face_length) < float_epsilon) { return kLeftFace;   }
	if (fabs(hit_point.x - half_face_length) < float_epsilon) { return kRightFace;  }
	if (fabs(hit_point.y - half_face_length) < float_epsilon) { return kTopFace;    }
	if (fabs(hit_point.y + half_face_length) < float_epsilon) { return kBottomFace; }

	return kUnknownFace;
}

D3DXPLANE RubikCube::GeneratePlane(Face face, D3DXVECTOR3& previous_point, D3DXVECTOR3& current_point)
{
	float abs_diff_x = fabs(previous_point.x - current_point.x);
	float abs_diff_y = fabs(previous_point.y - current_point.y);
	float abs_diff_z = fabs(previous_point.z - current_point.z);

	switch (face)
	{
	case kFrontFace:
	case kBackFace:
		if (abs_diff_x < abs_diff_y)
			return D3DXPLANE(1, 0, 0, -previous_point.x);
		else
			return D3DXPLANE(0, 1, 0, -previous_point.y);
		break;

	case kLeftFace:
	case kRightFace:
		if (abs_diff_y < abs_diff_z)
			return D3DXPLANE(0, 1, 0, -previous_point.y);
		else
			return D3DXPLANE(0, 0, 1, -previous_point.z);
		break;

	case kTopFace:
	case kBottomFace:
		if (abs_diff_x < abs_diff_z)
			return D3DXPLANE(1, 0, 0, -previous_point.x);
		else
			return D3DXPLANE(0, 0, 1, -previous_point.z);
		break;

	default:
		return D3DXPLANE(0, 0, 0, 0);
	}
}

D3DXVECTOR3 RubikCube::GetRotateAxis(Face face, D3DXVECTOR3& previous_point, D3DXVECTOR3& current_point)
{
	float abs_diff_x = fabs(previous_point.x - current_point.x);
	float abs_diff_y = fabs(previous_point.y - current_point.y);
	float abs_diff_z = fabs(previous_point.z - current_point.z);

	switch (face)
	{
	case kFrontFace:
	case kBackFace:
		if (abs_diff_x < abs_diff_y)
			return D3DXVECTOR3(1, 0, 0);
		else
			return D3DXVECTOR3(0, 1, 0);
		break;

	case kLeftFace:
	case kRightFace:
		if (abs_diff_y < abs_diff_z)
			return D3DXVECTOR3(0, 1, 0);
		else
			return D3DXVECTOR3(0, 0, 1);
		break;

	case kTopFace:
	case kBottomFace:
		if (abs_diff_x < abs_diff_z)
			return D3DXVECTOR3(1, 0, 0);
		else
			return D3DXVECTOR3(0, 0, 1);
		break;

	default:
		return D3DXVECTOR3(0, 0, 0);
	}
}

RotateDirection RubikCube::GetRotateDirection(Face face, D3DXVECTOR3& axis, D3DXVECTOR3& previous_vector, D3DXVECTOR3& current_vector)
{
	float delta_x = previous_vector.x - current_vector.x;
	float delta_y = previous_vector.y - current_vector.y;
	float delta_z = previous_vector.z - current_vector.z;

	// Rotate around x-axis
	if (axis.x != 0)
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
	else if (axis.y != 0)
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
	D3DXQUATERNION quat = world_arcball_->GetRotationQuatIncreament();

	//extract rotation angle from quaternion
	float angle = 2.0f * acosf(quat.w) * rotate_speed_ ;

	return angle;
}

int  RubikCube::GetHitLayer(Face face, D3DXVECTOR3& rotate_axis, D3DXVECTOR3& hit_point)
{
	float length = cubes[0].GetLength();
	float gap    = gap_between_layers_;

	float half_face_length = face_length_ / 2;
	float float_epsilon = 0.0001f; 

	// X-Axis 
	if (rotate_axis.x != 0)
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
					if (hit_point.x + half_face_length >= i * (length + gap) 
					    && hit_point.x + half_face_length <= (i + 1) * (length + gap) - gap)
						return i;
				}
			}
		break;
		}
	}

	// Y-Axis
	else if (rotate_axis.y != 0)
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
					if (hit_point.y + half_face_length >= i * (length + gap)
						&& hit_point.y + half_face_length <= (i + 1) * (length + gap) - gap)
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
					if (hit_point.z + half_face_length >= i * (length + gap)
						&& hit_point.z + half_face_length <= (i + 1) * (length + gap) - gap)
						return i + 2 * kNumLayers;
				}
			}
			break;
		}
	}
}

void RubikCube::RotateLayer(int layer, D3DXVECTOR3& axis, float angle)
{
	for(int i = 0; i < kNumCubes; ++i)
	{
		if (cubes[i].InLayer(layer))
		{
			cubes[i].Rotate(axis, angle);
		}
	}
}
