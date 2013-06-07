#ifndef __ARCBALL_H__
#define __ARCBALL_H__

#include "d3dx9.h"

class ArcBall
{
public:
	ArcBall(void);
	~ArcBall(void);

public:
	void Reset() ;
	void OnBegin(int nX, int nY) ;
	void OnMove(int nX, int nY) ;
	void OnEnd() ;

	D3DXQUATERNION QuatFromBallPoints(const D3DXVECTOR3& vFrom, const D3DXVECTOR3& vTo);
	const D3DXMATRIX* GetRotationMatrix() ;
	D3DXQUATERNION GetRotationQuatIncreament() ;
	void SetWindow(int nWidth, int nHeight, float fRadius = 0.9f) ;

private:
	int		m_nWidth ;	// arc ball's window width
	int		m_nHeight ; // arc ball's window height
	float	m_fRadius ;	// arc ball's radius in screen coordinates
	bool	m_bDrag ;	// whether the arc ball is dragged

	D3DXVECTOR2		m_vCenter ;		// center of arc ball
	D3DXQUATERNION	m_qDown ;		// quaternion before mouse down
	D3DXQUATERNION	m_qNow ;		// current quaternion
	D3DXQUATERNION	m_increament ;	// rotation increment 
	D3DXVECTOR3		m_vDownPt ;		// starting point of arc ball rotate
	D3DXVECTOR3		m_oldPt ;		// old point 
	D3DXVECTOR3		m_vCurrentPt ;	// current point of arc ball rotate
	D3DXMATRIXA16	m_mRotation;

	// Convert scree point to arcball point(vector)
	D3DXVECTOR3	ScreenToVector(float fScreenPtX, float fScreenPtY) ;

};

#endif // end __ARCBALL_H__