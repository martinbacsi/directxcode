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

HRESULT D2D::LoadBitmapFromFile(
						   ID2D1RenderTarget *pRenderTarget,
						   IWICImagingFactory *pIWICFactory,
						   PCWSTR uri,
						   UINT destinationWidth,
						   UINT destinationHeight,
						   ID2D1Bitmap **ppBitmap
						   )
{
	HRESULT hr = S_OK;

	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
		);
	if (SUCCEEDED(hr))
	{

		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}

	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		// If a new width or height was specified, create an
		// IWICBitmapScaler and use it to resize the image.
		if (destinationWidth != 0 || destinationHeight != 0)
		{
			UINT originalWidth, originalHeight;
			hr = pSource->GetSize(&originalWidth, &originalHeight);
			if (SUCCEEDED(hr))
			{
				if (destinationWidth == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
					destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
				}
				else if (destinationHeight == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
					destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
				}

				hr = pIWICFactory->CreateBitmapScaler(&pScaler);
				if (SUCCEEDED(hr))
				{
					hr = pScaler->Initialize(
						pSource,
						destinationWidth,
						destinationHeight,
						WICBitmapInterpolationModeCubic
						);
				}
				if (SUCCEEDED(hr))
				{
					hr = pConverter->Initialize(
						pScaler,
						GUID_WICPixelFormat32bppPBGRA,
						WICBitmapDitherTypeNone,
						NULL,
						0.f,
						WICBitmapPaletteTypeMedianCut
						);
				}
			}
		}
		else // Don't scale the image.
		{
			hr = pConverter->Initialize(
				pSource,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0.f,
				WICBitmapPaletteTypeMedianCut
				);
		}
	}
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
			);
	}

	SAFE_RELEASE(pDecoder);
	SAFE_RELEASE(pSource);
	SAFE_RELEASE(pStream);
	SAFE_RELEASE(pConverter);
	SAFE_RELEASE(pScaler);

	return hr;
}