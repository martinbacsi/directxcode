#ifndef __ARCBALL_H__
#define __ARCBALL_H__

#include <D3D11.h>
#include <DirectXMath.h>

using namespace DirectX;

class ArcBall
{
public:
	ArcBall(void);
	~ArcBall(void);

public:
	void Reset() ;
	void OnBegin(int mouse_x, int mouse_y) ;
	void OnMove(int mouse_x, int mouse_y) ;
	void OnEnd() ;

	XMVECTOR QuatFromBallPoints(XMVECTOR& start_point, XMVECTOR& end_point);
	XMMATRIX GetRotationMatrix() ;
	XMVECTOR GetRotationQuatIncreament() ;
	void SetWindow(int window_width, int window_height, float arcball_radius = 0.9f) ;

private:
	int		window_width_ ;	// arc ball's window width
	int		window_height_ ; // arc ball's window height
	float	radius_ ;	// arc ball's radius in screen coordinates
	bool	is_dragged_ ;	// whether the arc ball is dragged

	XMVECTOR	previous_quaternion_ ;	// quaternion before mouse down
	XMVECTOR	current_quaternion_ ;	// current quaternion
	XMVECTOR	rotation_increament_ ;	// rotation increment 
	XMVECTOR	previous_point_ ;		// starting point of arc ball rotate
	XMVECTOR	current_point_ ;		// current point of arc ball rotate
	XMVECTOR	old_point_ ;			// old point 
	XMMATRIX	rotate_matrix_;			// rotation matrix

	// Convert scree point to arcball point(vector)
	XMVECTOR ScreenToVector(int screen_x, int screen_y);
};

#endif // end __ARCBALL_H__