// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer CameraPositionConstantBuffer : register(b1)
{
	float3 cameraPosition;
	float padding;
}

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

// Per-pixel color data passed through the pixel shader.
struct HullShaderInput
{
	//float4 position : SV_POSITION;
	float3 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float tessellationFactor : TESS;
};

// Simple shader to do vertex processing on the GPU.
HullShaderInput main(VertexShaderInput input)
{
	HullShaderInput output;

	output.position = input.position;
	output.tex = input.tex;
	output.normal = normalize(mul(input.normal, (float3x3)model));
	output.tangent = normalize(mul(input.tangent, (float3x3)model));
	output.binormal = normalize(mul(input.binormal, (float3x3)model));

	float3 modelPosition = mul(float4(output.position, 1.0f), model).xyz;

	float distanceToCamera = distance(modelPosition, cameraPosition);

	float tess = saturate((3.0f - distanceToCamera) / (3.0f - 1.0f));

	output.tessellationFactor = 1.0f + tess * (64.0f - 1.0f);

	return output;
}