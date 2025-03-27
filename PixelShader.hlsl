#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float2 uvScale;
	float2 uvOffset;

	float roughness;
	float3 camPos;
}

Texture2D SurfaceTexture : register(t0);
SamplerState BasicSampler : register(s0);

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
	input.uv = input.uv * uvScale + uvOffset;
	float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv);
	float4 finalColor = surfaceColor * colorTint;

	return float4(roughness.rrr, 1);

	return finalColor;
}