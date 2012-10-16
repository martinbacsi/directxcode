#ifndef	__LETTERHUNTER_H__
#define __LETTERHUNTER_H__

#include <vector>

#include "D2D.h"
#include "TextObject.h"
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
	void update();
	void render();
	void handleKeyboardMessage();
	void resize(int width, int height);
	void pause();
	void quit();

private:
	HWND hwnd_;
	int windowWidth;
	int windowHeight;

	// Text related
	vector<TextObject*> textBuffer_;
	int maxTextCount_;
	static const int TEXTCOUNT = 3;

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
	void	resetTextObject(TextObject* textObject);	// This function does not work yet

	// Add new text object to text buffer
	void    addNewTextObject();
};

#endif //end __LETTERHUNTER_H__

