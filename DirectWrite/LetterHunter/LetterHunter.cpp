#include "LetterHunter.h"
#include "Utilities.h"
#include "windows.h"

LetterHunter::LetterHunter(void)
{
}

LetterHunter::LetterHunter(HWND hwnd, int maxTextCount)
	:hwnd_(hwnd),
	maxTextCount_(100),
	d2d_(NULL),
	dinput_(NULL),
	textBuffer_(NULL)
{
	// Initialize Direct2D
	d2d_ = new D2D();
	d2d_->createDeviceIndependentResources();
	d2d_->createDeviceResources(hwnd);

	// Initialize Direct Input
	dinput_ = new DInput();
}

void LetterHunter::release()
{
	SAFE_DELETE(d2d_);
	SAFE_DELETE(dinput_);

	for(vector<TextObject*>::iterator itor = textBuffer_.begin(); itor != textBuffer_.end(); ++itor)
	{
		SAFE_DELETE(*itor);
	}
}

LetterHunter::~LetterHunter(void)
{
	release();
}

void LetterHunter::initialize()
{
	// Get screen resolution
	const HWND hDesktop = GetDesktopWindow();

	RECT desktopRect;
	GetWindowRect(hDesktop, &desktopRect);

	int width  = desktopRect.right;
	int height = desktopRect.bottom;

	setWindowWidth(windowWidth);
	setWindowHeight(windowHeight);

	// Initizlie text objects
	initializeText();
}

void LetterHunter::update()
{
	for(vector<TextObject*>::iterator itor = textBuffer_.begin(); itor != textBuffer_.end(); ++itor)
	{
		if((*itor)->getLiveState() == true)
		{
			(*itor)->update();
		}
		else
		{
			resetTextObject(*itor);
		}
	}
}

void LetterHunter::render()
{
	// Create device dependent resources
	d2d_->createDeviceResources(hwnd_);

	// Get Hwnd render target
	ID2D1HwndRenderTarget* rendertarget = d2d_->getD2DHwndRenderTarget();

	// Update text object before rendering
	update();

	rendertarget->BeginDraw();

	// Set render target background color to white
	rendertarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	// render text objects
	for(vector<TextObject*>::const_iterator itor = textBuffer_.begin(); itor != textBuffer_.end(); ++itor)
	{
		if((*itor)->getLiveState() == true)
		{
			(*itor)->render();
		}
	}

	rendertarget->EndDraw();
}

void LetterHunter::handleKeyboardMessage()
{
	dinput_->update();

	// Get the key pressed
	char hitLetter = dinput_->getKey();

	// Loop all the textObject to see if any of them match the hit letter
	for(int i = 0; i < textBuffer_.size(); ++i)
	{
		// Only test with live object
		if(textBuffer_[i]->getLiveState() == true)
		{
			// Get the active index in textObject
			int activeIndex = textBuffer_[i]->getActiveIndex();

			// Get the active letter in textObject
			char currentLetter = textBuffer_[i]->getText()[activeIndex];
			
			// If the active letter match the hit letter, update the textObject
			if(currentLetter == hitLetter)
			{
				// Change the color of the hit letter
				textBuffer_[i]->setFillColorRange(activeIndex, 1, D2D1::ColorF(D2D1::ColorF::White));
				textBuffer_[i]->setFillColorRange(activeIndex, 1, D2D1::ColorF(D2D1::ColorF::Black));

				// Update active index of text
				textBuffer_[i]->setActiveIndex(activeIndex + 1);

				// If all the letters in the text was hit, we treat the text as dead
				if(activeIndex + 1 == textBuffer_[i]->getTextLength())
				{
					textBuffer_[i]->setLiveState(false);
				}
			}
		}
	}
}

void LetterHunter::resize(int width, int height)
{
	d2d_->onResize(width, height);
}

int LetterHunter::getwindowWidth() const
{
	return windowWidth;
}

void LetterHunter::setWindowWidth(int width)
{
	windowWidth = width;
}

int LetterHunter::getWindowHeight() const
{
	return windowHeight;
}

void LetterHunter::setWindowHeight(int height)
{
	windowHeight = height;
}

void LetterHunter::initializeText()
{
	wchar_t* textBuffer[TEXTCOUNT] = {
		L"ABC",
		L"B",
		L"C",
		/*L"D",
		L"G",
		L"F",
		L"G",
		L"H",
		L"I",
		L"J"*/
	};

	ID2D1Factory*			D2DFactory		= d2d_->getD2DFactory();
	ID2D1HwndRenderTarget*	renderTarget	= d2d_->getD2DHwndRenderTarget();
	IDWriteFactory*			DWriteFactory	= d2d_->getDWriteFactory();

	for(int i = 0; i < TEXTCOUNT; ++i)
	{
		TextObject* textObj = new TextObject(
			D2DFactory,
			renderTarget,
			DWriteFactory,
			textBuffer[i],
			D2D1::ColorF(D2D1::ColorF::Red),
			D2D1::ColorF(D2D1::ColorF::Black),
			2.0f,
			200
			);

		// Generate 10 random numbers between 1 and 100
		float a[10] = {0};
		a[i] = randomFloat(0.1, 0.5);

		// Set text position
		textObj->setPosition(i * 200.0f, 0.0f);

		// Set text velocity
		textObj->setVelocity(0, a[i]);

		// Set text fill in color
		textObj->setFillColor(D2D1::ColorF(D2D1::ColorF::Green));

		// Set text outline color
		textObj->setOutlineColor(D2D1::ColorF(D2D1::ColorF::Red));

		// Set outline width
		textObj->setOutlineWidth(1.0f);

		textBuffer_.push_back(textObj);
	}
}

// ===========================================================
// This function need refactoring, we should call the constructor to rebuild a text, not set everything for it.
// ===========================================================
void LetterHunter::resetTextObject(TextObject* textObject)
{
	textObject->setLiveState(true);
	textObject->setVelocity(0, 0.1f);
	textObject->setActiveIndex(0);  
	textObject->setPosition(500, 0);
}