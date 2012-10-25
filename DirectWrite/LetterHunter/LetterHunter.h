#ifndef	__LETTERHUNTER_H__
#define __LETTERHUNTER_H__

#include <vector>

#include "D2D.h"
#include "TextObject.h"
#include "Bullet.h"
#include "DInput.h"
#include "SoundManager.h"

using namespace std;

class LetterHunter
{
public:
	LetterHunter(void);
	LetterHunter(HWND hwnd, int maxTextCount);
	virtual ~LetterHunter(void);

	void initialize();
	void release();
	void update(float timeDelta);
	void render(float timeDelta);
	void resize(int width, int height);
	void run(float timeDelta);
	void pause();
	void quit();

private:
	HWND hwnd_;
	int windowWidth;
	int windowHeight;

	// Text related
	vector<TextObject*> textBuffer_;

	// The curent text object which is being hit, the keyboard messange handler 
	// will only check this object when recieve keyboard message
	TextObject*		currentTextObject_;

	// Bullet buffer, include dead bullet
	vector<Bullet*> bulletBuffer_;

	int maxTextCount_;
	static const int TEXTCOUNT = 10;
	static const int BULLETCOUNT = 10;

	D2D*	d2d_;
	DInput*	dinput_;
	SoundManager* soundManager_;

	// Level of the game, this decides the moving speed of the letter, we compute the letter position as below
	// [ newPosition = velocity * time * gameLevel ], when level raise up, the moving speed also becames fast.
	int gameLevel_;

	// the total score of current user, level will increase when score match a given value(1000 for instance).
	int gameScore_;

private:
	int		getwindowWidth() const;
	void	setWindowWidth(int width);

	int		getWindowHeight() const;
	void	setWindowHeight(int height);

	void	initializeText();
	void	initializeBullet();
	void	setBulletObject(BaseLetter* letter);
	void	resetTextObject(TextObject* textObject);	// This function does not work yet
	
	// If we have multiple text objects match current key hit, we use the one
	// closest to the bottom of the screen as our target.
	TextObject*	findTarget(wchar_t hitKey);

	// Determine whether the key pressed match a letter of the given text on screen.
	void	shootCheck(wchar_t key);

	// Detect whether a letter was hit by a bullet
	void	hitDetect(Bullet* bullet, TextObject* textObject); 

	// destroy everything on the screen.
	void	hitAll();

	// Add new text object to text buffer
	void    addNewTextObject();

	void    setTextSpeedFactor(float speedFactor);
};

#endif //end __LETTERHUNTER_H__

