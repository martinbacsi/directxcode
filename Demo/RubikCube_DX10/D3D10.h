#ifndef __D3D9_H__
#define __D3D9_H__

#include <d3dx10.h>
#include <DxErr.h>

#include "Camera.h"
#include "Math.h"

class D3D10
{
public:
	D3D10(void);
	~D3D10(void);

public:
	void InitD3D10(HWND hWnd);
	ID3D10Texture2D* CreateTexture(int texWidth, int texHeight, D3DCOLOR color);
	ID3D10Texture2D* CreateInnerTexture(int texWidth, int texHeight, D3DCOLOR color);
	void ResizeD3DScene(int width, int height);
	void ToggleFullScreen();
	void SetupLight();
	void SetupMatrix();
	void FrameMove();
	Ray CalculatePickingRay(int x, int y);
	D3DXVECTOR3 ScreenToVector3(int x, int y);
	LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	//HWND getWindowHandle() const;
	//LPDIRECT3D9 GetD3D10() const;
	ID3D10Device* GetD3DDevice() const;
	D3DPRESENT_PARAMETERS GetD3Dpp() const;

	void SetBackBufferWidth(int width);
	void SetBackBufferHeight(int height);

	bool GetIsFullScreen() const;
	void SetIsFullScreen(bool is_fullscreen);

	int	GetScreenWidth() const;
	int GetScreenHeight() const;

	int	 GetLastWindowWidth() const;
	void SetLastWindowWidth(int windowWidth);

	int	 GetLastWindowHeight() const;
	void SetLastWindowHeight(int windowHeight);

private:
	ID3D10Device*			d3ddevice_;		// D3D9 Device
	IDXGISwapChain*			swapchain_;		// DXGI swap chain
	ID3D10RenderTargetView* rendertaregtview_; // D3D10 render target view
	D3DPRESENT_PARAMETERS	d3dpp_;			// D3D presentation parameters
	bool					is_fullscreen_;	// Is Game in Full-Screen mode?

	HWND hWnd;				// Handle of game window
	int	last_window_width_;	// Last window width
	int	last_window_height_;	// Last window height

	int screen_width_;	// The maximum resolution width
	int screen_height_;	// The maximum resolution height

private:
	Camera*					camera;			// Model-view camera
};

#endif // end __D3D9_H__