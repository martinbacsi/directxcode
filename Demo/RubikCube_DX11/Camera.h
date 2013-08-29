#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "ArcBall.h"

class Camera
{
public:
	Camera(void);
	~Camera(void);

public:
	void Reset() ;
	void OnFrameMove() ;
	LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) ;
	void SetWorldMatrix(const XMMATRIX& world_matrix);
	void SetViewParams(const XMVECTOR& eye_point, const XMVECTOR& lookat_point, const XMVECTOR& up_vector);
	void SetProjParams(float field_of_view, float aspect_ratio, float near_plane, float far_plane) ;
	void SetWindow(int window_width, int window_height, float arcball_radius = 1.0f) ;
	const XMMATRIX GetWorldMatrix() const ;
	const XMMATRIX GetViewMatrix() const ;
	const XMMATRIX GetProjMatrix() const ;
	const XMVECTOR GetEyePoint() const ;

private:
	bool	frame_need_update_ ;
	float	radius_;				// Distance from the camera to model 
	float	max_radius_ ;			// The Maximum distance from the camera to the model
	float	min_radius_ ;			// The Minimum distance from the camera to the model
	int		mouse_wheel_delta_;		// Amount of middle wheel scroll (+/-)
	
	XMVECTOR eye_point_ ;			// Eye position
	XMVECTOR lookat_point_ ;		// Look at position
	XMVECTOR up_vector_ ;				// Up vector

	XMMATRIX world_matrix_ ;			// World matrix of model
	XMMATRIX view_matrix_ ;			// Camera View matrix
	XMMATRIX proj_matrix ;			// Camera Projection matrix

	ArcBall view_arcball_ ;			// View arc ball
};

#endif // end __CAMERA_H__