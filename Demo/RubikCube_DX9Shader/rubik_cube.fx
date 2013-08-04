// World, View and Projection Matrix
uniform extern float4x4 gWVP; 

// Face texture
texture FaceTexture;

// Inner texture
texture InnerTexture;

sampler CubeTextureSampler = sampler_state
{
	Texture = <FaceTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

// Output Vertex structure
struct OutputVS
{
      float4 posH : POSITION0;
	  float2 TextureUV : TEXCOORD0;
};

OutputVS BasicVS(float4 posL : POSITION0, float2 vTexture : TEXCOORD0)
{
      // Zero out our output.
      OutputVS outVS = (OutputVS)0;

      // Transform to homogeneous clip space.
      outVS.posH = mul(posL, gWVP);

	  // texture coordinate
	  outVS.TextureUV = vTexture;

      // Done--return the output.
      return outVS;
}

float4 BasicPS(OutputVS In) : COLOR
{
	float4 Output = tex2D(CubeTextureSampler, In.TextureUV);
	return Output;
}

technique Tech1
{
	pass p0
    {
		vertexShader = compile vs_2_0 BasicVS();
		pixelShader  = compile ps_2_0 BasicPS();
    }
}
