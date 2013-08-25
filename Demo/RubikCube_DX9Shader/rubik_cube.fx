
// World, View and Projection Matrix
matrix matWVP; 

int FaceId;

// Eye position 
float3 EyePosition;

// Input vertex structure
struct InputVS
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 Tex : TEXCOORD0;
};

// Output Vertex structure
struct OutputVS
{
	float4 posH : POSITION;
	float4 color : COLOR;
	float2 Tex : TEXCOORD0;
};

// Vertex shader
OutputVS BasicVS(InputVS inputVS)
{
	// Zero out our output.
	OutputVS outVS = (OutputVS)0;

	// Transform to homogeneous clip space.
	outVS.posH = mul(inputVS.pos, matWVP);

	// texture coordinate
	outVS.Tex = inputVS.Tex;

	// Done--return the output.
	return outVS;
}

// Pixel shader
float4 BasicPS(OutputVS outputVS) : COLOR
{
	// Side color, black
	if (outputVS.Tex.x >  0.0f && outputVS.Tex.x < 0.05f ||
	    outputVS.Tex.x > 0.95f && outputVS.Tex.x <  1.0f ||
		outputVS.Tex.y >  0.0f && outputVS.Tex.y < 0.05f ||
	    outputVS.Tex.y > 0.95f && outputVS.Tex.y <  1.0f )
	{
		return float4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	// Face color
	if (FaceId == 0)
	{
		return float4(1.0f, 1.0f, 1.0f, 1.0f); // White
	}
	else if (FaceId == 1)
	{
		return float4(1.0f, 1.0f, 0.0, 1.0f); // Yellow
	}
	else if (FaceId == 2)
	{
		return float4(1.0f, 0.0f, 0.0f, 1.0f); // Red
	}
	else if (FaceId == 3)
	{
		return float4(1.0f, 0.65f, 0.0f, 1.0f); // Orange
	}
	else if (FaceId == 4)
	{
		return float4(0.0f, 1.0f, 0.0f, 1.0f); // Green
	}
	else if (FaceId == 5)
	{
		return float4(0.0f, 0.0f, 1.0f, 1.0f); // Blue
	}
	else 
	{
		return float4(0.0f, 0.0f, 0.0f, 1.0f); // Black
	}
}

technique Tech1
{
	pass p0
    {
		vertexShader = compile vs_3_0 BasicVS();
		pixelShader  = compile ps_3_0 BasicPS();
    }
}
