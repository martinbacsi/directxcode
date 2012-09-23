#include "Text.h"

#define SAFE_RELEASE(P) if(P){P->Release() ; P = NULL ;}

Text::Text(
		ID2D1Factory* d2dFactory, 
		ID2D1HwndRenderTarget* rendertarget, 
		IDWriteFactory*	dwriteFactory,
		wchar_t* fontText, 
		D2D1_COLOR_F fillColor,
		D2D1_COLOR_F outlineColor,
		float width,
		int fontSize
		)
		:
	pD2DFactory(d2dFactory),
	pRenderTarget(rendertarget),
	pDWriteFactory(dwriteFactory),
	text(fontText),
	outlineWidth(width),
	isVisible(true), 
	pFontFile(NULL), 

	matrix(D2D1::Matrix3x2F::Identity()),
	pPathGeometry(NULL),
	pTransformedGeometry(NULL)
{
	
	fontSize = fontSize;
	this->fillColor = fillColor;
	this->outlineColor = outlineColor;

	// Create font file reference
	const WCHAR* filePath = L"C:/Windows/Fonts/ariblk.TTF";
	HRESULT hr = pDWriteFactory->CreateFontFileReference(
		filePath,
		NULL,
		&pFontFile
		);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create font file reference failed!", L"Error", 0);
		return;
	}

	// Create font face
	IDWriteFontFile* fontFileArray[] = { pFontFile };
	pDWriteFactory->CreateFontFace(
		DWRITE_FONT_FACE_TYPE_TRUETYPE,
		1,
		fontFileArray,
		0,
		DWRITE_FONT_SIMULATIONS_NONE,
		&pFontFace
		);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create font file face failed!", L"Error", 0);
		return;
	}

	// Create fill brush
	hr = pRenderTarget->CreateSolidColorBrush(fillColor, &pFillBrush);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create fill brush failed!", L"Error", 0);
		return;
	}

	// Create outline brush
	hr = pRenderTarget->CreateSolidColorBrush(outlineColor, &pOutlineBrush);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create outline brush failed!", L"Error", 0);
		return;
	}

	// Get text length
	UINT32 textLength = (UINT32)wcslen(text);

	UINT32* pCodePoints		= new UINT32[textLength];
	ZeroMemory(pCodePoints, sizeof(UINT32) * textLength);

	UINT16*	pGlyphIndices	= new UINT16[textLength];
	ZeroMemory(pGlyphIndices, sizeof(UINT16) * textLength);

	for(int i = 0; i < textLength; ++i)
	{
		pCodePoints[i] = text[i];
	}

	// Get glyph indices
	hr =pFontFace->GetGlyphIndicesW(
		pCodePoints,
		textLength,
		pGlyphIndices
		);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Get glyph indices failed!", L"Error", 0);
		return;
	}

	// Create path geometry
	hr = pD2DFactory->CreatePathGeometry(&pPathGeometry);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Create path geometry failed!", L"Error", 0);
		return;
	}

	// Open sink
	hr = pPathGeometry->Open(&pGeometrySink);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Open geometry sink failed!", L"Error", 0);
		return;
	}

	// Get glyph run outline
	hr = pFontFace->GetGlyphRunOutline(
		fontSize,				// font size
		pGlyphIndices,
		NULL,
		NULL,
		textLength,
		FALSE,
		FALSE,
		pGeometrySink
		);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"Get glyph run outline failed!", L"Error", 0);
		return;
	}

	// Close sink
	pGeometrySink->Close();

	if(pCodePoints)
	{
		delete [] pCodePoints;
		pCodePoints = NULL;
	}

	if(pGlyphIndices)
	{
		delete [] pGlyphIndices;
		pGlyphIndices = NULL;
	}
}

Text::~Text(void)
{
	SAFE_RELEASE(pTransformedGeometry);
	SAFE_RELEASE(pGeometrySink);
	SAFE_RELEASE(pPathGeometry);
	SAFE_RELEASE(pFontFile);
	SAFE_RELEASE(pFontFace);
	SAFE_RELEASE(pDWriteFactory);
	SAFE_RELEASE(pRenderTarget);
	SAFE_RELEASE(pFillBrush);
	SAFE_RELEASE(pOutlineBrush);
	SAFE_RELEASE(pD2DFactory);
}

bool Text::getVisible() const
{
	return isVisible;
}

void Text::setVisible(bool visibleFlag)
{
	isVisible = visibleFlag;
}

wchar_t* Text::getText() const
{
	return text;
}

void Text::setText(wchar_t* text)
{
	this->text = text;
}

D2D1_COLOR_F Text::getFillColor() const
{
	return fillColor;
}

void Text::setFillColor(D2D1_COLOR_F color)
{
	fillColor = color;
	pFillBrush->SetColor(fillColor);
}

D2D1_COLOR_F Text::getOutlineColor() const
{
	return outlineColor;
}

void Text::setOutlineColor(D2D1_COLOR_F color)
{
	outlineColor = color;
	pOutlineBrush->SetColor(outlineColor);
}

float	Text::getoutlineWidth() const
{
	return outlineWidth;
}

void Text::setOutlineWidth(float width)
{
	outlineWidth = width;
}

int Text::getSize() const
{
	return fontSize;
}

void Text::setSize(int size)
{
	fontSize = size;
}

D2D1_MATRIX_3X2_F Text::getTransformMatrix() const
{
	return matrix;
}

void Text::setTransformMatrix(D2D1_MATRIX_3X2_F& matrix)
{
	this->matrix = matrix;
}

void Text::setTransform(D2D1_MATRIX_3X2_F matrix)
{
	pD2DFactory->CreateTransformedGeometry(
		pPathGeometry,
		matrix,
		&pTransformedGeometry
		);
}

void Text::render()
{
	// Draw outline
	pRenderTarget->DrawGeometry(pTransformedGeometry, pOutlineBrush, outlineWidth);

	// Fill inner area
	pRenderTarget->FillGeometry(pTransformedGeometry, pFillBrush);
}