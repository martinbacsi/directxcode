#ifndef __TEXT_OBJECT_H__
#define __TEXT_OBJECT_H__

#include "OrdinaryLetter.h"

class TextObject
{
public:
	TextObject(void);
	TextObject(
		ID2D1Factory*			d2dFactory, 
		ID2D1HwndRenderTarget*	rendertarget, 
		IDWriteFactory*			dwriteFactory,
		wchar_t*		text, 
		float			fontSize		= 100,
		D2D1_COLOR_F	fillColor		= D2D1::ColorF(D2D1::ColorF::Black),
		D2D1_COLOR_F	outlineColor	= D2D1::ColorF(D2D1::ColorF::White),
		D2D1_COLOR_F	boundaryFillColor_ = D2D1::ColorF(0x007ACC),
		D2D1_COLOR_F    boundaryOutlineColor_ = D2D1::ColorF(0x732794),
		float			outlineWidth	= 2.0f
		);
	~TextObject(void);

	void update(float timeDelta);
	void render();

	// Reset a text object
	void reset(wchar_t* text, D2D1_POINT_2F& position, D2D_VECTOR_2F& velocity, D2D1_COLOR_F& fillColor);

	//Set text fill in color for a given range
	void setFillColorRange(int startIndex, int length, D2D1_COLOR_F& color);

	// Set text fill in color
	void setFillColor(D2D1_COLOR_F& color);

	// Set text outline color for a given range
	void setOutlineColorRange(int startIndex, int length, D2D1_COLOR_F& color);

	// SEt text outline color
	void setOutlineColor(D2D1_COLOR_F&	color);

	void setBoundaryOutlineColor(D2D1_COLOR_F& color);
	D2D1_COLOR_F getBoundaryOutlineColor() const;

	void setBoundaryFillColor(D2D1_COLOR_F& color);
	D2D1_COLOR_F getBoundaryFillColor() const;

	// Set text outline width for the given range
	void setOutlineWidthRange(int startIndex, int length, float width);

	// Set outline width of the entire text
	void setOutlineWidth(float width);

	// Set live state of the text for a given range
	void setLiveStateRange(int startIndex, int length, bool state);

	// Set live state of the entire text
	void setLiveState(bool state);

	// Get the active index in the text
	bool isLive() const;

	// Set visible state of the text for a given range
	void setVisibleState(int startIndex, int length, bool state);

	// Set the velocity of the text for a given range
	void setVelocityRange(int startIndex, int length, D2D_VECTOR_2F& velocity);

	// Set the velocity of the entire text
	void setVelocity(D2D_VECTOR_2F& velocity);
	D2D_VECTOR_2F getVelocity() const;

	// Set the position of the entire text
	void setPostionRange(int startIndex, int length, D2D1_POINT_2F& position);
	void setPosition(D2D1_POINT_2F& position);
	void setTransfrom(D2D1_MATRIX_3X2_F& matrix);

	// Set the active index of the text
	void setActiveIndex(int index);

	// Get the active index of the text
	int  getActiveIndex() const;

	// Get the text of the text Object
	wchar_t* getText() const;
	int		 getTextLength() const;

	// Get the first hitable letter in the text.
	BaseLetter* getFirstActiveLetterObject() const;
	
	// Handler when text was hit
	void		onHit();
	D2D1_RECT_F	getBoundaryRect() const;

	void		drawBoundary() const;
	bool		outofWindow(RECT&	windowRect);

	void setLetterSpeedFactor(float speedFactor);

private:
	// Create text string
	void createText(
		wchar_t* textString, 
		ID2D1Factory*			d2dFactory, 
		ID2D1HwndRenderTarget*	rendertarget, 
		IDWriteFactory*			dwriteFactory,
		float					fontSize = 100.0f
		);	

	// Get the letter object by index, each letter in the text was represent by a letter obejct.
	BaseLetter* getLetter(int index) const;

private:
	ID2D1Factory*			d2dFactory_;
	ID2D1HwndRenderTarget*	rendertarget_;
	IDWriteFactory*			dwriteFactory_;

	ID2D1SolidColorBrush*   boundaryOutlineBrush_;
	D2D1_COLOR_F			boundaryOutlineColor_;

	ID2D1SolidColorBrush*   boundaryFillBrush_;
	D2D1_COLOR_F			boundaryFillColor_;

	wchar_t*	text_;			// text of the text object
	int			length_;		// length of the text
	float		fontSize_;		// Font size
	float		letterSpace_;	// The space between two adjacent letters

	// The active index specify which letter is currently hitable in the text, when
	// a letter was hit, the active index move to the next letter.
	int			activeIndex_;	
	bool		isLive_;		// Is text live?

	BaseLetter** letterBuffer_;	// Buffer to hold the letter objects
};

#endif // end __TEXT_OBJECT_H__ 