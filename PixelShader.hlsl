#include "ShaderIncludes.hlsli"

#define MAX_LIGHTS (5)

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float2 uvScale;
	float2 uvOffset;

	float roughness;
	float3 camPos;

	float3 ambient;

	Light lights[MAX_LIGHTS];
	int numLights;
}

Texture2D SurfaceTexture : register(t0);
Texture2D NormalMap : register(t1);
SamplerState BasicSampler : register(s0);

// Provided function for attenuation
float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.Position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
	return att * att;
}

float3 calculateLightContributions(float3 worldPos, float3 normal)
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

		float3 diffuse =
			lights[ii].Intensity *
			saturate(dot(-incomingLightDir, normal)) *
			lights[ii].Color;

		// Specular
		float spec = 0.0f;
		float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;

		if (specExponent > 0.05)
		{
			float3 V = normalize(camPos - worldPos);
			float3 R = reflect(incomingLightDir, normal);
			spec = pow(saturate(dot(R, V)), specExponent);
		}

		spec *= any(diffuse);

		float3 specular = lights[ii].Color * spec;

		float3 contribution = (diffuse + specular);

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
	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2.0 - 1.0;
	unpackedNormal = normalize(unpackedNormal);

	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	input.uv = input.uv * uvScale + uvOffset;
	float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv) * colorTint;

	// Gram-Schmidt orthonormalize process
	float3 N = input.normal;
	float3 T = input.tangent;
	T = normalize(T - N * dot(T, N));
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	input.normal = mul(unpackedNormal, TBN);

	float4 finalColor = float4(ambient, 1.0);

	float3 lightContributions = calculateLightContributions(input.worldPosition, input.normal);
	finalColor += float4(lightContributions, 1.0);

	finalColor *= surfaceColor;

	return finalColor;
}