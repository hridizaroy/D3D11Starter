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

float random2D(float2 s)
{
	return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
}

float random(float s)
{
	return frac(sin(s) * 43758.5453123);
}

float4 main(VertexToPixel input) : SV_TARGET
{
	// Voronoi noise
	float scale = 20.0f;
	float2 scaledUV = scale * input.uv;
	int2 uvFloor = floor(scaledUV);

	float minDist = 100.0f;
	for (float ii = -1.0; ii <= 1.0; ii += 1.0)
	{
		for (float jj = -1.0; jj <= 1.0; jj += 1.0)
		{
			int2 neighborFloor = uvFloor + float2(ii, jj);

			float noiseX = random2D(neighborFloor);
			float noiseY = random(noiseX);

			float2 voronoiPoint = (neighborFloor + float2(noiseX, noiseY));

			float dist = length(voronoiPoint - scaledUV);

			minDist = min(minDist, dist);
		}
	}

	return float4(minDist, 0.0, 0.0, 1.0);
}