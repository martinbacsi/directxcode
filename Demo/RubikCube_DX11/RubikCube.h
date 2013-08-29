#ifndef __RUBIK_CUBE_H__
#define __RUBIK_CUBE_H__

#include "Cube.h"
#include "Camera.h"
#include "Math.h"

//#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// The 6 faces of the Rubik Cube
enum Face
{
	kFrontFace =  0,
	kBackFace  =  1,
	kLeftFace  =  2,
	kRightFace =  3,
	kTopFace   =  4,
	kBottomFace = 5,

	kUnknownFace = 10
};

enum RotateDirection
{
	kClockWise = 0,
	kCounterClockWise = 1,

	kUnknownDirection = 2
};

class RubikCube
{
public:
	RubikCube(void);
	~RubikCube(void);

	void Initialize(HWND hWnd);
	void Render();
	LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	int GetWindowPosX() const;
	int GetWindowPosY() const;
	int GetWindowWidth() const;
	int GetWindowHeight() const;

private:
	void Shuffle();
	void Restore(); 
	void ToggleFullScreen();
	void OnLeftButtonDown(int x, int y);
	void OnMouseMove(int x, int y);
	void OnLeftButtonUp();
	void InitD3D11(HWND hWnd);
	void ResizeD3DScene(int width, int height);
	void SetupMatrix();
	void InitCubes();
	void CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	void InitVertexShader();
	void InitPixelShader();
	void ResetLayerIds();
	void ResetTextures();
	D3DXVECTOR2 GetMaxScreenResolution();
	XMVECTOR ScreenToVector3(int x, int y);
	Ray CalculatePickingRay(int x, int y);
	Face GetPickedFace(XMVECTOR hit_point) const;	// Get the face picking by mouse 
	D3DXPLANE GeneratePlane(Face face, XMVECTOR& previous_point, XMVECTOR& current_point);
	XMVECTOR GetRotateAxis(Face face, XMVECTOR& previous_point, XMVECTOR& current_point);
	float CalculateRotateAngle();
	RotateDirection GetRotateDirection(Face face, XMVECTOR& axis, XMVECTOR& previous_vector, XMVECTOR& current_vector);
	int  GetHitLayer(Face face, XMVECTOR& rotate_axis, XMVECTOR& hit_point);
	void RotateLayer(int layer, XMVECTOR& axis, float angle);

private:
	const int kNumLayers;	// Number of layers in one direction, a 3 x 3 Rubik Cube has num_layers_ = 3.
	const int kNumCubes;	// Number of unit cubes, 27 unit cubes build up a rubik cube.
	Cube* cubes;			// Array to store 27 unit cubes

	const int kNumFaces;		// Number of faces
	Rect* faces;				// Store faces in rect

	float face_length_;			// Face length of the Rubik Cube
	float gap_between_layers_;	// the length between two layers.

	bool is_hit_;				// The picking ray hit the RubikCube
	int  hit_layer_;			// The layer hit by the picking Ray
	bool rotate_finish_;		// A rotaton action was finished.
	bool is_cubes_selected_;	// Does cubes selected in current rotation?
	bool window_active_;		// Window was inactive? turn when device lost, window minimized...

	XMVECTOR previous_hitpoint_;
	XMVECTOR current_hitpoint_;

	XMVECTOR current_vector_;		// Current hit point
	XMVECTOR previous_vector_;		// Last hit point

	float rotate_speed_;				// layer rotation speed
	float total_rotate_angle_;			// The angle rotated when mouse up
	XMVECTOR rotate_axis_;			// Rotate axis, X or Y or Z
	RotateDirection rotate_direction_;	// Rotate direction


	ArcBall* world_arcball_ ;
	Camera*	camera_;			// Model view camera

	HWND hWnd_;
	WINDOWPLACEMENT wp ;		// Window position and size

	// Initial window position and size
	int init_window_x_;
	int init_window_y_;
	int init_window_width_;
	int init_window_height_;

	// Last window size
	int last_window_width_;
	int last_window_height_;

	// Current window size, used in resizing window 
	int current_window_width_;
	int current_window_height_;

	ID3D11Device*			d3ddevice_;			// D3D9 Device
	IDXGISwapChain*			swap_chain_;		// DXGI swap chain
	ID3D11DeviceContext*	immediate_context_; // Immediate context
	ID3D11VertexShader*		vertex_shader_;		// Vertex shader
	ID3D11PixelShader*		pixel_shader_;		// Pixel shader

	ID3D11InputLayout*		input_layout_;
	IDXGIOutput*			output_;			// DXGI Adapter output
	ID3D11RenderTargetView* rendertarget_view_; // Rendertarget view
	ID3D11DepthStencilView* depth_stencil_view_;	// Depth-stencil view
	DXGI_SWAP_CHAIN_DESC    sd_;				// SwapChain descriptioin
	D3D11_VIEWPORT			vp_;				// Viewport
	bool					is_fullscreen_;		// Is Game in Full-Screen mode?

	HWND hWnd;			// Handle of game window
	int screen_width_;	// The maximum resolution width
	int screen_height_;	// The maximum resolution height
};

#endif // end __RUBIK_CUBE_H__