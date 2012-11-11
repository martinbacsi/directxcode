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
	soundManager_(NULL),
	score_(NULL),
	textBuffer_(NULL),
	currentTextObject_(NULL)
{
	// Initialize Direct2D
	d2d_ = new D2D();
	d2d_->createDeviceIndependentResources();
	d2d_->createDeviceResources(hwnd);

	// Initialize Direct Input
	dinput_ = new DInput();
	soundManager_ = new SoundManager();

	// Initialize score obejct
	score_ = new Score(d2d_->getD2DHwndRenderTarget(), d2d_->getDWriteFactory(), hwnd_);
	score_->setColor(D2D1::ColorF(D2D1::ColorF::Red));

	RECT rect;
	GetWindowRect(hwnd_, &rect);

	int width  = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	setWindowWidth(width);
	setWindowHeight(height);
}

void LetterHunter::release()
{
	SAFE_DELETE(d2d_);
	SAFE_DELETE(dinput_);
	SAFE_DELETE(score_);

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
	// Initizlie text objects
	initializeText();
	initializeBullet();
}

void LetterHunter::update(float timeDelta)
{
	dinput_->update();

	// Get the key pressed
	char hitLetterObject = '-';
	static bool keyUp = false;
	if(dinput_->getKey() != '-')
	{
		if(!keyUp)
		{
			keyUp = true;
			hitLetterObject = dinput_->getKey();
		}
	}
	else
	{
		keyUp = false;
	}

	// 0-9 was magic words
	if(hitLetterObject == '9')	// pause
	{
		setTextSpeedFactor(0);
	}

	if(hitLetterObject == ' ')
	{
		hitAll();
	}

	shootCheck(hitLetterObject);

	// Update text obejcts
	for(unsigned int i = 0; i < textBuffer_.size(); ++i)
	{
		if (textBuffer_[i]->isLive())
		{
			textBuffer_[i]->update(timeDelta);

			// If text object was out of window, set it to dead.
			RECT windowRect = {0, 0, windowWidth, windowHeight};
			if (textBuffer_[i]->outofWindow(windowRect))
			{
				textBuffer_[i]->setLiveState(false);
				if (currentTextObject_ == textBuffer_[i])
				{
					currentTextObject_ = NULL;
				}
			}
		}
		else
		{
			// When a text object was dead, reset a new one
			// The code here need update, otherwise we can not end the game
			resetTextObject(textBuffer_[i]);
		}
	}

	// Update bullet objects
	for(unsigned int i = 0; i < bulletBuffer_.size(); ++i)
	{
		if(bulletBuffer_[i]->isLive())
		{
			// Update bullet, position, live state and so on
			bulletBuffer_[i]->update(timeDelta);

			// Check whether bullet out of window, if true, set it's state to dead
			RECT windowRect = {0, 0, windowWidth, windowHeight};
			if(bulletBuffer_[i]->outofWindow(windowRect))
			{
				bulletBuffer_[i]->setLiveState(false);
				return;
			}

			if (currentTextObject_)
			{
				hitDetect(bulletBuffer_[i], currentTextObject_);
			}
		}
	}
}

void LetterHunter::render(float timeDelta)
{
	// Create device dependent resources
	d2d_->createDeviceResources(hwnd_);

	// Get Hwnd render target
	ID2D1HwndRenderTarget* rendertarget = d2d_->getD2DHwndRenderTarget();

	// Update text object before rendering
	update(timeDelta);

	rendertarget->BeginDraw();

	// Set render target background color to white
	rendertarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	// render background image
	wchar_t*	imageFile = L"./Media/Picture/Fairy_of_the_Water.jpg";
	drawBackgroundImage(imageFile);

	// render text objects
	for(unsigned int i = 0; i < textBuffer_.size(); ++i)
	{
		if(textBuffer_[i]->isLive())
		{
			textBuffer_[i]->render();
		}
	}

	// render bullet objects
	for(unsigned int i = 0; i < bulletBuffer_.size(); ++i)
	{
		if(bulletBuffer_[i]->isLive())
		{
			bulletBuffer_[i]->render();
		}
	}

	// render score
	score_->draw();

	rendertarget->EndDraw();
}

void LetterHunter::resize(int width, int height)
{
	// resize d2d render target
	d2d_->onResize(width, height);

	// Update window size
	setWindowWidth(width); 
	setWindowHeight(height);
}

void LetterHunter::run(float timeDelta)
{
	update(timeDelta);
	render(timeDelta);
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
	ID2D1Factory*			D2DFactory		= d2d_->getD2DFactory();
	ID2D1HwndRenderTarget*	renderTarget	= d2d_->getD2DHwndRenderTarget();
	IDWriteFactory*			DWriteFactory	= d2d_->getDWriteFactory();

	for(int i = 0; i < TEXTCOUNT; ++i)
	{
		// Geneate a random string
		const int strLength = 1;
		wchar_t* strBuffer = new wchar_t[strLength + 1];
		randomString(strBuffer, strLength);

		TextObject* textObj = new TextObject(
			D2DFactory,
			renderTarget,
			DWriteFactory,
			strBuffer,
			100
			);

		SAFE_DELETE(strBuffer);

		// Generate 10 random numbers between 1 and 100
		float a[10] = {0};
		float velocityY = randomFloat(10.0f, 50.0f);
		D2D_VECTOR_2F velocity = {0, velocityY};

		// Set text position
		float windowWidth = (float)getwindowWidth();
		D2D1_RECT_F textBoundRect = textObj->getBoundaryRect();
		float maxRight = windowWidth - (textBoundRect.right -textBoundRect.left);

		float positionX = randomFloat(0, maxRight);
		D2D1_POINT_2F position = {positionX, 0};
		textObj->setPosition(position);

		// Set text velocity
		textObj->setVelocity(velocity);

		D2D1_COLOR_F fillColor = randomColor();
		textObj->setFillColor(fillColor);

		textBuffer_.push_back(textObj);
	}
}

void LetterHunter::initializeBullet()
{
	ID2D1Factory*			D2DFactory		= d2d_->getD2DFactory();
	ID2D1HwndRenderTarget*	renderTarget	= d2d_->getD2DHwndRenderTarget();

	for(int i = 0; i < BULLETCOUNT; ++i)
	{
		Bullet* bullet = new Bullet(D2DFactory, renderTarget);
		bulletBuffer_.push_back(bullet);
	}
}

void LetterHunter::resetTextObject(TextObject* textObject)
{
	// Set text position
	float windowWidth = (float)getwindowWidth();
	D2D1_RECT_F textBoundRect = textObject->getBoundaryRect();
	float maxRight = windowWidth - (textBoundRect.right -textBoundRect.left);

	float positionX = randomFloat(0, maxRight);
	D2D1_POINT_2F position = {positionX, 0};

	// Set text velocity
	float velocityX = 0;
	float velocityY = randomFloat(0.1f, 0.5f);

	D2D_VECTOR_2F velocity = textObject->getVelocity();

	// Create text string
	const int strLength = 3;
	wchar_t* strBuffer = new wchar_t[strLength + 1]; // one more space for '\0'
	randomString(strBuffer, strLength);

	// Create text color
	D2D1_COLOR_F fillColor = randomColor();
	textObject->setFillColor(fillColor);

	// Reset text object
	textObject->reset(strBuffer, position, velocity, fillColor);

	SAFE_DELETE(strBuffer);
}

TextObject* LetterHunter::findTarget(wchar_t hitKey)
{
	float maxBottomofTextObject = 0;
	TextObject* lowestTextObject = NULL;

	for (unsigned int i = 0; i < textBuffer_.size(); ++i)
	{
		if (textBuffer_[i]->isLive())
		{
			BaseLetter* letterOjbect = textBuffer_[i]->getFirstActiveLetterObject();
			if (letterOjbect->getLetter() == hitKey)
			{
				D2D1_RECT_F rect = letterOjbect->getBoundRect();
				if(rect.bottom > maxBottomofTextObject)
				{
					maxBottomofTextObject = rect.bottom;
					lowestTextObject = textBuffer_[i];
				}
			}
		}
	}

	return lowestTextObject;
}

void LetterHunter::shootCheck(wchar_t key)
{
	if (currentTextObject_)
	{
		BaseLetter* letterOjbect = currentTextObject_->getFirstActiveLetterObject();
		if (key == letterOjbect->getLetter())
		{
			// Set the bullet object based on the letter object.
			setBulletObject(letterOjbect);

			// Play sound for the bullet
			soundManager_->onShoot();
		}
		else
		{
			// Warn user where is the target text object is, may be need some highlight mechnism.
		}
	}

	else
	{
		TextObject* targetTextObject = findTarget(key);
		if(targetTextObject)
		{
			currentTextObject_ = targetTextObject;
			BaseLetter* letterOjbect = targetTextObject->getFirstActiveLetterObject();
			setBulletObject(letterOjbect);
			soundManager_->onShoot();
		}
	}
}

void LetterHunter::hitDetect(Bullet* bullet, TextObject* textObject)
{
	// We trate the letter as hit when the bullet pass over the letter, that is the bullet.top < letter.bottom
	D2D1_RECT_F bulletRect = bullet->getBoundRect();

	D2D1_RECT_F textObejctRect = textObject->getBoundaryRect();

	if(bulletRect.top < textObejctRect.bottom)
	{
		// Update score
		score_->add(1);

		textObject->onHit();
		if(!textObject->isLive())
		{
			currentTextObject_ = NULL;
		}
		bullet->onHit();
		soundManager_->onHit();
	}
}

void LetterHunter::hitAll()
{
	currentTextObject_ = NULL;
	for(unsigned int i = 0; i < textBuffer_.size(); ++i)
	{
		textBuffer_[i]->setLiveState(false);
	}

	soundManager_->onHitAll();
}

void LetterHunter::setBulletObject(BaseLetter* letterObject)
{
	// find a invalid bullet
	for(unsigned int i = 0; i < bulletBuffer_.size(); ++i)
	{
		if(!bulletBuffer_[i]->isLive())
		{
			// reset state to true
			bulletBuffer_[i]->setLiveState(true);

			// Set target letter
			wchar_t letter = letterObject->getLetter();
			bulletBuffer_[i]->setTargetLetter(letter);

			// Get the letter position
			D2D1_POINT_2F letterPos = letterObject->getPosition();

			// set bullet position at the bottom of the letter being hit
			D2D1_POINT_2F bulletPos = {letterPos.x, (float)windowHeight};
			bulletBuffer_[i]->setPostion(bulletPos);

			// Set velocity of the bullet, the velocity should based on the distance of the letter and the window bottom
			D2D_VECTOR_2F velocity = {0, -10};
			bulletBuffer_[i]->setVelocity(velocity);

			break;
		}
	}
}

void LetterHunter::addNewTextObject()
{
	TextObject textObj;
	resetTextObject(&textObj);

	textBuffer_.push_back(&textObj);
}

void LetterHunter::setTextSpeedFactor(float speedFactor)
{
	for(unsigned int i = 0; i < textBuffer_.size(); ++i)
	{
		textBuffer_[i]->setLetterSpeedFactor(speedFactor);
	}
}

void LetterHunter::drawBackgroundImage(wchar_t* imageFile)
{
	static ID2D1Bitmap* pBitmap = NULL;
	ID2D1RenderTarget*	renderTarget = d2d_->getD2DHwndRenderTarget();

	static bool isImageLoad = false;
	if(!isImageLoad)
	{
		// Create WIC factory
		IWICImagingFactory*	pWICFactory	= NULL ;

		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory1,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void **>(&pWICFactory)
			) ;

		d2d_->LoadBitmapFromFile(
			renderTarget,
			pWICFactory,
			imageFile,
			0,
			0,
			&pBitmap
			);

		isImageLoad = true;
	}

	RECT rect;
	GetWindowRect(hwnd_, &rect);
	renderTarget->DrawBitmap(
		pBitmap,
		D2D1::Rect(rect.left, rect.top, rect.right, rect.bottom)
		);
}