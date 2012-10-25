#include "OrdinaryLetter.h"


OrdinaryLetter::OrdinaryLetter(void)
{
}

OrdinaryLetter::OrdinaryLetter(
	ID2D1Factory*			d2dFactory, 
	ID2D1HwndRenderTarget*	rendertarget, 
	IDWriteFactory*			dwriteFactory,
	wchar_t					letter,
	float					fontSize
	):BaseLetter(d2dFactory, rendertarget, dwriteFactory, letter, fontSize)
{
}


OrdinaryLetter::~OrdinaryLetter(void)
{
}
