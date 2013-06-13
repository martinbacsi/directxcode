#ifndef __CUBE_H__
#define __CUBE_H__

#include "d3dx9.h"

#define VERTEX_FVF ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

struct Vertex
{
	float  x,  y,  z; // position
	float nx, ny, nz; // normal
	float  u,  v;     // texture
};

class Cube
{
public:
	Cube(void);
	~Cube(void);

	void Init(D3DXVECTOR3 top_left_front_point);
	void SetDevice(LPDIRECT3DDEVICE9 pDevice);
	void SetTextureId(int faceId, int textureId);
	static void SetFaceTexture(LPDIRECT3DTEXTURE9* faceTextures, int numTextures);
	static void SetInnerTexture(LPDIRECT3DTEXTURE9 innerTexture);
	//void UpdateMatrix(D3DXVECTOR3 rotate_axis, int num_half_PI);
	void UpdateMinMaxPoints(D3DXVECTOR3 rotate_axis, int num_half_PI);
	void Rotate(D3DXVECTOR3& axis, float angle);
	void Draw();

	D3DXVECTOR3 GetMinPoint() const;
	D3DXVECTOR3 GetMaxPoint() const;

	void SetIsSelected(bool is_selected);
	bool GetIsSelected() const;

private:
	void InitBuffers(D3DXVECTOR3 topleftfront);
	void InitCornerPoints(D3DXVECTOR3 top_left_front_point);	// Initialize corner points.

private:
	float length_;								// side length_ of the cube.
	D3DXVECTOR3 max_point_;						// The max corner point of the cube(back-top-right corner)
	D3DXVECTOR3 min_point_;						// The min corner point of the cube(front-bottom-left corner)
	static const int kNumFaces_ = 6;			// The number of faces in a cube, this is always 6.
	const int kNumCornerPoints_;				// Number of corner points of the cube
	int textureId[kNumFaces_];					// the index is the faceId, the value is the textureId.

	bool is_selected_;							// Whether cube was selected in current rotation.

	static LPDIRECT3DTEXTURE9 pTextures[kNumFaces_];
	static LPDIRECT3DTEXTURE9 inner_texture_;	// Inner face texture.
	LPDIRECT3DINDEXBUFFER9  pIB[kNumFaces_] ;
	D3DXVECTOR3*			corner_points_;		// array to store the 8 corner poinst of the cube 
	LPDIRECT3DVERTEXBUFFER9 vertex_buffer_ ;
	LPDIRECT3DDEVICE9		d3d_device_ ;
	D3DXMATRIX				world_matrix_ ;		// world matrix for unit cube, for rotation.
};

#endif // end __CUBE_H__