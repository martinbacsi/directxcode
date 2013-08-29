#include "ArcBall.h"

ArcBall::ArcBall(void)
    : is_dragged_(false),
	  radius_(1.0f),
	  previous_point_(XMVectorSet(0, 0, 0, 1)),
	  current_point_(XMVectorSet(0, 0, 0, 1)),
	  old_point_(XMVectorSet(0, 0, 0, 1)),
	  previous_quaternion_(XMVectorSet(0, 0, 0, 1)),
      current_quaternion_(XMVectorSet(0, 0, 0, 1)),
      rotation_increament_(XMVectorSet(0, 0, 0, 1))
{
	rotate_matrix_ = XMMatrixIdentity();

	RECT rc ;
	GetClientRect(GetForegroundWindow(), &rc) ;

	int window_width  = rc.right - rc.left;
	int window_height = rc.bottom - rc.top;

	SetWindow(window_width, window_height) ;

	/*previous_point_ = XMVectorSet(0, 0, 0, 1);
	current_point_ = XMVectorSet(0, 0, 0, 1);
	old_point_ = XMVectorSet(0, 0, 0, 1);
	previous_quaternion_ = XMVectorSet(0, 0, 0, 1);
	current_quaternion_ = XMVectorSet(0, 0, 0, 1);
	rotation_increament_ = XMVectorSet(0, 0, 0, 1);*/
}

ArcBall::~ArcBall(void)
{
}

void ArcBall::Reset()
{
	is_dragged_ = false;
	radius_ = 1.0f;
	previous_quaternion_ = XMVectorSet(0, 0, 0, 1);
	current_quaternion_ = XMVectorSet(0, 0, 0, 1);
	rotation_increament_ = XMVectorSet(0, 0, 0, 1);
	rotate_matrix_ = XMMatrixIdentity();
}

void ArcBall::OnBegin(int mouse_x, int mouse_y)
{
	// enter drag state only if user click the window's client area
	if(mouse_x >= 0 && mouse_x <= window_width_ 
	   && mouse_y >= 0 && mouse_y < window_height_)
	{
		is_dragged_ = true ; // begin drag state
		previous_quaternion_ = current_quaternion_ ;
		previous_point_ = ScreenToVector(mouse_x, mouse_y);
		old_point_ = previous_point_ ;
	}
}

void ArcBall::OnMove(int mouse_x, int mouse_y)
{
	if(is_dragged_)
	{
		current_point_ = ScreenToVector(mouse_x, mouse_y) ;
		rotation_increament_ = QuatFromBallPoints(old_point_, current_point_) ;
		current_quaternion_ = XMQuaternionMultiply(previous_quaternion_, QuatFromBallPoints( previous_point_, current_point_)) ;
		old_point_ = current_point_ ;
	}
}

void ArcBall::OnEnd()
{
	is_dragged_ = false ;
}

void ArcBall::SetWindow(int window_width, int window_height, float arcball_radius)
{
	 window_width_  = window_width; 
	 window_height_ = window_height; 
	 radius_		= arcball_radius; 
}

XMMATRIX ArcBall::GetRotationMatrix()
{
	rotate_matrix_ = XMMatrixRotationQuaternion(current_quaternion_) ;
	return rotate_matrix_;
}

XMVECTOR ArcBall::GetRotationQuatIncreament()
{
	return rotation_increament_ ;
}

XMVECTOR ArcBall::QuatFromBallPoints(XMVECTOR& start_point, XMVECTOR& end_point)
{
	XMVECTOR dotVector = XMVector3Dot(start_point, end_point);
	float fDot = XMVectorGetX(dotVector);

	XMVECTOR vPart;
	vPart = XMVector3Cross(start_point, end_point);	

	XMVECTOR result = XMVectorSet(XMVectorGetX(vPart), XMVectorGetY(vPart), XMVectorGetZ(vPart), fDot);
	return result;
}

XMVECTOR ArcBall::ScreenToVector(int screen_x, int screen_y)
{
	// Scale to screen
	float x = -(screen_x - window_width_ / 2) / (radius_ * window_width_ / 2);
	float y = (screen_y - window_height_ / 2) / (radius_ * window_height_ / 2);

	float z = 0.0f;
	float mag = x * x + y * y;

	if(mag > 1.0f)
	{
		float scale = 1.0f / sqrtf(mag);
		x *= scale;
		y *= scale;
	}
	else
		z = sqrtf(1.0f - mag);

	return XMVectorSet(x, y, z, 1);
}
