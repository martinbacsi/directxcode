#include "Bullet.h"


Bullet::Bullet(void)
	:pD2DFactory_(NULL),
	pRenderTarget_(NULL),
	pRectangleGeometry_(NULL),
	pTransformedGeometry_(NULL),
	pOutlineBrush_(NULL),
	pFillBrush_(NULL),
	matrix_(D2D1::Matrix3x2F::Identity()),
	liveState_(false),
	totalTime_(0)
{
	// How to initialize struction in initilize list?
	velocity_.x = 0;
	velocity_.y = 0;
	initPosition_.x = 0;
	initPosition_.y = 0;
}

Bullet::Bullet(ID2D1Factory* pD2DFactory, ID2D1HwndRenderTarget* pRenderTarget)
	:pRectangleGeometry_(NULL),
	pTransformedGeometry_(NULL),
	pOutlineBrush_(NULL),
	pFillBrush_(NULL),
	matrix_(D2D1::Matrix3x2F::Identity()),
	liveState_(false),
	totalTime_(0)
{
	pD2DFactory_ = pD2DFactory;
	pRenderTarget_ = pRenderTarget;

	// Create geometry
	D2D1_RECT_F rect = D2D1::RectF(0, 0, 50, 50);
	pD2DFactory_->CreateRectangleGeometry(rect, &pRectangleGeometry_);

	// Create outline brush
	pRenderTarget_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &pOutlineBrush_);

	// Create fill in brush
	pRenderTarget_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &pFillBrush_);
}

void Bullet::setVelocity(D2D_VECTOR_2F velocity)
{
	velocity_ = velocity;
}

void Bullet::setPostion(D2D_POINT_2F pos)
{
	matrix_._31 = pos.x;
	matrix_._32 = pos.y;
}

D2D_POINT_2F Bullet::getPosition() const
{
	D2D_POINT_2F pos = {matrix_._31, matrix_._32};
	return pos;
}

void Bullet::setLiveState(bool liveState)
{
	liveState_ = liveState;
}

bool Bullet::isLive() const
{
	return liveState_;
}

void Bullet::setTargetLetter(wchar_t letter)
{
	targetLetter_ = letter;
}

wchar_t Bullet::getTargetLetter() const
{
	return targetLetter_;
}

ID2D1TransformedGeometry* Bullet::getTransformedGeometry() const
{
	return pTransformedGeometry_;
}

void Bullet::getBoundRect(D2D1_RECT_F* rect) const
{
	pTransformedGeometry_->GetBounds(D2D1::Matrix3x2F::Identity(), rect);
}

bool Bullet::outofWindow(RECT& windowRect)
{
	// Get boundary rectangle of current letter
	D2D1_RECT_F rect;
	getBoundRect(&rect);

	// Check whether letter was out of window
	if( rect.bottom < windowRect.top 
		|| rect.top > windowRect.bottom
		|| rect.right < windowRect.left
		|| rect.left > windowRect.right )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Bullet::onHit()
{
	setLiveState(false);
}

void Bullet::update(float timeDelta)
{
	// Fixed speed rate
	timeDelta = 10.0f;

	// Calculate the new position
	D2D1_POINT_2F currentPos = getPosition();
	float newX = currentPos.x + velocity_.x * timeDelta;
	float newY = currentPos.y + velocity_.y * timeDelta;

	matrix_ = D2D1::Matrix3x2F::Translation(newX, newY);
	pD2DFactory_->CreateTransformedGeometry(
		pRectangleGeometry_, 
		matrix_,
		&pTransformedGeometry_
		);
}

void Bullet::render()
{
	pRenderTarget_->DrawGeometry(pTransformedGeometry_, pOutlineBrush_);
	pRenderTarget_->FillGeometry(pTransformedGeometry_, pFillBrush_);
}

Bullet::~Bullet(void)
{
}
