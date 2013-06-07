#include "RubikCube.h"
#include "DXErr.h"
#include <time.h>

RubikCube::RubikCube(void)
    : kNumCubes(27),
	  total_rotate_angle_(0),
	  rotate_speed_(1.5f),
	  is_hit_(false),
	  is_cubes_selected_(false)
{
	d3d9 = new D3D9();

	camera = new Camera();

	// Create 27 unit cubes
	cubes = new Cube[kNumCubes];

	cube_length_ = 10.0f;
	gap_between_layers_ = 0.2f;

	// Cube length
	length = 10.0f;

	// Spaces between cubes
	gaps = 0.2f;

	// Create 6 faces
	faces = new Rect[6];

	// The Rubik cube was build in the following order
	// The center of the cube is the origin of the coordiantes system, so we can 
	// Calulate the 8 corners of the Rubik cube with length and gaps
	D3DXVECTOR3 A(-(1.5f * length + gaps),   1.5f * length + gaps , -(1.5f * length + gaps)); // The front-top-left corner
	D3DXVECTOR3 B(  1.5f * length + gaps ,   1.5f * length + gaps , -(1.5f * length + gaps));
	D3DXVECTOR3 C(  1.5f * length + gaps , -(1.5f * length + gaps), -(1.5f * length + gaps));
	D3DXVECTOR3 D(-(1.5f * length + gaps), -(1.5f * length + gaps), -(1.5f * length + gaps));

	D3DXVECTOR3 E(-(1.5f * length + gaps),   1.5f * length + gaps , (1.5f * length + gaps)); // The back-top-left corner
	D3DXVECTOR3 F(  1.5f * length + gaps ,   1.5f * length + gaps , (1.5f * length + gaps));
	D3DXVECTOR3 G(  1.5f * length + gaps , -(1.5f * length + gaps), (1.5f * length + gaps));
	D3DXVECTOR3 H(-(1.5f * length + gaps), -(1.5f * length + gaps), (1.5f * length + gaps));

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

	// Face length
	faceLength = 3 * length + 2 * gaps;

	// Half length of the face
	HalfWidth = faceLength / 2;

	inactive = false;
	OneRotateFinish = true;

	initWindowPosX   = 50;
	initWindowPosY   = 50;

	InitWindowWidth  = 800 ;
	InitWindowHeight = 800 ;

	CurrentWindowWidth = InitWindowWidth ;
	CurrentWindowHeight = InitWindowHeight ;

	LastWindowWidth = CurrentWindowWidth ;
	LastWindowHeight = CurrentWindowHeight ;

	texWidth  = 128;
	texHeight = 128;

	for(int i = 0; i < numFaces; ++i)
	{
		textureId[i] = -1;
		pTextures[i] = NULL;
	}

	pInnerTexture = NULL;
}

RubikCube::~RubikCube(void)
{
	// Delete d3d9 objects;
	delete d3d9;
	d3d9 = NULL;

	// Delete camera
	delete camera;
	camera = NULL;

	// Delete cubes
	delete []cubes;
	cubes = NULL;

	// Delete faces
	delete []faces;
	faces = NULL;

	// Release textures
	for(int i = 0; i < numFaces; ++i)
	{
		if(pTextures[i] != NULL)
		{
			pTextures[i]->Release();
			pTextures[i] = NULL;
		}
	}
}

void RubikCube::Initialize(HWND hWnd)
{
	d3d9->InitD3D9(hWnd);
	this->hWnd = hWnd;

	InitTextures();

	// Initialize the top-front-left corner of each unit cubes, we use 27 unit
	// cubes to build up a Rubik cube.
	D3DXVECTOR3 initPos[] = 
	{
		// Front layer, from left to right, top to bottom. 9 cubes
		D3DXVECTOR3( -(1.5f * length + gaps),    1.5f * length + gaps, -(1.5f * length + gaps) ),
		D3DXVECTOR3(          -0.5f * length,    1.5f * length + gaps, -(1.5f * length + gaps) ),
		D3DXVECTOR3(    0.5f * length + gaps,    1.5f * length + gaps, -(1.5f * length + gaps) ),

		D3DXVECTOR3( -(1.5f * length + gaps),           0.5f * length, -(1.5f * length + gaps) ),
		D3DXVECTOR3(          -0.5f * length,           0.5f * length, -(1.5f * length + gaps) ),
		D3DXVECTOR3(    0.5f * length + gaps,           0.5f * length, -(1.5f * length + gaps) ),

		D3DXVECTOR3( -(1.5f * length + gaps), -(0.5f * length + gaps), -(1.5f * length + gaps) ),
		D3DXVECTOR3(          -0.5f * length, -(0.5f * length + gaps), -(1.5f * length + gaps) ),
		D3DXVECTOR3(    0.5f * length + gaps, -(0.5f * length + gaps), -(1.5f * length + gaps) ),

		// Middle layer, from left to right, top to bottom. 9 cubes
		D3DXVECTOR3( -(1.5f * length + gaps),    1.5f * length + gaps,          -0.5f * length ),
		D3DXVECTOR3(          -0.5f * length,    1.5f * length + gaps,          -0.5f * length ),
		D3DXVECTOR3(    0.5f * length + gaps,    1.5f * length + gaps,          -0.5f * length ),

		D3DXVECTOR3( -(1.5f * length + gaps),           0.5f * length,          -0.5f * length ),
		D3DXVECTOR3(          -0.5f * length,           0.5f * length,          -0.5f * length ),
		D3DXVECTOR3(    0.5f * length + gaps,           0.5f * length,          -0.5f * length ),

		D3DXVECTOR3( -(1.5f * length + gaps), -(0.5f * length + gaps),          -0.5f * length ),
		D3DXVECTOR3(          -0.5f * length, -(0.5f * length + gaps),          -0.5f * length ),
		D3DXVECTOR3(    0.5f * length + gaps, -(0.5f * length + gaps),          -0.5f * length ),

		// Back layer, from left to right, top to bottom. 9 cubes
		D3DXVECTOR3( -(1.5f * length + gaps),    1.5f * length + gaps,    0.5f * length + gaps ),
		D3DXVECTOR3(          -0.5f * length,    1.5f * length + gaps,    0.5f * length + gaps ),
		D3DXVECTOR3(    0.5f * length + gaps,    1.5f * length + gaps,    0.5f * length + gaps ),

		D3DXVECTOR3( -(1.5f * length + gaps),           0.5f * length,    0.5f * length + gaps ),
		D3DXVECTOR3(          -0.5f * length,           0.5f * length,    0.5f * length + gaps ),
		D3DXVECTOR3(    0.5f * length + gaps,           0.5f * length,    0.5f * length + gaps ),

		D3DXVECTOR3( -(1.5f * length + gaps), -(0.5f * length + gaps),    0.5f * length + gaps ),
		D3DXVECTOR3(          -0.5f * length, -(0.5f * length + gaps),    0.5f * length + gaps ),
		D3DXVECTOR3(    0.5f * length + gaps, -(0.5f * length + gaps),    0.5f * length + gaps ),
	};

	// Initialize the 27 unit cubes
	for(int i = 0; i < kNumCubes; i++)
	{
		cubes[i].SetDevice(d3d9->getD3DDevice());
		cubes[i].Init(initPos[i]);
	}

	// Set texture for each cube and each face

	// Front face
	for(int i = 0; i <= 8; ++i)
	{
		cubes[i].SetTextureId(0, 0);
	}

	// Back face
	for(int i = 18; i <= 26; ++i)
	{
		cubes[i].SetTextureId(1, 1);
	}

	// Left face
	for(int i = 0; i <= 24; i += 3)
	{
		cubes[i].SetTextureId(2, 2);
	}

	// Right face
	for(int i = 2; i <= 26; i += 3)
	{
		cubes[i].SetTextureId(3, 3);
	}

	// Top face
	for(int i = 0; i <= 20; ++i)
	{
		if(i % 9 < 3)
		{
			cubes[i].SetTextureId(4, 4);
		}
	}

	// Bottom face
	for(int i = 6; i <= 26; ++i)
	{
		if(i % 9 > 5)
		{
			cubes[i].SetTextureId(5, 5);
		}
	}
}

void RubikCube::Render()
{
	// Game paused or device lost, yields 25ms to other program
	if(inactive)
	{
		Sleep(25) ;
	}

	// Update frame
	d3d9->FrameMove() ;

	d3d9->setupMatrix();

	d3d9->setupLight();

	LPDIRECT3DDEVICE9 pd3dDevice = d3d9->getD3DDevice();

	// Clear the back buffer to a black color
	pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x4F94CD, 1.0f, 0);

	if( SUCCEEDED(pd3dDevice->BeginScene()))
	{
		//draw all unit cubes to build the Rubik cube
		for(int i = 0; i < kNumCubes; i++)
		{
			cubes[i].Draw();
		}

		// Restore world matrix since the Draw function in class Cube has set the world matrix for each cube
		D3DXMATRIX matWorld = camera->GetWorldMatrix() ;
		pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld) ;

		pd3dDevice->EndScene();
	}

	// Present the back buffer contents to the display
	HRESULT hr = pd3dDevice->Present(NULL, NULL, NULL, NULL);

	// Render failed, try to reset device
	if(FAILED(hr))
	{
		d3d9->ResetDevice() ;
	}
}

void RubikCube::Shuffle()
{
	if(!OneRotateFinish)
		return ;
	OneRotateFinish = false ;

	// Set the random seed
	srand((unsigned int)time(0));

	// Generate direction
	char dir = ' ' ;
	if(rand() % 2 == 0)
		dir = 'c' ;
	else
		dir = 'r' ;

	// Rotate 20 times
	for(int i = 0; i < 20; i++)
	{
		// Generate the layer
		int layer = rand() % 9 ;
		//RotationLayer(layer, dir, D3DX_PI / 2) ;
		//UpdateLayerInfo(layer, dir, 1) ;
	}

	OneRotateFinish = true ;
}

// Switch from window mode and full-screen mode
void RubikCube::ToggleFullScreen()
{
	wp.length = sizeof(WINDOWPLACEMENT) ;

	// Window -> Full-Screen
	if(d3d9->getFullScreen() == false)
	{
		d3d9->setFullScreen(true);
		d3d9->setWindowMode(false);

		// Get and save window placement
		GetWindowPlacement(hWnd, &wp) ;

		// Update back buffer to desktop resolution
		d3d9->setBackBufferWidth(d3d9->getMaxDisplayWidth());
		d3d9->setBackBufferHeight(d3d9->getMaxDiplayHeight());
	}
	else // Full-Screen -> Window
	{
		d3d9->setFullScreen(false);
		d3d9->setWindowMode(true);

		// Get window placement
		int windowWidth  = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
		int windowHeight = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;

		//// Why does this not work?
		//d3d9->setBackBufferWidth(windowWidth);
		//d3d9->setBackBufferHeight(windowHeight);

		// Update back buffer size
		d3d9->setBackBufferWidth(LastWindowWidth);
		d3d9->setBackBufferHeight(LastWindowHeight);

		// Restore window placement
		SetWindowPlacement(hWnd, &wp) ;
	}

	// Display mode changed, we need to reset device
	d3d9->ResetDevice() ;
}

void RubikCube::OnLeftButtonDown(int x, int y)
{
	if(!OneRotateFinish) // another rotate is in process, return directly
		return ;
	OneRotateFinish = false ; // Prevent the other rotation during this rotate

	is_cubes_selected_ = false;

	Ray ray = d3d9->CalculatePickingRay(x, y) ;

	D3DXVECTOR3 currentHitPoint;	// hit point on the face
	float maxDist = 100000.0f ;

	// Select the face nearest to the camera
	for(int i = 0; i < numFaces; i++)
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
	WorldBall.OnBegin(x, y) ;
}

void RubikCube::OnMouseMove(int x, int y)
{
	WorldBall.OnMove(x, y) ;

	// Get the picked face
	Face face = GetPickedFace(previous_hitpoint_);

	Ray picking_ray = d3d9->CalculatePickingRay(x, y);
	RayRectIntersection(picking_ray, faces[face], current_hitpoint_);

	D3DXPLANE plane = GeneratePlane(face, previous_hitpoint_, current_hitpoint_);

	if (!is_cubes_selected_)
	{
		MarkRotateCubes(plane);
		is_cubes_selected_ = true;
	
		rotate_axis_ = GetRotateAxis(face, previous_hitpoint_, current_hitpoint_);
	}

	float angle = CalculateRotateAngle();

	rotate_direction_ = GetRotateDirection(face, rotate_axis_, previous_hitpoint_, current_hitpoint_);

	// Flip the angle if the direction is clockwise
	// the positive direction is counter-clockwise around the rotate axis when look through the axis toward the origin.
	if (rotate_direction_ == kClockWise)
		angle = -angle;

	// Accumulate total angle
	total_rotate_angle_ += angle;

	// Rotate
	Rotate(rotate_axis_, angle);
}

// When Left button up, complete the rotation of the left angle to align the cube and update the layer info
void RubikCube::OnLeftButtonUp()
{
	is_hit_ = false ;

	WorldBall.OnEnd();

	float left_angle = 0.0f ;	// the angle need to rotate when mouse is up
	int numOfHalfPI = 0 ;

	// The left direction
	RotateDirection left_direction;
	if(total_rotate_angle_ >= 0.0f)
		left_direction = kClockWise ;
	else
		left_direction = kCounterClockWise ;

	// Count the absolute value of total_angle_
	total_rotate_angle_ = abs(total_rotate_angle_) ;

	// Count how many 90 degrees the cube has rotate
	while (total_rotate_angle_ > D3DX_PI / 2)
	{
		total_rotate_angle_ -= D3DX_PI / 2;
		numOfHalfPI++ ;
	}

	// more than D3DX_PI / 4 and less than D3DX_PI / 2 it treated as D3DX_PI / 2
	if(total_rotate_angle_ > D3DX_PI / 4)
	{
		numOfHalfPI++ ;
		left_angle = D3DX_PI / 2 - total_rotate_angle_ ;
	}
	else
	{
		left_angle = -total_rotate_angle_;
	}

	Rotate(rotate_axis_, left_angle);

	// clear total_angle_ for next accumulation
	total_rotate_angle_ = 0.0f ;

	OneRotateFinish = true ; // Make the next rotate enable

	// Restore all selected flags of cubes
	for (int i = 0; i < kNumCubes; ++i)
	{
		cubes[i].SetIsSelected(false);
	}
}

LRESULT RubikCube::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)    
	{
	case WM_CREATE:
		break;

	case WM_ACTIVATE:
		if(LOWORD(wParam) == WA_INACTIVE)
			inactive = true ;
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

	case WM_CAPTURECHANGED:
		OnLeftButtonUp();
		ReleaseCapture();
		break;

	case WM_LBUTTONUP:
		{
			OnLeftButtonUp();
			ReleaseCapture();
		}
		break ;

	case WM_MOUSEMOVE:
		{
			// Only response when the picking ray hit the cube
			if(is_hit_)
			{
				int iMouseX = ( short )LOWORD( lParam );
				int iMouseY = ( short )HIWORD( lParam );
				OnMouseMove(iMouseX, iMouseY) ;
			}
		}
		break ;

	case WM_PAINT: 
		Render();
		break ;

	case WM_KEYDOWN:
		{
			switch( wParam )
			{
			case 'F':
				ToggleFullScreen() ;
				break;
			case 'R':
				Shuffle() ;
				break ;
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
				inactive = true ;

			else if (wParam == SIZE_MAXIMIZED)
			{
				// Get current window size
				CurrentWindowWidth = ( short )LOWORD( lParam );
				CurrentWindowHeight = ( short )HIWORD( lParam );

				if(CurrentWindowWidth != LastWindowWidth || CurrentWindowHeight != LastWindowHeight)
				{
					d3d9->setBackBufferWidth(CurrentWindowWidth);
					d3d9->setBackBufferHeight(CurrentWindowHeight);
					d3d9->ResetDevice();

					LastWindowWidth = CurrentWindowWidth ;
					LastWindowHeight = CurrentWindowHeight ;
				}
			}

			else if (wParam == SIZE_RESTORED)
			{
				inactive = false ;

				// Maximized -> Full Screen
				if (d3d9->getFullScreen() == true)
				{
					// Update back buffer to desktop resolution
					d3d9->setWindowMode(false);
					d3d9->setBackBufferWidth(d3d9->getMaxDisplayWidth());
					d3d9->setBackBufferHeight(d3d9->getMaxDiplayHeight());

					// Reset device
					d3d9->ResetDevice();
				}
				else
				{
					// Get current window size
					CurrentWindowWidth = ( short )LOWORD( lParam );
					CurrentWindowHeight = ( short )HIWORD( lParam );

					// Reset device
					d3d9->setBackBufferWidth(CurrentWindowWidth);
					d3d9->setBackBufferHeight(CurrentWindowHeight);
					d3d9->ResetDevice();

					LastWindowWidth = CurrentWindowWidth ;
					LastWindowHeight = CurrentWindowHeight ;
				}
			}
		}
		break ;

	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage (0) ;
		return 0 ;    
	}

	return d3d9->HandleMessages(hWnd, uMsg, wParam, lParam);
}

void RubikCube::InitTextures()
{
	DWORD colors[numFaces] = 
	{
		0xffffffff, // White,   front face
		0xffffff00, // Yellow,	back face
		0xffff0000, // Red,		left face
		0xffffa500,	// Orange,	right face
		0xff00ff00, // Green,	top face
		0xff0000ff, // Blue,	bottom face
	};

	// Create face textures
	for(int i = 0; i < numFaces; ++i)
	{
		pTextures[i] = d3d9->CreateTexture(texWidth, texHeight, colors[i]);
	}

	// Create inner texture
	pInnerTexture = d3d9->CreateInnerTexture(texWidth, texHeight, 0xffffffff);

	Cube::SetFaceTexture(pTextures, numFaces);
	Cube::SetInnerTexture(pInnerTexture);
}

int RubikCube::getWindowPosX() const
{
	return initWindowPosX;
}

int RubikCube::getWindowPosY() const
{
	return initWindowPosY;
}

int RubikCube::getWindowWidth() const
{
	return InitWindowWidth;
}

int RubikCube::getWindowHeight() const
{
	return InitWindowHeight;
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
	float face_length = 3 * cube_length_ + 2 * gap_between_layers_;
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

void RubikCube::MarkRotateCubes(D3DXPLANE& plane)
{
	for (int i = 0; i < kNumCubes; ++i)
	{
		// Build a box with the cube's min/max points
		D3DXVECTOR3 min_point = cubes[i].GetMinPoint();
		D3DXVECTOR3 max_point = cubes[i].GetMaxPoint();
		Box box(min_point, max_point);

		// Test whether box intersection with the plane
		// if true, set the cube as selected, the selected cube will be rotated 
		if (PlaneBoxIntersection(plane, box))
		{
			cubes[i].SetIsSelected(true);
		}
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

RotateDirection RubikCube::GetRotateDirection(Face face, D3DXVECTOR3& axis, D3DXVECTOR3& previous_point, D3DXVECTOR3& current_point)
{
	float delta_x = previous_point.x - current_point.x;
	float delta_y = previous_point.y - current_point.y;
	float delta_z = previous_point.z - current_point.z;

	// Rotate around x-axis
	if (axis.x != 0)
	{
		switch (face)
		{
			case kFrontFace:
				if (delta_y < 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			case kBackFace:
				if (delta_y > 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			case kTopFace:
				if (delta_z < 0) { return kCounterClockWise; }
				else             { return kClockWise; }
				break;

			case kBottomFace:
				if (delta_z > 0) { return kCounterClockWise; }
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
	D3DXQUATERNION quat = WorldBall.GetRotationQuatIncreament();

	//extract rotation angle from quaternion
	float angle = 2.0f * acosf(quat.w) * rotate_speed_ ;

	return angle;
}

void RubikCube::Rotate(D3DXVECTOR3& axis, float angle)
{
	for (int i = 0; i < kNumCubes; ++i)
	{
		if (cubes[i].GetIsSelected())
		{
			cubes[i].Rotate(axis, angle);
		}
	}
}