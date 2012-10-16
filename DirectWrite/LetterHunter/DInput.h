#ifndef __DINPUT_H__
#define __DINPUT_H__

#include <d3dx9.h>
#include <dinput.h>
#include <DxErr.h>

// This Macro wraps the Trace function to show error box
#define ERRORBOX(msg) 

// Define left, right and wheel button 
#define BUTTON_LEFT 0
#define BUTTON_RIGHT 1 
#define BUTTON_WHEEL 2 

class DInput
{
public:
	DInput(void);
	~DInput(void);

	void		update() ;					// Update keyboard
	const char	getKey() const;				// Get the key pressed, we only process numbers(0 - 9) and letters(A - Z)
	bool		ButtonDown(int button) ;	// Is a mouse button down?
	float		MouseDX() const;
	float		MouseDY() const;

private:
	bool Init() ;					// Initialize Direct Input
	BOOL keyDown(int key) const;	// Is a keyboard key pressed?
	bool InitKeyboard() ;			// Initialize keyboard
	bool InitMouse() ;				// Initialize mouse
	void Release() ;

private:
	LPDIRECTINPUT8			m_pDIObject ;
	LPDIRECTINPUTDEVICE8	m_pDIKeyboardDevice ;	// Keyboard device
	LPDIRECTINPUTDEVICE8	m_pDIMouseDevice ;		// Mouse device
	char					m_KeyBuffer[256] ;		// Buffer for keys
	DIMOUSESTATE2			m_MouseState ;			// Mouse state
};

#endif // end __DINPUT_H__
