#ifndef __D3D9_H__
#define __D3D9_H__

#include <d3dx9.h>
#include <DxErr.h>

#include "Camera.h"
#include "Math.h"

class D3D9
{
public:
	D3D9(void);
	~D3D9(void);

public:
	void InitD3D9(HWND hWnd);
	LPDIRECT3DTEXTURE9 CreateTexture(int texWidth, int texHeight, D3DCOLOR color);
	LPDIRECT3DTEXTURE9 CreateInnerTexture(int texWidth, int texHeight, D3DCOLOR color);
	void ResizeD3DScene(int width, int height);
	HRESULT ResetDevice();
	void ToggleFullScreen();
	void setupLight();
	void setupMatrix();
	void FrameMove();
	Ray CalculatePickingRay(int x, int y);
	D3DXVECTOR3 ScreenToVector3(int x, int y);
	LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	//HWND getWindowHandle() const;
	LPDIRECT3D9 getD3D9() const;
	LPDIRECT3DDEVICE9 getD3DDevice() const;
	D3DPRESENT_PARAMETERS getD3Dpp() const;

	void setBackBufferWidth(int width);
	void setBackBufferHeight(int height);

	void setFullScreen(bool isFullScreen);
	bool getFullScreen() const;

	int	getMaxDisplayWidth() const;
	int getMaxDiplayHeight() const;

	int		getLastWindowWidth() const;
	void	setLastWindowWidth(int windowWidth);

	int		getLastWindowHeight() const;
	void	setLastWindowHeight(int windowHeight);

	// Set d3d display mode Window or full scree
	void setWindowMode(bool isWindowed);

private:
	LPDIRECT3D9				pD3D;			// Direct3D object
	LPDIRECT3DDEVICE9		pd3dDevice;		// D3D9 Device
	D3DPRESENT_PARAMETERS	d3dpp;			// D3D presentation parameters
	bool					isFullScreen;	// Is Game in Full-Screen mode?

	HWND hWnd;				// Handle of game window
	int	lastWindowWidth;	// Last window width
	int	lastWindowHeight;	// Last window height

	int maxDisplayWidth;	// The maximum resolution width
	int maxDisplayHeight;	// The maximum resolution height

private:
	Camera*					camera;			// Model-view camera
};

#endif // end __D3D9_H__