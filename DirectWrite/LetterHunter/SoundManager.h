#ifndef __SOUND_MANAGER_H__
#define __SOUND_MANAGER_H__

#include "Sound.h"

class SoundManager
{
	enum SOUND_TYPE
	{
		SOUND_HIT_LETTER  = 0x0,
		SOUND_HIT_ALL	  = 0x1,
		SOUND_SEND_BULLET = 0x2,
	};

public:
	SoundManager(void);
	~SoundManager(void);

	// Play sound when letter was hit
	void onHit();

	// Play sound when hit all letters
	void onHitAll();

	// Play sound when send a bullet
	void onShoot();

private:
	void initialize();
	void playSound(SOUND_TYPE soundType);

	Sound**	sound_;
	static const int NUM_SOUND = 3;
};

#endif // end __SOUND_MANAGER_H__

