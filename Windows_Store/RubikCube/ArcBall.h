#ifndef __ARCBALL_H__
#define __ARCBALL_H__

using namespace DirectX;

ref class ArcBall sealed
{
public:
	ArcBall(void);

private:
	~ArcBall(void);

internal:
	void    Init(float nWidth, float nHeight);
	void	Reset() ;
	void	OnBegin(float nX, float nY) ;
	void	OnMove(float nX, float nY) ;
	void	OnEnd() ;

	XMFLOAT4 QuatFromBallPoints(XMVECTOR startPoint, XMVECTOR endPoint );
	XMMATRIX GetRotationMatrix() ;
	XMMATRIX GetRotationMatrixIncreament();
	XMFLOAT4 GetRotationQuat() ;
	XMFLOAT4 GetRotationQuatIncreament() ;
	void SetOffset( INT nX, INT nY ) ;
	void SetWindow( float nWidth, float nHeight, float fRadius = 0.9f) ;

private:
	POINT	m_Offset ;	// window offset
	float	m_nWidth ;	// arc ball's window width
	float	m_nHeight ; // arc ball's window height
	float	m_fRadius ;	// arc ball's radius in screen coordinates
	bool	m_bDrag ;	// whether the arc ball is dragged

	XMFLOAT2	m_vCenter ;		// center of arc ball
	XMFLOAT4	m_qDown ;		// quaternion before mouse down
	XMFLOAT4	m_qNow ;		// current quaternion
	XMFLOAT4	m_increament ;	// rotation increment 
	XMVECTOR	m_vDownPt ;		// starting point of arc ball rotate
	XMVECTOR	m_oldPt ;		// old point 
	XMVECTOR	m_vCurrentPt ;	// current point of arc ball rotate
	XMMATRIX	m_mRotation;

	// Convert scree point to arcball point(vector)
	XMVECTOR		ScreenToVector(float fScreenPtX, float fScreenPtY) ;

};

#endif // end __ARCBALL_H__