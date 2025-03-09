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
	float scale = 20.0f;
	float2 scaledUV = scale * input.uv;
	int2 uvFloor = floor(scaledUV);

	if (uvFloor.x % 2 != uvFloor.y % 2)
	{
		return float4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		return float4(1.0, 1.0, 0.0, 1.0);
	}
}