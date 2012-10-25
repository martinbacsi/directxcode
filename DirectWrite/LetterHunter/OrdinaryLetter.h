#ifndef __COMMON_LETTER_H__
#define __COMMON_LETTER_H__

#include "BaseLetter.h"
class OrdinaryLetter :
	public BaseLetter
{
public:
	OrdinaryLetter(void);
	OrdinaryLetter(
		ID2D1Factory*			d2dFactory, 
		ID2D1HwndRenderTarget*	rendertarget, 
		IDWriteFactory*			dwriteFactory,
		wchar_t					letter,
		float					fontSize = 100
		);
	virtual ~OrdinaryLetter(void);
};

#endif // __COMMON_LETTER_H__

