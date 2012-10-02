#ifndef __D2D_H__
#define __D2D_H_

#include <d2d1.h>
#include <dwrite.h>

class D2D
{
public:
	D2D(void);
	~D2D(void);

public:
	void createDeviceIndependentResources();
	void createDeviceResources(HWND Hwnd);
	void discardDeviceResources();

	void onResize(UINT32 width, UINT32 height);

	ID2D1Factory*			getD2DFactory() const;
	ID2D1HwndRenderTarget*	getD2DHwndRenderTarget() const;
	IDWriteFactory*			getDWriteFactory() const;

private:
	ID2D1Factory*			D2DFactory_;
	ID2D1HwndRenderTarget*	D2DHwndRenderTarget_;
	IDWriteFactory*			DWriteFactory_;
};

#endif // end __D2D_H_
