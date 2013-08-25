// World matrix
uniform extern float4 World;

// World, View and Projection Matrix
uniform extern float4x4 gWVP; 

// Face texture
texture FaceTexture;

// Inner texture
texture InnerTexture;

// Whether current texture is face texture or inner texture?
bool Is_Face_Texture;

// Eye position 
float3 EyePosition;

// Face texture sampler
sampler FaceTextureSampler = sampler_state
{
	Texture   = <FaceTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

// Inner texture sampler
sampler InnerTextureSampler = sampler_state
{
	Texture   = <InnerTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
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
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 texUV : TEXCOORD0;
};

// Output Vertex structure
struct OutputVS
{
	float4 posH : POSITION;
	float4 color : COLOR;
	float2 texUV : TEXCOORD0;
};

// Directional light
float3 ParallelLight(SurfaceInfo surface, LightInfo light, float3 eyePos)
{
	// The final result color
	float3 litColor = float3(0.0f, 0.0f, 0.0f);

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

		float3 specFactor = pow(max(dot(R, toEye), 0.0f), specPower);

		// Add diffuse color 
		litColor += diffuseFactor * surface.diffuse * light.diffuse;

		// Add specular color
		litColor += specFactor * surface.specular * light.specular;
	}

	return litColor;
}

// Point light
float3 PointLight(SurfaceInfo surface, LightInfo light, float3 eyePos)
{
	float3 litColor = float3(0.0f, 0.0f, 0.0f);

	// Vector from surface to point light
	float3 lightVec = light.pos - surface.pos;

	// Distance from surface to light
	float d = length(lightVec);

	// If point light too far from surface(out of light.range), no light recieved
	if (d > light.range)
		return float3(0.0f, 0.0f, 0.0f);

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
	outVS.posH = mul(inputVS.pos, gWVP);

	// texture coordinate
	outVS.texUV = inputVS.texUV;

	// Transfer vertex normal to world space and normalize it.
	float4 normalW = normalize(mul(float4(inputVS.normal, 1.0f), World));

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
	/*float3 litColor = ParallelLight(surfaceInfo, lightInfo, EyePosition);

	outVS.color = float4(litColor, 1.0f);*/

	// Done--return the output.
	return outVS;
}

// Pixel shader
float4 BasicPS(OutputVS outputVS) : COLOR
{
	float4 Output;

	if (Is_Face_Texture)
	{
		Output = /*outputVS.color * */tex2D(FaceTextureSampler, outputVS.texUV);
	}
	else
	{
		Output = /*outputVS.color * */tex2D(InnerTextureSampler, outputVS.texUV);
	}

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
