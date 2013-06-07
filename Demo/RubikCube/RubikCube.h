#ifndef __RUBIK_CUBE_H__
#define __RUBIK_CUBE_H__

#include "Cube.h"
#include "Camera.h"
#include "D3D9.h"
#include "Math.h"

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
	int getWindowPosX() const;
	int getWindowPosY() const;
	int getWindowWidth() const;
	int getWindowHeight() const;

private:
	void Shuffle();
	void ToggleFullScreen();
	void OnLeftButtonDown(int x, int y);
	void OnMouseMove(int x, int y);
	void OnLeftButtonUp();
	void InitTextures();
	Face GetPickedFace(D3DXVECTOR3 hit_point) const; // Get the face picking by mouse 
	D3DXPLANE GeneratePlane(Face face, D3DXVECTOR3& previous_point, D3DXVECTOR3& current_point);
	void MarkRotateCubes(D3DXPLANE& plane);			// Mark which cube to rotate, set is_selected = true
	D3DXVECTOR3 GetRotateAxis(Face face, D3DXVECTOR3& previous_point, D3DXVECTOR3& current_point);
	float CalculateRotateAngle();
	RotateDirection GetRotateDirection(Face face, D3DXVECTOR3& axis, D3DXVECTOR3& previous_point, D3DXVECTOR3& current_point);
	void Rotate(D3DXVECTOR3& axis, float angle);		// Rotate a layer

private:
	const int kNumCubes; // Number of unit cubes, 27 unit cubes build up a rubik cube.
	Cube* cubes;						// Array to store 27 unit cubes

	static const int numFaces = 6;  // Number of faces
	Rect* faces;					// Store faces in rect

	float cube_length_;				// length of the unit cube.
	float gap_between_layers_;		// the length between two layers.
	float gaps;						// spaces between two samll cubes
	float length;					// this value should get from the cube interface.
	float faceLength;				// Face length is the length of the square face of the Rubik.
	float HalfWidth;				// Half of the face length

	bool is_hit_;					// The picking ray hit the RubikCube
	bool OneRotateFinish;			// A rotaton action was finished.
	bool is_cubes_selected_;				// Does cubes selected in current rotation?
	//bool hitFlag;					// Determine whether the picking ray hit the cube.
	//bool AlreadyGetLayer;			// 
	bool inactive;					// Window was inactive? turn when device lost, window minimized...

	D3DXVECTOR3 current_hitpoint_;		// Current hit point
	D3DXVECTOR3 previous_hitpoint_;		// Last hit point


	float rotate_speed_;				// layer rotation speed
	float total_rotate_angle_;			// The angle rotated when mouse up
	D3DXVECTOR3 rotate_axis_;			// Rotate axis, X or Y or Z
	RotateDirection rotate_direction_;	// Rotate direction


	ArcBall WorldBall ;
	Camera*					camera;			// Model view camera

	HWND hWnd;
	WINDOWPLACEMENT wp ;	// Window position and size

	// Initial window position and size
	int initWindowPosX;
	int initWindowPosY;
	int InitWindowWidth;
	int InitWindowHeight;

	// Last window size
	int LastWindowWidth;
	int LastWindowHeight;

	// Current window size, used in resizing window 
	int CurrentWindowWidth;
	int CurrentWindowHeight;

	// Textures
	int texWidth;								// Texture width in pixel.
	int texHeight;								// Texture height in pixel.
	int textureId[numFaces];					// The index is the faceId, the value is the textureId.
	LPDIRECT3DTEXTURE9	pTextures[numFaces];	// Face textures
	LPDIRECT3DTEXTURE9	pInnerTexture;			// Inner face texture.

	// Objects from other classes
	D3D9* d3d9;
};

#endif // end __RUBIK_CUBE_H__