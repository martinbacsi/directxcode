#include "Cube.h"

ID3D10ShaderResourceView* Cube::inner_texture_ = NULL;
ID3D10ShaderResourceView* Cube::pTextures[kNumFaces_] = { NULL };

Cube::Cube(void)
	 : kNumCornerPoints_(8),
	   length_(10.0f),
	   layer_id_x_(-1),
	   layer_id_y_(-1),
	   layer_id_z_(-1),
	   vertex_buffer_(NULL),
	   input_layout_(NULL),
	   effects_(NULL),
	   technique_(NULL),
	   handle_world_matrix_(NULL),
	   handle_wvp_matrix_(NULL),
	   handle_face_texture_(NULL),
	   handle_inner_texture_(NULL),
	   handle_is_face_texture_(NULL),
	   handle_eye_position_(NULL),
	   rasterization_state_(NULL)
{
	for (int i = 0; i < kNumFaces_; ++i)
	{
		pIB[i] = NULL;
		textureId[i] = -1;
	}

	corner_points_ = new D3DXVECTOR3[kNumCornerPoints_];
	D3DXMatrixIdentity(&world_matrix_);
}

Cube::~Cube(void)
{
	// Delete corner points
	delete corner_points_;
	corner_points_ = NULL;

	// Release vertex buffer
	if (vertex_buffer_ != NULL)
	{
		vertex_buffer_->Release();
		vertex_buffer_ = NULL;
	}

	// Release index buffer
	for (int i = 0; i < kNumFaces_; ++i)
	{
		if (pIB[i] != NULL)
		{
			pIB[i]->Release();
			pIB[i] = NULL;
		}
	}

	// Relase input layout
	if (input_layout_)
	{
		input_layout_->Release();
		input_layout_ = NULL;
	}

	// Release effets
	if (effects_)
	{
		effects_->Release();
		effects_ = NULL;
	}
}

void Cube::Init(D3DXVECTOR3& top_left_front_point)
{
	InitVertexBuffer(top_left_front_point);
	InitIndexBuffer();
	InitCornerPoints(top_left_front_point);
	InitEffects();
	UpdateCenter();
}

void Cube::InitVertexBuffer(D3DXVECTOR3& front_bottom_left)
{
	float x = front_bottom_left.x;
	float y = front_bottom_left.y;
	float z = front_bottom_left.z;

	/* Example of front face
   1               2
	---------------
	|             |
	|             |
	|             |
	|             |
	|             |
	---------------
   0               3
	*/

	// Vertex buffer data
	Vertex vertices[] =
	{
		// Front face
		{          x,           y,           z,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f}, // 0
		{          x, y + length_,           z,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f}, // 1
		{x + length_, y + length_,           z,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f}, // 2
		{x + length_,           y,           z,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f}, // 3

		// Back face
		{x + length_,           y, z + length_,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f}, // 4
		{x + length_, y + length_, z + length_,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f}, // 5
		{          x, y + length_, z + length_,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f}, // 6
		{          x,           y, z + length_,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f}, // 7

		// Left face
		{          x,           y, z + length_, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f}, // 8
		{          x, y + length_, z + length_, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f}, // 9
		{          x, y + length_,           z, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f}, // 10
		{          x,           y,           z, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f}, // 11

		// Right face 
		{x + length_,           y,           z,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f}, // 12
		{x + length_, y + length_,           z,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f}, // 13
		{x + length_, y + length_, z + length_,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f}, // 14
		{x + length_,           y, z + length_,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f}, // 15

		// Top face
		{          x, y + length_,           z,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f}, // 16
		{          x, y + length_, z + length_,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f}, // 17
		{x + length_, y + length_, z + length_,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f}, // 18
		{x + length_, y + length_,           z,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f}, // 19

		// Bottom face
		{x + length_,           y,           z,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f}, // 20
		{x + length_,           y, z + length_,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f}, // 21
		{          x,           y, z + length_,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f}, // 22
		{          x,           y,           z,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f}, // 23
	};

	// Fill in  vertex buffer description
	D3D10_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D10_USAGE_DYNAMIC; // Enable CPU access
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE; // CPU can write this buffer.
	
	// This function will also used to restore the Rubik Cube, but the CreateVertexBuffer function will cost and hold the memory if vertex_buffer_ not released
	// When user press the 'R' frequently, memory usage increase time and time. so it's better to add a if branch to determine whether the buffer was created, if
	// that's true, we didn't create it again, we only lock it and copy the data, so the buffer will  only created once when the app starts.
	if (vertex_buffer_ == NULL)
	{
		D3D10_SUBRESOURCE_DATA vertex_data;
		ZeroMemory(&vertex_data, sizeof(vertex_data));
		vertex_data.pSysMem = vertices;
		HRESULT hr = d3d_device_->CreateBuffer(&bd, &vertex_data, &vertex_buffer_);
		if (FAILED(hr))
		{
			MessageBox(NULL, L"Create vertex buffer failed", L"Error", 0);
		}
	}

	// Copy vertex data
	/*
	For DirectX10, we need to update the code here, since Direct3D10 create and init Data for vertex buffer
	at the same time while Direct3D9 separate the process of createing vertex buffer and init data.
	*/
	void* pVertices;
	vertex_buffer_->Map(D3D10_MAP_WRITE_DISCARD, 0, &pVertices);
	memcpy(pVertices, vertices, sizeof(vertices));
	vertex_buffer_->Unmap();
}

void Cube::InitIndexBuffer()
{
	// Indices for triangle strips
	DWORD indicesFront[]  = { 0,  1,  3,  2};
	DWORD indicesBack[]   = { 4,  5,  7,  6};
	DWORD indicesLeft[]   = { 8,  9, 11, 10};
	DWORD indicesRight[]  = {12, 13, 15, 14};
	DWORD indicesTop[]    = {16, 17, 19, 18};
	DWORD indicesBottom[] = {20, 21, 23, 22};

	DWORD* indices[kNumFaces_] = {indicesFront, indicesBack, indicesLeft, indicesRight, indicesTop, indicesBottom};

	for(int i = 0; i < kNumFaces_; ++i)
	{
		// Only create index buffer once, prevent high memory usage when user press 'R' frequently, see comments in InitVertexBuffer.
		if (pIB[i] == NULL)
		{
			// Fill in index buffer description
			D3D10_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D10_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(indicesFront) * kNumFaces_;
			bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;

			// Create index buffer and copy data
			D3D10_SUBRESOURCE_DATA index_data;
			ZeroMemory(&index_data, sizeof(index_data));
			index_data.pSysMem = indices;
			HRESULT hr = d3d_device_->CreateBuffer(&bd, &index_data, &pIB[i]);
			if (FAILED(hr))
			{
				MessageBox(NULL, L"Create index buffer failed", L"Error", 0);
			}
		}

		// Copy index data
		void* pIndices;
		vertex_buffer_->Map(D3D10_MAP_WRITE_DISCARD, 0, &pIndices);
		memcpy(pIndices, indices, sizeof(indices));
		vertex_buffer_->Unmap();
	}
}

void Cube::InitCornerPoints(D3DXVECTOR3& front_bottom_left)
{
	// Calculate the min/max pint of the cube
	// min point is the front bottom left corner of the cube
	D3DXVECTOR3 min_point(front_bottom_left.x, front_bottom_left.y, front_bottom_left.z);

	// max point is the back top right corner of the cube
	D3DXVECTOR3 max_point(front_bottom_left.x + length_, front_bottom_left.y + length_, front_bottom_left.z + length_);

	/* The 8 points were count first on the front side from the bottom-left corner in clock-wise order
	 Then on the back side, with the same order

	 // Front face
		1-----------2
		|  front    |
		|  side     |
		|           |
		|           |
		0-----------3
	*/
	corner_points_[0] = D3DXVECTOR3(min_point.x, min_point.y, min_point.z);
	corner_points_[1] = D3DXVECTOR3(min_point.x, max_point.y, min_point.z);
	corner_points_[2] = D3DXVECTOR3(max_point.x, max_point.y, min_point.z);
	corner_points_[3] = D3DXVECTOR3(max_point.x, min_point.y, min_point.z);

	/* Back face
	    5-----------6
		|  front    |
		|  side     |
		|           |
		|           |
		4-----------7
	*/
	corner_points_[4] = D3DXVECTOR3(max_point.x, min_point.y, max_point.z);
	corner_points_[5] = D3DXVECTOR3(max_point.x, max_point.y, max_point.z);
	corner_points_[6] = D3DXVECTOR3(min_point.x, max_point.y, max_point.z);
	corner_points_[7] = D3DXVECTOR3(min_point.x, min_point.y, max_point.z);

	// Initilize min_point and max_point
	min_point_ = min_point;
	max_point_ = max_point;
}

void Cube::InitRasterizationState()
{
	rasterization_desc_.AntialiasedLineEnable = true;
	rasterization_desc_.CullMode = D3D10_CULL_BACK;
	rasterization_desc_.DepthBias = 0;
	rasterization_desc_.DepthBiasClamp = 0.0f;
	rasterization_desc_.DepthClipEnable = true;
	rasterization_desc_.FillMode = D3D10_FILL_SOLID;
	rasterization_desc_.FrontCounterClockwise = false;
	rasterization_desc_.MultisampleEnable = false;
	rasterization_desc_.ScissorEnable = false;
	rasterization_desc_.SlopeScaledDepthBias = 0.0f;

	d3d_device_->CreateRasterizerState(&rasterization_desc_, &rasterization_state_);
}

void Cube::InitEffects()
{
	ID3DBlob* pErrorBlob;

	// Create the effect
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

	// Compile the effects file
    HRESULT hr = D3DX10CreateEffectFromFile( L"rubik_cube.fx", NULL, NULL, "fx_4_0", dwShaderFlags, 0,
                                         d3d_device_, NULL, NULL, &effects_, &pErrorBlob, NULL);

    // Output the error message if compile failed
	if(FAILED(hr))
    {
        if(pErrorBlob != NULL)
		{
			OutputDebugStringA((CHAR*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
    }

	// Release the Blob
	if(pErrorBlob)
	{
		pErrorBlob->Release();
	}

    // Obtain the technique
    technique_ = effects_->GetTechniqueByName("Render");
	if (technique_ == NULL)
	{
		MessageBox(NULL, L"Get technique failed", L"Error", 0);
	}

	 // Obtain shader variables
	handle_world_matrix_	= effects_->GetVariableByName("World")->AsMatrix();
	handle_wvp_matrix_      = effects_->GetVariableByName("gWVP")->AsMatrix();
	handle_is_face_texture_ = effects_->GetVariableByName("Is_Face_Texture")->AsScalar();
	handle_face_texture_	= effects_->GetVariableByName("FaceTexture")->AsShaderResource();
	handle_inner_texture_   = effects_->GetVariableByName("InnerTexture")->AsShaderResource();
	handle_eye_position_    = effects_->GetVariableByName("EyePosition")->AsVector();

    // Define the input layout
    D3D10_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };

	// Create the input layout
    UINT numElements = sizeof(layout) / sizeof(layout[0]);
	D3D10_PASS_DESC PassDesc;
	ZeroMemory(&PassDesc, sizeof(PassDesc));
    technique_->GetPassByIndex(0)->GetDesc(&PassDesc);
    hr = d3d_device_->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
                                          PassDesc.IAInputSignatureSize, &input_layout_);
    if(FAILED(hr))
	{
		MessageBox(NULL, L"Create input layout failed", L"Error", 0);
	}
}

D3DXVECTOR3 Cube::CalculateCenter(D3DXVECTOR3& min_point, D3DXVECTOR3& max_point)
{
	return (min_point + max_point) / 2;
}

void Cube::SetTextureId(int faceId, int texId)
{
	textureId[faceId] = texId;
}

void Cube::SetFaceTexture(ID3D10ShaderResourceView** faceTextures, int numTextures)
{
	for(int i = 0; i < numTextures; ++i)
	{
		pTextures[i] = faceTextures[i];
	}
}

void Cube::SetInnerTexture(ID3D10ShaderResourceView* innerTexture)
{
	inner_texture_ = innerTexture;
}

void Cube::SetDevice(ID3D10Device* pDevice)
{
	d3d_device_ = pDevice;
}

void Cube::UpdateMinMaxPoints(D3DXVECTOR3& rotate_axis, int num_half_PI)
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
		D3DXMatrixRotationAxis(&rotate_matrix, &rotate_axis, (float)D3DX_PI / 2);
	}
	else if (num_half_PI == 2)
	{
		D3DXMatrixRotationAxis(&rotate_matrix, &rotate_axis, (float)D3DX_PI);
	}
	else // (num_half_PI == 3)
	{
		D3DXMatrixRotationAxis(&rotate_matrix, &rotate_axis, 1.5f * (float)D3DX_PI);
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

void Cube::UpdateCenter()
{
	center_ = (min_point_ + max_point_) / 2;
}

void Cube::Rotate(D3DXVECTOR3& axis, float angle)
{
	// Calculate the rotation matrix
	D3DXMATRIX rotate_matrix;
	D3DXMatrixRotationAxis(&rotate_matrix, &axis, angle);

	// This may cause the matrix multiplication accumulate errors, how to fix it?
	world_matrix_ *= rotate_matrix;
}

void Cube::Draw(D3DXMATRIX& view_matrix, D3DXMATRIX& proj_matrix, D3DXVECTOR3& eye_pos)
{
	// Setup world matrix for current cube
	handle_world_matrix_->SetMatrix((float*)world_matrix_);

	// Set world view projection matrix
	D3DXMATRIX wvp_matrix = world_matrix_ * view_matrix * proj_matrix;
	handle_wvp_matrix_->SetMatrix((float*)wvp_matrix);

	// Set eye position
	D3DXVECTOR4 eye_position(eye_pos.x, eye_pos.y, eye_pos.z, 1.0f);
	handle_eye_position_->SetFloatVector((float*)eye_position);

	// Set vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	d3d_device_->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

	// Set the input layout
	d3d_device_->IASetInputLayout(input_layout_);

	// Set geometry type
	d3d_device_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Set rasterazition state
	d3d_device_->RSSetState(rasterization_state_);

	// Apply each pass in technique and draw triangle.
	D3D10_TECHNIQUE_DESC techDesc;
	technique_->GetDesc(&techDesc);
	for (unsigned int i = 0; i < techDesc.Passes; ++i)
	{
		technique_->GetPassByIndex(i)->Apply(0);

		// Draw cube by draw every face of the cube
		for(int i = 0; i < kNumFaces_; ++i)
		{
			/*if(textureId[i] >= 0)
			{
				handle_is_face_texture_->SetBool(true);
				handle_face_texture_->SetResource(pTextures[textureId[i]]);
			}
			else
			{
				handle_is_face_texture_->SetBool(false);
				handle_inner_texture_->SetResource(inner_texture_);
			}*/

			// Set index buffer
			d3d_device_->IASetIndexBuffer(pIB[i], DXGI_FORMAT_R32_UINT, 0);
			d3d_device_->DrawIndexed(24, 0, 0);
		}
	}
}

float Cube::GetLength() const
{
	return length_;
}

D3DXVECTOR3 Cube::GetMinPoint() const
{
	return min_point_;
}

D3DXVECTOR3 Cube::GetMaxPoint() const
{
	return max_point_;
}

D3DXVECTOR3 Cube::GetCenter() const
{
	return center_;
}

void Cube::SetWorldMatrix(D3DXMATRIX& world_matrix)
{
	world_matrix_ = world_matrix;
}

bool Cube::InLayer(int layer_id)
{
	return ( layer_id_x_ == layer_id 
		  || layer_id_y_ == layer_id
		  || layer_id_z_ == layer_id );
}

void Cube::SetLayerIdX(int layer_id_x)
{
	layer_id_x_ = layer_id_x;
}

void Cube::SetLayerIdY(int layer_id_y)
{
	layer_id_y_ = layer_id_y;
}

void Cube::SetLayerIdZ(int layer_id_z)
{
	layer_id_z_ = layer_id_z;
}
