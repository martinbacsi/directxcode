#ifndef __CUBE_H__
#define __CUBE_H__

#include <D3D11.h>
#include <DirectXMath.h>

using namespace DirectX;

struct Vertex
{
	float  x,  y,  z; // position
	float  u,  v;     // texture
};

// Constant buffer
struct ConstantBuffer
{
	int texture_id;
	XMMATRIX matWVP; // World-View-Projection matrix
};

class Cube
{
public:
	Cube(void);
	~Cube(void);

	void Init(XMVECTOR& top_left_front_point);
	void SetDevice(ID3D11Device* pDevice);
	void SetTextureId(int faceId, int textureId);
	void UpdateMinMaxPoints(XMVECTOR& rotate_axis, int num_half_PI);
	void UpdateCenter();
	void UpdateLayerId();
	void Rotate(XMVECTOR& axis, float angle);
	void Draw(ID3D11DeviceContext* immediate_context, XMMATRIX& view_matrix, XMMATRIX& proj_matrix);

	float GetLength() const;

	XMVECTOR GetMinPoint() const;
	XMVECTOR GetMaxPoint() const;
	XMVECTOR GetCenter() const;

	void SetWorldMatrix(XMMATRIX& world_matrix);

	// Set layer id
	void SetLayerIdX(int layer_id_x);
	void SetLayerIdY(int layer_id_y);
	void SetLayerIdZ(int layer_id_z);

	// Determine whether cube in a given layer
	bool InLayer(int layer_id);

private:
	void InitBuffers(XMVECTOR& front_bottom_left);
	void InitVertexBuffer(XMVECTOR& front_bottom_left);
	void InitIndexBuffer();
	void InitConstantBuffer();
	void InitCornerPoints(XMVECTOR& front_bottom_left_point);	// Initialize corner points.
	void InitInputLayout();
	XMVECTOR CalculateCenter(XMVECTOR& min_point, XMVECTOR& max_point);
	void InitLayerIds();

private:
	float length_;								// side length_ of the cube.
	XMVECTOR max_point_;						// The max corner point of the cube(back-top-right corner)
	XMVECTOR min_point_;						// The min corner point of the cube(front-bottom-left corner)
	XMVECTOR center_;						// Cube center
	static const int kNumFaces_ = 6;			// The number of faces in a cube, this is always 6.
	const int kNumCornerPoints_;				// Number of corner points of the cube
	int textureId[kNumFaces_];					// the index is the faceId, the value is the textureId.

	// Each cube in the Rubik Cube has 3 layer id, for a 3 x 3 Rubik Cube, the layer id was count as below:
	// along X axis, from negative to positive(left -> right) 0, 1, 2
	// along Y axis, from negative to positive(bottom -> top) 3, 4, 5
	// along Z axis, from negative to positive(front -> back) 6, 7, 8
	// So the front-top-left corner cube's id is:
	// layer_x_ = 0, layer_y_ = 5, layer_z_ = 6
	int layer_id_x_;
	int layer_id_y_;
	int layer_id_z_;

	ID3D11Buffer*	vertex_buffer_ ;	// Vertex buffer
	ID3D11Buffer*	pIB[kNumFaces_] ;
	ID3D11Buffer*	constant_buffer_;
	XMVECTOR*		corner_points_;		// array to store the 8 corner poinst of the cube 
	ID3D11Device*	d3d_device_ ;
	XMMATRIX		world_matrix_ ;		// world matrix for unit cube, for rotation.

};

#endif // end __CUBE_H__