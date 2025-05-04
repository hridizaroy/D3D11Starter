/*
	Code adapted from https://github.com/lettier/3d-game-shaders-for-beginners/blob/master/demonstration/shaders/fragment/chromatic-aberration.frag
*/

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float redOffset = 0.009;
	float greenOffset = 0.006;
	float blueOffset = -0.006;

	float2 direction = input.uv;

	float4 fragColor = Pixels.Sample(ClampSampler, input.uv);

	fragColor.r = Pixels.Sample(ClampSampler, input.uv + (direction * redOffset)).r;
	fragColor.g = Pixels.Sample(ClampSampler, input.uv + (direction * greenOffset)).g;
	fragColor.b = Pixels.Sample(ClampSampler, input.uv + (direction * blueOffset)).b;

	return fragColor;
}