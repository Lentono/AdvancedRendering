cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

struct VertexShaderInput
{
	float3 position : POSITION;
};

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float2 canvasXY : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	output.position = float4(sign(input.position.xy), 0, 1);

	float aspectRatio = projection._m00 / projection._m11;
	output.canvasXY = sign(input.position.xy) * float2(1.0, aspectRatio);

	return output;
}