// World matrix
float4x4 World;

// World, View and Projection Matrix
float4x4 gWVP; 

// Face texture
Texture2D CubeTexture;

// Eye position 
float3 EyePosition;

// Face texture sampler
SamplerState CubeTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

// Light information
struct LightInfo
{
	float3 pos;	// position
	float3 dir; // direction
	float4 ambient; 
	float4 diffuse;
	float4 specular;
	float3 att;			// attenuation parameters (a0, a1 and a2), for spot and point light only
	float spotPower;	// power factor for spot light
	float range;		// Not available for directional light
};

// Surface information, server as material
struct SurfaceInfo
{
	float3 pos;		// position
	float3 normal;	
	float4 diffuse;
	float4 specular;
};

// Input vertex structure
struct InputVS
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 texUV : TEXCOORD0;
};

// Output Vertex structure
struct OutputVS
{
	float4 posH : SV_Position;
	float4 color : COLOR;
	float2 texUV : TEXCOORD0;
};

// Directional light
float4 ParallelLight(SurfaceInfo surface, LightInfo light, float3 eyePos)
{
	// The final result color
	float4 litColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// The light vector aims opposite the direction the light ray travel
	float3 lightVec = -light.dir;

	// Add the ambient term. why here use *, but not +?
	litColor += surface.diffuse * light.ambient;

	float diffuseFactor = max(dot(lightVec, surface.normal), 0);

	[branch]
	if (diffuseFactor > 0.0f)
	{
		float specPower = max(surface.specular.a, 1.0f);

		// Vector from reflection point to eye position
		float3 toEye = normalize(eyePos - surface.pos);

		// Calculate the reflection vector of light ray
		float3 R = reflect(light.dir, surface.normal);

		float4 specFactor = pow(max(dot(R, toEye), 0.0f), specPower);

		// Add diffuse color 
		litColor += diffuseFactor * surface.diffuse * light.diffuse;

		// Add specular color
		litColor += specFactor * surface.specular * light.specular;
	}

	return litColor;
}

// Point light
float4 PointLight(SurfaceInfo surface, LightInfo light, float3 eyePos)
{
	float4 litColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// Vector from surface to point light
	float3 lightVec = light.pos - surface.pos;

	// Distance from surface to light
	float d = length(lightVec);

	// If point light too far from surface(out of light.range), no light recieved
	if (d > light.range)
		return float4(0.0f, 0.0f, 0.0f, 1.0f);

	// Normalize the light vector
	lightVec = normalize(lightVec);
	// lightVec /= d;

	// Add the ambient light term
	litColor += surface.diffuse * light.ambient;

	float diffuseFactor = dot(lightVec, surface.normal);

//	[branch]
	if (diffuseFactor > 0.0f)
	{
		float specPower = max(surface.specular.a, 1.0f);
		float3 toEye    = normalize(eyePos - surface.pos);
		float3 R        = reflect(light.dir, surface.normal);
		float specFactor= pow(max(dot(R, toEye), 0.0f), specPower);

		// Add diffuse color
		litColor += diffuseFactor * surface.diffuse * light.diffuse;

		// Add specular color
		litColor += specFactor * surface.specular * light.specular;
	}

	return litColor / dot(light.att, float3(1.0f, d, d * d));
}

// Vertex shader
OutputVS BasicVS(InputVS inputVS)
{
	// Zero out our output.
	OutputVS outVS = (OutputVS)0;

	// Transform to homogeneous clip space.
	outVS.posH = mul(float4(inputVS.pos, 1.0f), gWVP);

	// texture coordinate
	outVS.texUV = inputVS.texUV;

	// Transfer vertex normal to world space and normalize it.
	// float4 normalW = normalize(mul(float4(inputVS.normal, 1.0f), World));
	float3 normalW = normalize(mul(inputVS.normal, (float3x3)World));

	// Create a surface(material)
	SurfaceInfo surfaceInfo;
	surfaceInfo.pos      = inputVS.pos;
	surfaceInfo.normal   = normalW;
	surfaceInfo.diffuse  = float4(1.0f, 1.0f, 1.0f, 1.0f);
	surfaceInfo.specular = float4(1.0f, 1.0f, 1.0f, 1.0f);

	// Create a light 
	LightInfo lightInfo;
	// lightInfo.pos      = EyePosition; // not used in parallel light
	lightInfo.dir	   = float3(0.0f, 0.0f, 0.0f); // not used in point light
	// lightInfo.range    = 250.0f; // not used in parallel light
	// lightInfo.att.x    = 0.0f;
	// lightInfo.att.y    = 0.05f;
	// lightInfo.att.z    = 0.0f;
	// lightInfo.spotPower= 0.0f; // not use in point light or parallel light
	lightInfo.ambient  = float4(1.0f, 1.0f, 1.0f, 1.0f);
	lightInfo.diffuse  = float4(1.0f, 1.0f, 0.0f, 1.0f);
	lightInfo.specular = float4(1.0f, 1.0f, 0.0f, 1.0f);

	// Point light
	// float3 litColor = PointLight(surfaceInfo, lightInfo, EyePosition);
	float3 litColor = ParallelLight(surfaceInfo, lightInfo, EyePosition);
	outVS.color = float4(litColor, 1.0f);

	// Done--return the output.
	return outVS;
}

// Pixel shader
float4 BasicPS(OutputVS outputVS) : SV_Target
{
	float4 Output;

	Output = CubeTexture.Sample(CubeTextureSampler, outputVS.texUV);

	return Output;
}

technique10 Render
{
	pass p0
    {
		SetVertexShader(CompileShader(vs_4_0, BasicVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, BasicPS()));
    }
}
