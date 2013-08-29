#include "Camera.h"

Camera::Camera(void)
    : radius_(100),
	  min_radius_(50),
	  max_radius_(300),
	  mouse_wheel_delta_(0),
	  frame_need_update_(false)
{
	world_matrix_ = XMMatrixIdentity();
	view_matrix_  = XMMatrixIdentity();
}

Camera::~Camera(void)
{
}

void Camera::Reset()
{
	frame_need_update_ = false ;
	world_matrix_ = XMMatrixIdentity();
	view_arcball_.Reset();
}

// Update the scene for every frame
void Camera::OnFrameMove()
{
	// No need to handle if no drag since last frame move
	if(!frame_need_update_)
		return ;
	frame_need_update_ = false ;

	if(mouse_wheel_delta_)
	{
		radius_ -= mouse_wheel_delta_ * radius_ * 0.1f / 360.0f;

		// Make the radius in range of [min_radius_, max_radius_]
		// This can Prevent the cube became too big or too small
		radius_ = max(radius_, min_radius_) ;
		radius_ = min(radius_, max_radius_) ;
	}

	// The mouse delta is retrieved IN every WM_MOUSE message and do not accumulate, so clear it after one frame
	mouse_wheel_delta_ = 0 ;

	// Get the inverse of the view Arcball's rotation matrix
	XMMATRIX rotate_matrix_inverse ;
	rotate_matrix_inverse = XMMatrixInverse(NULL, view_arcball_.GetRotationMatrix());

	// Transform local up vector based on camera's rotation matrix
	XMVECTOR world_up_vector;
	XMVECTOR loacal_up_vector = XMVectorSet(0, 1, 0, 0);
	world_up_vector = XMVector3TransformCoord(loacal_up_vector, rotate_matrix_inverse);

	// Transform local ahead vector based on camera's rotation matrix
	XMVECTOR world_ahead_vector;
	XMVECTOR local_ahead_vector = XMVectorSet(0, 0, 1, 0);
	world_ahead_vector = XMVector3TransformCoord(local_ahead_vector, rotate_matrix_inverse);

	// Update the eye point based on a radius away from the lookAt position
	eye_point_ = lookat_point_ - world_ahead_vector * radius_;

	// Update the view matrix
	view_matrix_ = XMMatrixLookAtLH(eye_point_, lookat_point_, world_up_vector);
}

// This function is used to handling the mouse message for the view arc ball
// include the right button message, the left button message is handled in the game window message process function
LRESULT Camera::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// update view arc ball
	if(uMsg == WM_RBUTTONDOWN)
	{
		SetCapture(hWnd) ;

		frame_need_update_ = true ;
		int mouse_x = (short)LOWORD(lParam) ;
		int mouse_y = (short)HIWORD(lParam) ;
		view_arcball_.OnBegin(mouse_x, mouse_y) ;
	}

	// mouse move
	if(uMsg == WM_MOUSEMOVE)
	{
		frame_need_update_ = true ;
		int mouse_x = (short)LOWORD(lParam);
		int mouse_y = (short)HIWORD(lParam);
		view_arcball_.OnMove(mouse_x, mouse_y) ;
	}

	// right button up, terminate view arc ball rotation
	if(uMsg == WM_RBUTTONUP)
	{
		frame_need_update_ = true ;
		view_arcball_.OnEnd();
		ReleaseCapture() ;
	}

	// Mouse wheel, zoom in/out
	if(uMsg == WM_MOUSEWHEEL) 
	{
		frame_need_update_ = true ;
		mouse_wheel_delta_ += (short)HIWORD(wParam);
	}

	return TRUE ;
}

void Camera::SetWorldMatrix(const XMMATRIX& world_matrix)
{
	world_matrix_ = world_matrix;
}

void Camera::SetViewParams(const XMVECTOR& eye_point, const XMVECTOR& lookat_point, const XMVECTOR& up_vector)
{
	eye_point_	  = eye_point ;
	lookat_point_ = lookat_point ;
	up_vector_	  = up_vector ;

	view_matrix_ = XMMatrixLookAtLH(eye_point, lookat_point, up_vector) ;
	frame_need_update_ = true ;
}

void Camera::SetProjParams(float field_of_view, float aspect_ratio, float near_plane, float far_plane)
{
	proj_matrix = XMMatrixPerspectiveFovLH(field_of_view, aspect_ratio, near_plane, far_plane) ;
	frame_need_update_ = true ;
}

void Camera::SetWindow(int window_width, int window_height, float arcball_radius)
{
	view_arcball_.SetWindow(window_width, window_height, arcball_radius) ;
}

const XMMATRIX Camera::GetWorldMatrix() const
{
	return world_matrix_ ;
}

const XMMATRIX Camera::GetViewMatrix() const
{
	return view_matrix_ ;
}

const XMMATRIX Camera::GetProjMatrix() const
{
	return proj_matrix ;
}

const XMVECTOR Camera::GetEyePoint() const
{
	return eye_point_ ;
}