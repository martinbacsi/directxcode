#include "Cube.h"

LPDIRECT3DTEXTURE9 Cube::inner_texture_ = NULL;
LPDIRECT3DTEXTURE9 Cube::pTextures[kNumFaces_] = { NULL };

Cube::Cube(void)
	 : kNumCornerPoints_(8),
	   is_selected_(false)
{
	length_ = 10.0f;

	for (int i = 0; i < kNumFaces_; ++i)
	{
		pIB[i] = NULL;
		textureId[i] = -1;
	}

	corner_points_ = new D3DXVECTOR3[kNumCornerPoints_];

	vertex_buffer_ = NULL;

	D3DXMatrixIdentity(&world_matrix_);
}

Cube::~Cube(void)
{
	// Delete corner points
	delete corner_points_;
	corner_points_ = NULL;

	// Release vertex buffer
	if(vertex_buffer_ != NULL)
	{
		vertex_buffer_->Release();
		vertex_buffer_ = NULL;
	}

	// Release index buffer
	for(int i = 0; i < kNumFaces_; ++i)
	{
		if(pIB[i] != NULL)
		{
			pIB[i]->Release();
			pIB[i] = NULL;
		}
	}
}

void Cube::Init(D3DXVECTOR3 top_left_front_point)
{
	InitBuffers(top_left_front_point);
	InitCornerPoints(top_left_front_point);
}

// Initialize vertex and index buffer
void Cube::InitBuffers(D3DXVECTOR3 topleftfront)
{
	float x = topleftfront.x;
	float y = topleftfront.y;
	float z = topleftfront.z;

	// Vertex buffer data
	Vertex vertices[] =
	{
		// Front face
		{          x,           y,           z,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f}, // 0
		{x + length_,           y,           z,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f}, // 1
		{x + length_, y - length_,           z,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f}, // 2
		{          x, y - length_,           z,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f}, // 3

		// Back face
		{x + length_,           y, z + length_,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f}, // 4
		{          x,           y, z + length_,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f}, // 5
		{          x, y - length_, z + length_,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f}, // 6
		{x + length_, y - length_, z + length_,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f}, // 7

		// Left face
		{          x,           y, z + length_, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f}, // 8
		{          x,           y,           z, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f}, // 9
		{          x, y - length_,           z, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f}, // 10
		{          x, y - length_, z + length_, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f}, // 11

		// Right face 
		{x + length_,           y,           z,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f}, // 12
		{x + length_,           y, z + length_,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f}, // 13
		{x + length_, y - length_, z + length_,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f}, // 14
		{x + length_, y - length_,           z,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f}, // 15

		// Top face
		{          x,           y, z + length_,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f}, // 16
		{x + length_,           y, z + length_,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f}, // 17
		{x + length_,           y,           z,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f}, // 18
		{          x,           y,           z,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f}, // 19

		// Bottom face
		{x + length_, y - length_, z + length_,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f}, // 20
		{          x, y - length_, z + length_,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f}, // 21
		{          x, y - length_,           z,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f}, // 22
		{x + length_, y - length_,           z,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f}, // 23
	};

	
	// Create vertex buffer
	if( FAILED( d3d_device_->CreateVertexBuffer( sizeof(vertices) * sizeof(Vertex),
		D3DUSAGE_WRITEONLY, 
		VERTEX_FVF,
		D3DPOOL_MANAGED, 
		&vertex_buffer_, 
		NULL ) ) )
	{
		MessageBox(NULL, L"Create vertex buffer failed", L"Error", 0);
	}

	// Copy vertex data
	VOID* pVertices;
	if( FAILED( vertex_buffer_->Lock( 0, sizeof(vertices), (void**)&pVertices, 0 ) ) )
		MessageBox(NULL, L"Copy vertex buffer failed", L"Error", 0);
	memcpy( pVertices, vertices, sizeof(vertices) );
	vertex_buffer_->Unlock();

	// Triangle strips
	WORD indicesFront[]  = { 0,  1,  3,  2};
	WORD indicesBack[]   = { 4,  5,  7,  6};
	WORD indicesLeft[]   = { 8,  9, 11, 10};
	WORD indicesRight[]  = {12, 13, 15, 14};
	WORD indicesTop[]    = {16, 17, 19, 18};
	WORD indicesBottom[] = {20, 21, 23, 22};

	WORD* indices[kNumFaces_] = {indicesFront, indicesBack, indicesLeft, indicesRight, indicesTop, indicesBottom};

	for(int i = 0; i < kNumFaces_; ++i)
	{
		// Create index buffer
		if( FAILED( d3d_device_->CreateIndexBuffer( sizeof(indicesFront) * sizeof(WORD), 
			D3DUSAGE_WRITEONLY, 
			D3DFMT_INDEX16, 
			D3DPOOL_MANAGED, 
			&pIB[i], 
			0) ) )
		{
			MessageBox(NULL, L"Create index buffer failed", L"Error", 0);
		}

		// Copy index data
		VOID *pIndices;
		if( FAILED( pIB[i]->Lock( 0, sizeof(indicesFront), (void **)&pIndices, 0) ) )
			MessageBox(NULL, L"Copy index buffer data failed", L"Error", 0);
		memcpy(pIndices, indices[i], sizeof(indicesFront) );
		pIB[i]->Unlock() ;
	}
}

void Cube::InitCornerPoints(D3DXVECTOR3 top_left_front_point)
{
	/* The 8 points were count first on the front side from the top-left corner in clock-wise order
	 Then on the back side, with the same order
		1-----------2
		|  front    |
		|  side     |
		|           |
		|           |
		4-----------3
	*/

	// Calculate the min/max pint of the cube
	// min point is the front bottom left corner of the cube
	D3DXVECTOR3 min_point(top_left_front_point.x, top_left_front_point.y - length_, top_left_front_point.z);

	// max point is the back top right corner of the cube
	D3DXVECTOR3 max_point(top_left_front_point.x +length_, top_left_front_point.y, top_left_front_point.z + length_);


	// Front face
	corner_points_[0] = D3DXVECTOR3(min_point.x, max_point.y, min_point.z);
	corner_points_[1] = D3DXVECTOR3(max_point.x, max_point.y, min_point.z);
	corner_points_[2] = D3DXVECTOR3(max_point.x, min_point.y, min_point.z);
	corner_points_[3] = min_point;

	// Back face
	corner_points_[4] = D3DXVECTOR3(min_point.x, max_point.y, max_point.z);
	corner_points_[5] = max_point;
	corner_points_[6] = D3DXVECTOR3(max_point.x, min_point.y, max_point.z);
	corner_points_[7] = D3DXVECTOR3(min_point.x, min_point.y, max_point.z);

	// Initilize min_point and max_point
	min_point_ = min_point;
	max_point_ = max_point;
}

void Cube::SetTextureId(int faceId, int texId)
{
	textureId[faceId] = texId;
}

void Cube::SetFaceTexture(LPDIRECT3DTEXTURE9* faceTextures, int numTextures)
{
	for(int i = 0; i < numTextures; ++i)
	{
		pTextures[i] = faceTextures[i];
	}
}

void Cube::SetInnerTexture(LPDIRECT3DTEXTURE9 innerTexture)
{
	inner_texture_ = innerTexture;
}

void Cube::SetDevice(LPDIRECT3DDEVICE9 pDevice)
{
	d3d_device_ = pDevice;
}

void Cube::UpdateMinMaxPoints(D3DXVECTOR3 rotate_axis, int num_half_PI)
{
	// Build up the rotation matrix with the overall angle
	// This angle is times of D3DX_PI / 2.
	D3DXMATRIX rotate_matrix;
	D3DXMatrixIdentity(&rotate_matrix);
	
	if (num_half_PI == 0)
	{
		D3DXMatrixRotationAxis(&rotate_matrix, &rotate_axis, 0);
	}
	else if (num_half_PI == 1)
	{
		D3DXMatrixRotationAxis(&rotate_matrix, &rotate_axis, D3DX_PI / 2);
	}
	else if (num_half_PI == 2)
	{
		D3DXMatrixRotationAxis(&rotate_matrix, &rotate_axis, D3DX_PI);
	}
	else // (num_half_PI == 3)
	{
		D3DXMatrixRotationAxis(&rotate_matrix, &rotate_axis, 1.5f * D3DX_PI);
	}

	// Translate the min_point_ and max_point_ of the cube, after rotation, the two points 
	// was changed, need to recalculate them with the rotation matrix.
	D3DXVECTOR3 min_point;
	D3DXVECTOR3 max_point;
	D3DXVec3TransformCoord(&min_point, &min_point_, &rotate_matrix);
	D3DXVec3TransformCoord(&max_point, &max_point_, &rotate_matrix);

	// After translate by the world matrix, the min/max point need recalculate
	min_point_.x = min(min_point.x, max_point.x);
	min_point_.y = min(min_point.y, max_point.y);
	min_point_.z = min(min_point.z, max_point.z);

	max_point_.x = max(min_point.x, max_point.x);
	max_point_.y = max(min_point.y, max_point.y);
	max_point_.z = max(min_point.z, max_point.z);
}

void Cube::Rotate(D3DXVECTOR3& axis, float angle)
{
	// Calculate the rotation matrix
	D3DXMATRIX rotate_matrix;
	D3DXMatrixRotationAxis(&rotate_matrix, &axis, angle);

	// This may cause the matrix multiplication accumulate errors, how to fix it?
	world_matrix_ *= rotate_matrix;
}

void Cube::Draw()
{
	// Setup world matrix for current cube
	d3d_device_->SetTransform(D3DTS_WORLD, &world_matrix_) ;

	// Draw cube by draw every face of the cube
	for(int i = 0; i < kNumFaces_; ++i)
	{
		if(textureId[i] >= 0)
		{
			d3d_device_->SetTexture(0, pTextures[textureId[i]]);
		}
		else
		{
			d3d_device_->SetTexture(0, inner_texture_);
		}

		d3d_device_->SetStreamSource(0, vertex_buffer_, 0, sizeof(Vertex));
		d3d_device_->SetIndices(pIB[i]) ;
		d3d_device_->SetFVF(VERTEX_FVF);

		d3d_device_->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, 24, 0, 2);
	}
}

D3DXVECTOR3 Cube::GetMinPoint() const
{
	return min_point_;
}

D3DXVECTOR3 Cube::GetMaxPoint() const
{
	return max_point_;
}

void Cube::SetIsSelected(bool is_selected)
{
	is_selected_ = is_selected;
}

bool Cube::GetIsSelected() const
{
	return is_selected_;
}
