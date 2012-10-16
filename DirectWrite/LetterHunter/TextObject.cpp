#include "TextObject.h"
#include "Utilities.h"

TextObject::TextObject(void)
{
}

TextObject::TextObject(
	ID2D1Factory*			d2dFactory, 
	ID2D1HwndRenderTarget*	rendertarget, 
	IDWriteFactory*			dwriteFactory,
	wchar_t*		text, 
	D2D1_COLOR_F	fillColor,
	D2D1_COLOR_F	outlineColor,
	float			outlineWidth,
	int				fontSize
	)
	:
	d2dFactory_(d2dFactory),
	rendertarget_(rendertarget),
	dwriteFactory_(dwriteFactory),
	text_(NULL),
	fontSize_(fontSize),
	activeIndex_(0),
	isLive_(true),
    letterBuffer_(NULL)
{
	createText(text, d2dFactory_, rendertarget_, dwriteFactory_);
}

void TextObject::createText(
	wchar_t* textString, 
	ID2D1Factory* d2dFactory, 
	ID2D1HwndRenderTarget*	rendertarget, 
	IDWriteFactory*			dwriteFactory
	)
{
	
	// Initialize each letter in the text to a Letter object
	length_ = wcslen(textString);

	text_ = new wchar_t[length_ + 1]; // Add one more space for '\0'
	for(int i = 0; i < length_; ++i)
	{
		text_[i] = textString[i];
	}

	text_[length_] = '\0';

	letterBuffer_ = new Letter*[length_];
	
	// top left coordinates of current letter
	float currentX = 0; 
	float currentY = 0;
	letterSpace_ = fontSize_ / 10.0f; // space between adjacent letters.

	for(int i = 0; i < length_; ++i)
	{
		letterBuffer_[i] = new Letter(
			d2dFactory,
			rendertarget,
			dwriteFactory,
			textString[i]		// here is a string instead
			);
		
		D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Translation(currentX, currentY);
		letterBuffer_[i]->translate(currentX, currentY);

		// Get the boundary of the current letter
		D2D1_RECT_F rect;
		letterBuffer_[i]->getBound(&rect);

		// Move the next letter horizontally, so that they were not overlapped.
		currentX += (rect.right - rect.left) + letterSpace_;
		currentY = 0;
	}
}

TextObject::~TextObject(void)
{
	SAFE_DELETE(text_);
	//SAFE_DELETE_ARRAY(letterBuffer_);
}

void TextObject::reset(wchar_t* text, float x, float y, float velocityX, float velocityY, D2D1_COLOR_F& fillColor)
{
	activeIndex_	= 0;
	isLive_			= true;
	SAFE_DELETE_ARRAY(letterBuffer_);
	SAFE_DELETE(text_);

	createText(text, d2dFactory_, rendertarget_, dwriteFactory_);

	setPosition(x, y);

	setVelocity(velocityX, velocityY);

	setFillColor(fillColor);
}

void TextObject::update()
{
	for(int i = 0; i < length_; ++i)
	{
		letterBuffer_[i]->update();
	}
}

void TextObject::render()
{
	for(int i = 0; i < length_; ++i)
	{
		letterBuffer_[i]->render();
	}
}

void TextObject::setFillColorRange(int startIndex, int length, D2D1_COLOR_F& color)
{
	for(int i = startIndex; i < startIndex + length; ++i)
	{
		letterBuffer_[i]->setFillColor(color);
	}
}

void TextObject::setFillColor(D2D1_COLOR_F& color)
{
	setFillColorRange(0, length_, color);
}

void TextObject::setOutlineColorRange(int startIndex, int length, D2D1_COLOR_F& color)
{
	for(int i = startIndex; i < startIndex + length; ++i)
	{
		letterBuffer_[i]->setOutlineColor(color);
	}
}

void TextObject::setOutlineColor(D2D1_COLOR_F& color)
{
	setOutlineColorRange(0, length_, color);
}

void TextObject::setOutlineWidthRange(int startIndex, int length, float width)
{
	for(int i = 0; i < length_; ++i)
	{
		letterBuffer_[i]->setOutlineWidth(width);
	}
}

void TextObject::setOutlineWidth(float width)
{
	setOutlineWidthRange(0, length_, width);
}

void TextObject::setLiveStateRange(int startIndex, int length, bool state)
{
	for(int i = startIndex; i < startIndex + length; ++i)
	{
		letterBuffer_[i]->setLiveState(state);
	}
}

void TextObject::setLiveState(bool state)
{
	isLive_ = state;
}

bool TextObject::getLiveState() const
{
	return isLive_;
}

void TextObject::setVelocityRange(int startIndex, int length, float x, float y)
{
	for(int i = startIndex; i < startIndex + length_; ++i)
	{
		letterBuffer_[i]->setVelocity(x, y);
	}
}

void TextObject::setVelocity(float x, float y)
{
	setVelocityRange(0, length_, x, y);
}

void TextObject::setPostionRange(int startIndex, int length, float x, float y)
{
	float currentX = x;
	float currentY = y;

	// the boundary rectangle of current letter in text
	D2D1_RECT_F rect;

	for(int i = startIndex; i < length; ++i)
	{
		// Set the position for current letter
		letterBuffer_[i]->setPosition(currentX, currentY);

		// Get the boundary of current letter
		letterBuffer_[i]->getBound(&rect);

		// Calculate the position for next letter, since letters in a string are horizantaly aligned, so 
		// each letter has the same y-coordinate, but has an increament of the x-coordinate.
		currentX += (rect.right - rect.left) + letterSpace_;
	}
}

void TextObject::setPosition(float x, float y)
{
	setPostionRange(0, length_, x, y);
}

void TextObject::setTransfrom(D2D1_MATRIX_3X2_F& matrix)
{
	for(int i = 0; i < length_; ++i)
	{
		letterBuffer_[i]->setTransform(matrix);
	}
}

void TextObject::setActiveIndex(int index)
{
	activeIndex_ = index;
}

int TextObject::getActiveIndex() const
{
	return activeIndex_;
}

wchar_t* TextObject::getText() const
{
	return text_;
}

int TextObject::getTextLength() const
{
	return length_;
}

void TextObject::setLetterSpeedFactor(float speedFactor)
{
	for(int i = 0; i < length_; ++i)
	{
		letterBuffer_[i]->setSpeedFactor(speedFactor);
	}
}