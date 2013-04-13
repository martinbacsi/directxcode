#include "pch.h"
#include "ArcBall.h"

ArcBall::ArcBall(void)
{
	Reset() ;
	m_vDownPt    = XMVectorSet( 0, 0, 0, 1);
	m_oldPt	     = XMVectorSet( 0, 0, 0, 1);
	m_vCurrentPt = XMVectorSet( 0, 0, 0, 1);
	m_Offset.x   = 0 ;
	m_Offset.y   = 0 ;
}

void ArcBall::Init(float nWidth, float nHeight)
{
	SetWindow(nWidth, nHeight);
}

ArcBall::~ArcBall()
{
}

void ArcBall::Reset()
{
	m_qDown      = XMFLOAT4(0, 0, 0, 1);
	m_qNow       = XMFLOAT4(0, 0, 0, 1);
	m_increament = XMFLOAT4(0, 0, 0, 1);
	m_mRotation  = XMMatrixIdentity();
	m_bDrag      = FALSE;
	m_fRadius    = 1.0f;
}

void ArcBall::OnBegin(float nX, float nY)
{
	// enter drag state only if user click the window's client area
	if( nX >= m_Offset.x && 
		nX <= m_Offset.x + m_nWidth &&
		nY >= m_Offset.y &&
		nY < m_Offset.y + m_nHeight)
	{
		m_bDrag = true ; // begin drag state
		m_qDown = m_qNow ;
		m_vDownPt = ScreenToVector(nX, nY);
		m_oldPt = m_vDownPt ;
	}
}

void ArcBall::OnMove(float nX, float nY)
{
	if(m_bDrag)
	{
		m_vCurrentPt = ScreenToVector(nX, nY);
		m_increament = QuatFromBallPoints( m_oldPt, m_vCurrentPt );

		XMStoreFloat4(&m_qNow, XMQuaternionMultiply(XMLoadFloat4(&m_qDown), XMLoadFloat4(&QuatFromBallPoints( m_vDownPt, m_vCurrentPt))));
		m_oldPt = m_vCurrentPt ;
	}
}

void ArcBall::OnEnd()
{
	m_bDrag = false ;
}

void ArcBall::SetOffset( INT nX, INT nY )
{
	m_Offset.x = nX ; 
	m_Offset.y = nY ;
}

void ArcBall::SetWindow(float nWidth, float nHeight, float fRadius)
{
	 m_nWidth  = nWidth; 
	 m_nHeight = nHeight; 
	 m_fRadius = fRadius; 
	 m_vCenter = XMFLOAT2(m_nWidth / 2.0f, m_nHeight / 2.0f);
}

XMFLOAT4 ArcBall::GetRotationQuat()
{
	return m_qNow ;
}

XMMATRIX ArcBall::GetRotationMatrix()
{
	m_mRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&m_qNow)) ;
	return m_mRotation;
}

XMFLOAT4 ArcBall::GetRotationQuatIncreament()
{
	return m_increament ;
}

XMMATRIX ArcBall::GetRotationMatrixIncreament()
{
	m_mRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&m_increament));
	return m_mRotation;
}

XMFLOAT4 ArcBall::QuatFromBallPoints(XMVECTOR startPoint, XMVECTOR endPoint )
{
	XMVECTOR dotVector = XMVector3Dot(startPoint, endPoint);
	float fDot = XMVectorGetX(dotVector);

	XMVECTOR vPart;
	vPart = XMVector3Cross(startPoint, endPoint);		

	XMFLOAT4 result = XMFLOAT4(XMVectorGetX(vPart), XMVectorGetY(vPart), XMVectorGetZ(vPart), fDot);
	return result;
}

XMVECTOR ArcBall::ScreenToVector(float fScreenPtX, float fScreenPtY)
{
	// Scale to screen
	FLOAT x = -( fScreenPtX - m_Offset.x - m_nWidth / 2 ) / ( m_fRadius * m_nWidth / 2 );
	FLOAT y = ( fScreenPtY - m_Offset.y - m_nHeight / 2 ) / ( m_fRadius * m_nHeight / 2 );

	FLOAT z = 0.0f;
	FLOAT mag = x * x + y * y;

	if( mag > 1.0f )
	{
		FLOAT scale = 1.0f / sqrtf( mag );
		x *= scale;
		y *= scale;
	}
	else
		z = sqrtf( 1.0f - mag );

	return XMVectorSet( x, y, z, 1);
}