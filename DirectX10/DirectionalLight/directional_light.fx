//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

matrix World;
matrix View;
matrix Projection;
matrix WorldViewProj;

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

LightInfo gLight;
float3	  gEyePoint;	// Eye position used in lighting calculation

struct SurfaceInfo
{
	float3 pos;		// position
	float3 normal;	
	float4 diffuse;
	float4 specular;
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

struct VS_INPUT
{
    float3 pos : POSITION;
	float3 normal : NORMAL;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float3 posW : POSITION; // Position in world space
	float3 normalW : NORMAL; // normal in world space
	float4 posH : SV_POSITION; // position in homogenious space(transformed by worl/view/projection matrix
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

	// Transform vertex position to world space
    output.posW = mul(float4(input.pos, 1.0f), World);

	// Transform vertex normal to world space
	output.normalW = mul(float4(input.normal, 0.0f), World);

	// Transform to homogeneous clip space.
	output.posH = mul(float4(input.pos, 1.0f), WorldViewProj);

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT psinput) : SV_Target
{
	// Normalize the vertex normal.
	psinput.normalW = normalize(psinput.normalW);

	float4 diffuse = (1.0f, 1.0f, 1.0f, 1.0f);
	float4 specular = (1.0f, 1.0f, 1.0f, 1.0f);
	SurfaceInfo surface = {psinput.posW, psinput.normalW, diffuse, specular};

	// Apply directional light
	float3 litColor = ParallelLight(surface, gLight, gEyePoint);

	return float4(litColor, 1.0f);
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
