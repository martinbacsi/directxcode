#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <Windows.h>
#include <DirectXMath.h>

#include "ArcBall.h"

using namespace DirectX;

class ModelViewCamera
{
public:
	ModelViewCamera(void);
	~ModelViewCamera(void);

	void Init(float windowWidth, float windowHeight);
	void Reset();
	void OnFrameMove() ;
	void ModelViewCamera::SetViewParams(XMFLOAT3 pvEyePt, XMFLOAT3 pvLookatPt, XMFLOAT3 pvUp);
	void SetProjParams(float fFOV, float fAspect, float fNearPlane, float fFarPlane) ;
	void SetWindow(float nWidth, float nHeight, float fArcballRadius = 0.9f) ;
	const XMMATRIX* GetWorldMatrix() const ;
	const XMMATRIX* GetViewMatrix() const ;
	const XMMATRIX* GetProjMatrix() const ;
	void SetWorldMatrix(XMMATRIX *matModelWorld) ;
	XMVECTOR GetRotationQuat();
	const XMMATRIX* GetModelRotMatrix() const ;
	LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) ;

private:
	bool m_bDragSinceLastUpdate ;
	float m_fRadius;				// Distance from the camera to model 
	float m_fMaxRadius ;			// The Maximum distance from the camera to the model
	float m_fMinRadius ;			// The Minimum distance from the camera to the model
	int m_nMouseWheelDelta;			// Amount of middle wheel scroll (+/-)
	XMFLOAT3 m_vModelCenter ;		// Model center
	
	XMVECTOR m_vEyePt ;				// Eye position
	XMVECTOR m_vLookatPt ;			// Look at position
	XMVECTOR m_vUp ;				// Up vector

	float m_fFOV;					// Field of view
	float m_fAspect;				// Aspect ratio
	float m_fNearPlane;				// Near plane
	float m_fFarPlane;				// Far plane
	
	XMMATRIX m_matView ;			// Camera View matrix
	XMMATRIX m_matProj ;			// Camera Projection matrix
	XMMATRIX m_matWorld ;			// World matrix of model
	XMMATRIX m_matModelRotate ;		// Rotation matrix of model
	XMMATRIX m_matModelRotLast ;	// model last rotation matrix
	XMMATRIX m_matCameraRotLast ;	// Camera last rotation matrix

	ArcBall m_WorldArcBall ;		// World arc ball 
	ArcBall m_ViewArcBall ;			// View arc ball
};

#endif // end __CAMERA_H__