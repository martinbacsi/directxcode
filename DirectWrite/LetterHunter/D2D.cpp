#include "D2D.h"
#include "Utilities.h"

D2D::D2D(void)
	:D2DFactory_(NULL),
	D2DHwndRenderTarget_(NULL),
	DWriteFactory_(NULL)
{
}


D2D::~D2D(void)
{
	discardDeviceResources();
}

void D2D::createDeviceIndependentResources()
{
	// Create Direct2D Factory
	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&D2DFactory_
		);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create Direct2D factory failed!", L"Error", 0);
		return;
	}

	// Create DirectWrite Factory
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED, 
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&DWriteFactory_)
		);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create DirectWrite factory failed!", L"Error", 0);
		return;
	}
}

void D2D::createDeviceResources(HWND Hwnd)
{
	// Get the client area size
	RECT rc;
	GetClientRect(Hwnd, &rc);

	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

	// Only creaste once
	if(!D2DHwndRenderTarget_)
	{
		// Create render target
		HRESULT hr = D2DFactory_->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(Hwnd, size),
			&D2DHwndRenderTarget_
			);
		if(FAILED(hr))
		{
			MessageBox(Hwnd, L"Create render target failed!", L"Error", 0);
			return;
		}
	}
}

void D2D::discardDeviceResources()
{
	SAFE_RELEASE(DWriteFactory_);
	SAFE_RELEASE(D2DHwndRenderTarget_);
	SAFE_RELEASE(D2DFactory_);
}

void D2D::onResize(UINT32 width, UINT32 height)
{
	if(D2DHwndRenderTarget_)
	{
		D2D1_SIZE_U size = D2D1::SizeU(width, height);
		D2DHwndRenderTarget_->Resize(size);
	}
}

ID2D1Factory* D2D::getD2DFactory() const
{
	return D2DFactory_;
}

ID2D1HwndRenderTarget* D2D::getD2DHwndRenderTarget() const
{
	return D2DHwndRenderTarget_;
}

IDWriteFactory* D2D::getDWriteFactory() const
{
	return DWriteFactory_;
}