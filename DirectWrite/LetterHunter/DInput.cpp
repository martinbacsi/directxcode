#include "DInput.h"
#include "Utilities.h"

DInput::DInput(void)
{
	m_pDIObject = NULL ;
	Init() ;
}

DInput::~DInput(void)
{
	Release() ;
}

void DInput::Release()
{
	// Release keyboard device
	if (m_pDIKeyboardDevice != NULL)
	{
		m_pDIKeyboardDevice->Unacquire() ;
		SAFE_RELEASE(m_pDIKeyboardDevice);
	}

	// Release mouse device
	if (m_pDIMouseDevice != NULL)
	{
		m_pDIMouseDevice->Unacquire() ;
		SAFE_RELEASE(m_pDIMouseDevice);
	}

	// Release Direct Input object
	SAFE_RELEASE(m_pDIObject);
}

// Initialize Direct input object
bool DInput::Init()
{
	// Create Direct Input object
	HRESULT hr = DirectInput8Create( GetModuleHandle(NULL), 
		DIRECTINPUT_VERSION, 
		IID_IDirectInput8, 
		(void **)&m_pDIObject, 
		NULL) ;

	if (FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr) ;
		return false ;
	}

	// Initialize keyboard
	else if (!InitKeyboard())
	{
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr) ;
	}

	// Initialize mouse
	else if (!InitMouse())
	{
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr) ;
	}

	return true ;
}

// Initialize keyboard device
bool DInput::InitKeyboard()
{
	// Create keyboard device
	HRESULT hr = m_pDIObject->CreateDevice(GUID_SysKeyboard, &m_pDIKeyboardDevice, NULL) ;
	if (FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr) ;
		return false ;
	}

	// Set keyboard data format
	hr = m_pDIKeyboardDevice->SetDataFormat(&c_dfDIKeyboard) ;
	if(FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr) ;
		return false ;
	}

	// Set keyboard cooperate level
	hr = m_pDIKeyboardDevice->SetCooperativeLevel(GetForegroundWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE) ;
	if (FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr) ;
		return false ;
	}

	// Acquire for keyboard input
	hr = m_pDIKeyboardDevice->Acquire() ;
	if (FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr) ;
		return false ;
	}

	return true ;
}

bool DInput::InitMouse()
{
	//DIDEVCAPS MouseCapabilities; //device capabilities

	// Create mouse device
	HRESULT hr = m_pDIObject->CreateDevice(GUID_SysMouse, &m_pDIMouseDevice, NULL) ;
	if(FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr) ;
		return false ;
	}

	// Set mouse data format
	hr = m_pDIMouseDevice->SetDataFormat(&c_dfDIMouse2) ;
	if(FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr) ;
		return false ;
	}

	// Set mouse cooperate level
	hr = m_pDIMouseDevice->SetCooperativeLevel(GetForegroundWindow(),DISCL_BACKGROUND | DISCL_NONEXCLUSIVE) ;
	if(FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr) ;
		return false ;
	}
	
	// Acquire for mouse input
	hr = m_pDIMouseDevice->Acquire() ;
	if(FAILED(hr))
	{
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr) ;
		return false ;
	}

	return true ;
}

// Update keyboard state
void DInput::update()
{
	// Update keyboard
	if (FAILED(m_pDIKeyboardDevice->GetDeviceState(sizeof(m_KeyBuffer), (LPVOID)&m_KeyBuffer)))
	{
		// Keyboard lost, zero out keyboard data structure.
		ZeroMemory(m_KeyBuffer, sizeof(m_KeyBuffer));

		// Try to acquire for next time we poll.
		m_pDIKeyboardDevice->Acquire() ;
	}

	// Update mouse
	if(DIERR_INPUTLOST == m_pDIMouseDevice->GetDeviceState(sizeof(m_MouseState),(LPVOID)&m_MouseState))
	{
		ZeroMemory(&m_MouseState, sizeof(m_MouseState)) ;
		m_pDIMouseDevice->Acquire();
	}
}

// Determine whether a key was pressed
BOOL DInput::keyDown(int key) const
{
	return (m_KeyBuffer[key] & 0x80);
}

const char DInput::getKey() const
{
	const int numKeys = 37;
	const int keys[numKeys] = 
	{
		0x0B, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,		// 0 - 9
		0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24,		// A - J
		0x25, 0x26, 0x32, 0x31, 0x18, 0x19,	0x10, 0x13,	0x1F, 0x14,		// K - T
		0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C,								// U - Z
		0x39															// Space
	};

	const char letters[numKeys + 1] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ "; // The last one is Space

	for(int i = 0; i < numKeys; ++i)
	{
		if(keyDown(keys[i]))
			return letters[i];
	}

	return '-'; // Not a number, not a letter.
}

bool DInput::ButtonDown(int button)
{
	return (m_MouseState.rgbButtons[button] & 0x80) != 0;
}

float DInput::MouseDX() const
{
	return (float)m_MouseState.lX;
}

float DInput::MouseDY() const
{
	return (float)m_MouseState.lY;
}