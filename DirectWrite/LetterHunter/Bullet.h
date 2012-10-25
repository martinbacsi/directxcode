#ifndef __BULLET_H__
#define __BULLET_H__

#include <C:\Program Files\Windows Kits\8.0\Include\um\d2d1.h>

class Bullet
{
public:
	Bullet(void);
	Bullet(ID2D1Factory* pD2DFactory, ID2D1HwndRenderTarget* pRenderTarget);

	~Bullet(void);

	void setVelocity(D2D_VECTOR_2F velocity);
	void setPostion(D2D_POINT_2F pos);
	D2D_POINT_2F getPosition() const;
	void setLiveState(bool liveState);
	bool isLive() const;
	void setTargetLetter(wchar_t letter);
	wchar_t getTargetLetter() const;
	ID2D1TransformedGeometry* getTransformedGeometry() const;

	void getBoundRect(D2D1_RECT_F* rect) const;

	// Determine whether the bullet was out of current window, if true, the 
	// bullet was set to dead.
	bool outofWindow(RECT& windowRect);

	// Hanler for hit
	void onHit();
	void update(float timeDelta);
	void render();

private:
	D2D_VECTOR_2F			velocity_;
	D2D_VECTOR_2F			initPosition_;
	D2D1_MATRIX_3X2_F		matrix_;
	float					totalTime_;
	bool					liveState_;

	// The letter which the bullet aimed to, this is used for hit detection
	// Instead of loop all the letter object on screen, we only check the letter object same 
	// as the targetLetter_
	wchar_t					targetLetter_;

	ID2D1Factory*				pD2DFactory_;
	ID2D1HwndRenderTarget*		pRenderTarget_;
	ID2D1RectangleGeometry*		pRectangleGeometry_;
	ID2D1TransformedGeometry*	pTransformedGeometry_;
	ID2D1SolidColorBrush*		pOutlineBrush_;
	ID2D1SolidColorBrush*		pFillBrush_;
};

#endif // end __BULLET_H__