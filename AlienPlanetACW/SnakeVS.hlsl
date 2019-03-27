// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 position : POSITION;
};

// Per-pixel color data passed through the pixel shader.
struct GeometryShaderInput
{
	float4 position : SV_POSITION;
	//float3 position : POSITION;
};

// Simple shader to do vertex processing on the GPU.
GeometryShaderInput main(VertexShaderInput input)
{
	GeometryShaderInput output;

	//input.position.y = noise(input.position);

	output.position = float4(input.position, 1.0f);

	return output;
}