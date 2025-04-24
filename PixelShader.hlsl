#include "ShaderIncludes.hlsli"

#define MAX_LIGHTS (5)

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float2 uvScale;
	float2 uvOffset;

	float roughness;
	float3 camPos;

	Light lights[MAX_LIGHTS];
	int numLights;
}

Texture2D Albedo : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
Texture2D ShadowMap : register(t4);

SamplerState BasicSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

// Provided function for attenuation
float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.Position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
	return att * att;
}

float3 calculateLightContributions(float3 worldPos, float3 normal,
	float roughness, float metalness, float3 specColor, float3 surfaceColor,
	float shadowAmount)
{
	float3 totalContribution = float3(0.0, 0.0, 0.0);

	for (int ii = 0; ii < numLights; ii++)
	{
		// Diffuse
		float3 incomingLightDir = float3(0, 0, 0);

		if (lights[ii].Type == LIGHT_TYPE_DIRECTIONAL)
		{
			incomingLightDir = normalize(lights[ii].Direction);
		}
		else
		{
			incomingLightDir = normalize(worldPos - lights[ii].Position);
		}
		
		float diff = DiffusePBR(normal, -incomingLightDir);
		float3 toCam = normalize(camPos - worldPos);
		float3 F;
		float3 spec = MicrofacetBRDF(normal, -incomingLightDir, toCam, roughness, specColor, F);
		float3 balancedDiff = DiffuseEnergyConserve(diff, F, metalness);
		float3 contribution = (balancedDiff * surfaceColor + spec) * lights[ii].Intensity * lights[ii].Color;

		if (lights[ii].Type != LIGHT_TYPE_DIRECTIONAL)
		{
			contribution *= Attenuate(lights[ii], worldPos);

			if (lights[ii].Type == LIGHT_TYPE_SPOT)
			{
				// Provided code for spot light term calculation
				
				// Get cos(angle) between pixel and light direction
				float pixelAngle = saturate(dot(incomingLightDir, lights[ii].Direction));

				// Get cosines of angles and calculate range
				float cosOuter = cos(lights[ii].SpotOuterAngle);
				float cosInner = cos(lights[ii].SpotInnerAngle);
				float falloffRange = cosOuter - cosInner;

				// Linear falloff over the range, clamp 0-1, apply to light calc
				float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
				contribution *= spotTerm;
			}
		}

		// If this is the first light, apply the shadowing result
		if (ii == 4)
		{
			contribution *= shadowAmount;
		}

		totalContribution += contribution;
	}

	return totalContribution;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	input.shadowMapPos /= input.shadowMapPos.w;
	float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
	shadowUV.y = 1 - shadowUV.y;
	float distToLight = input.shadowMapPos.z;

	// Get a ratio of comparison results using SampleCmpLevelZero()
	float shadowAmount = ShadowMap.SampleCmpLevelZero(
		ShadowSampler,
		shadowUV,
		distToLight).r;
	
	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2.0 - 1.0;
	unpackedNormal = normalize(unpackedNormal);

	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	input.uv = input.uv * uvScale + uvOffset;
	float3 surfaceColor = (pow(Albedo.Sample(BasicSampler, input.uv), 2.2f) * colorTint).rgb;

	// Gram-Schmidt orthonormalize process
	float3 N = input.normal;
	float3 T = input.tangent;
	T = normalize(T - N * dot(T, N));
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	input.normal = mul(unpackedNormal, TBN);

	float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
	float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;

	// Specular color determination -----------------
	// Assume albedo texture is actually holding specular color where metalness == 1
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we lerp the specular color to match
	float3 specColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);

	float3 lightContributions = calculateLightContributions(
		input.worldPosition, input.normal, roughness, metalness, specColor, surfaceColor,
		shadowAmount);

	return float4(pow(lightContributions, 1.0f/2.2f), 1.0f);
}