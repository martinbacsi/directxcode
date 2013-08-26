int TextureId;
matrix WVPMatrix; // World-View-Porj matrix

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
};

//--------------------------------------------------------------------------------------
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, WVPMatrix);
    output.Tex = input.Tex;

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input ) : SV_Target
{
	// Side color, black
	if (input.Tex.x >  0.0f && input.Tex.x < 0.05f ||
	    input.Tex.x > 0.95f && input.Tex.x <  1.0f ||
		input.Tex.y >  0.0f && input.Tex.y < 0.05f ||
	    input.Tex.y > 0.95f && input.Tex.y <  1.0f )
	{
		return float4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	// Face color
	if (TextureId == 0)
	{
		return float4(1.0f, 1.0f, 1.0f, 1.0f); // White
	}
	else if (TextureId == 1)
	{
		return float4(1.0f, 1.0f, 0.0, 1.0f); // Yellow
	}
	else if (TextureId == 2)
	{
		return float4(1.0f, 0.0f, 0.0f, 1.0f); // Red
	}
	else if (TextureId == 3)
	{
		return float4(1.0f, 0.65f, 0.0f, 1.0f); // Orange
	}
	else if (TextureId == 4)
	{
		return float4(0.0f, 1.0f, 0.0f, 1.0f); // Green
	}
	else if (TextureId == 5)
	{
		return float4(0.0f, 0.0f, 1.0f, 1.0f); // Blue
	}
	else 
	{
		return float4(0.0f, 0.0f, 0.0f, 1.0f); // Black
	}
}

//--------------------------------------------------------------------------------------
technique10 Render
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}
