struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

cbuffer ExternalData : register(b0)
{
	float4 colorTint;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(input.uv, 0.0f, 1.0f);
}