#include "pch.h"
#include "ModelViewCamera.h"

ModelViewCamera::ModelViewCamera(void)
{
	m_matWorld         = XMMatrixIdentity();
	m_matModelRotate   = XMMatrixIdentity();
	m_matModelRotLast  = XMMatrixIdentity();
	m_matCameraRotLast = XMMatrixIdentity();
//	m_vModelCenter     = XMFLOAT3( 0, 0, 0 );
	m_fRadius          = 2.0f;			
	m_fMaxRadius       = 1500.0f;
	m_fMinRadius       = 200.0f ;
	m_nMouseWheelDelta = 0 ;

	m_bDragSinceLastUpdate = false ;
}

ModelViewCamera::~ModelViewCamera(void)
{
}

void ModelViewCamera::Init(float windowWidth, float windowHeight)
{
	m_WorldArcBall.Init(windowWidth, windowHeight);
	m_ViewArcBall.Init(windowWidth, windowHeight);
}

void ModelViewCamera::Reset()
{
	m_matWorld         = XMMatrixIdentity() ;
	m_matModelRotate   = XMMatrixIdentity() ;
	m_matModelRotLast  = XMMatrixIdentity() ;
	m_matCameraRotLast = XMMatrixIdentity();

	m_bDragSinceLastUpdate = false ;
	m_WorldArcBall.Reset();
	m_ViewArcBall.Reset();
}

// Update the scene for every frame
void ModelViewCamera::OnFrameMove()
{
	// Return directly if there is no pointer event since the last update
	// The pointer events need to handle including
	// 1. Drag pointer and move
	// 2. Mouse wheel rotation
	if(!m_bDragSinceLastUpdate)
		return ;

	m_bDragSinceLastUpdate = false ;

	if(m_nMouseWheelDelta)
	{
		m_fRadius -= m_nMouseWheelDelta * m_fRadius * 0.1f / 120.0f;

		// Make the radius in range of [m_fMinRadius, m_fMaxRadius]
		// This can Prevent the cube became too big or too small
		//m_fRadius = max(m_fRadius, m_fMinRadius) ;
		//m_fRadius = min(m_fRadius, m_fMaxRadius) ;
	}

	// The mouse delta is retrieved IN every WM_MOUSE message and do not accumulate, so clear it after one frame
	m_nMouseWheelDelta = 0 ;

	// Get the inverse of the view Arcball's rotation matrix
	XMMATRIX mCameraRot ;
	mCameraRot = XMMatrixInverse( NULL, m_ViewArcBall.GetRotationMatrix() );

	// Transform vectors based on camera's rotation matrix
	XMVECTOR vWorldUp;
	XMVECTOR vLocalUp = XMVectorSet(0, 1, 0, 0);
	vWorldUp = XMVector3TransformCoord(vLocalUp, mCameraRot);

	XMVECTOR vWorldAhead;
	XMVECTOR vLocalAhead = XMVectorSet(0, 0, 1, 0);
	vWorldAhead = XMVector3TransformCoord(vLocalAhead, mCameraRot);

	// Update the eye point based on a radius away from the lookAt position
	m_vEyePt = XMVectorSubtract(m_vLookatPt, XMVectorScale(vWorldAhead, m_fRadius));

	// Update the view matrix
	m_matView = XMMatrixLookAtLH(m_vEyePt, m_vLookatPt, vWorldUp);
}

void ModelViewCamera::OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	m_bDragSinceLastUpdate = true ;

	// Enable pointer capture
	sender->SetPointerCapture();

	// Get pointer position
	float pointerX = args->CurrentPoint->Position.X;
	float pointerY = args->CurrentPoint->Position.Y;

	m_ViewArcBall.OnBegin(pointerX, pointerY) ;
}

void ModelViewCamera::OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	m_bDragSinceLastUpdate = true ;
	sender->ReleasePointerCapture();
	m_ViewArcBall.OnEnd();
}

void ModelViewCamera::OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	m_bDragSinceLastUpdate = true ;

	// Get pointer position
	float pointerX = args->CurrentPoint->Position.X;
	float pointerY = args->CurrentPoint->Position.Y;

	// Update view/world arcball
	//m_WorldArcBall.OnMove(pointerX, pointerY) ;
	m_ViewArcBall.OnMove(pointerX, pointerY) ;
}

void ModelViewCamera::OnMouseWheelRotate(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	// Mouse wheel, zoom in/out
	m_bDragSinceLastUpdate = true ;
	m_nMouseWheelDelta += args->CurrentPoint->Properties->MouseWheelDelta;
}

void ModelViewCamera::SetViewParams(XMFLOAT3 pvEyePt, XMFLOAT3 pvLookatPt, XMFLOAT3 pvUp)
{
	m_vEyePt	= XMLoadFloat3(&pvEyePt);
	m_vLookatPt = XMLoadFloat3(&pvLookatPt);
	m_vUp		= XMLoadFloat3(&pvUp);

	m_matView = XMMatrixLookAtLH(m_vEyePt, m_vLookatPt, m_vUp);
	m_bDragSinceLastUpdate = true ;
}

void ModelViewCamera::SetProjParams(float fFOV, float fAspect, float fNearPlane, float fFarPlane)
{
	m_fFOV			= fFOV ;
	m_fAspect		= fAspect ;
	m_fNearPlane	= fNearPlane ;
	m_fFarPlane		= fFarPlane ;

	m_matProj = XMMatrixPerspectiveFovLH(fFOV, fAspect, fNearPlane, fFarPlane) ;
	m_bDragSinceLastUpdate = true ;
}

void ModelViewCamera::SetWindow(float nWidth, float nHeight, float fArcballRadius)
{
	m_WorldArcBall.SetWindow(nWidth, nHeight, fArcballRadius) ;
	m_ViewArcBall.SetWindow(nWidth, nHeight, fArcballRadius) ;
}

const XMMATRIX* ModelViewCamera::GetWorldMatrix() const
{
	return &m_matWorld ;
}

const XMMATRIX* ModelViewCamera::GetViewMatrix() const
{
	return &m_matView ;
}

const XMMATRIX* ModelViewCamera::GetProjMatrix() const
{
	return &m_matProj ;
}

void ModelViewCamera::SetWorldMatrix(XMMATRIX *matModelWorld)
{
	m_matWorld = *matModelWorld ;
	m_bDragSinceLastUpdate = true ;
}

XMVECTOR ModelViewCamera::GetRotationQuat()
{
	return m_WorldArcBall.GetRotationQuat() ;
}

const XMMATRIX* ModelViewCamera::GetModelRotMatrix() const
{
	return &m_matModelRotate ;
}