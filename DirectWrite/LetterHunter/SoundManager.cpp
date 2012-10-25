#include "SoundManager.h"


SoundManager::SoundManager(void):sound_(NULL)
{
	sound_ = new Sound*[NUM_SOUND];
	for(int i = 0; i < NUM_SOUND; ++i)
	{
		sound_[i] = new Sound();
	}

	initialize();
}

SoundManager::~SoundManager(void)
{
	SAFE_DELETE_ARRAY(sound_);
}

void SoundManager::onHit()
{
	playSound(SOUND_HIT_LETTER);
}

void SoundManager::onHitAll()
{
	playSound(SOUND_HIT_ALL);
}

void SoundManager::onShoot()
{
	playSound(SOUND_SEND_BULLET);
}

void SoundManager::initialize()
{
	// The order of the file in this array must same as the order in the SOUND_TYPE enum.
	wchar_t* audioFile[NUM_SOUND] = 
	{
		L"Media/Sound/hit_letter.wav",
		L"Media/Sound/hit_all.wav",
		L"Media/Sound/send_bullet.wav"
	};

	for(int i = 0; i < NUM_SOUND; ++i)
	{
		sound_[i]->initialize(audioFile[i]);
	}
}

void SoundManager::playSound(SOUND_TYPE soundType)
{
	sound_[soundType]->play();
}