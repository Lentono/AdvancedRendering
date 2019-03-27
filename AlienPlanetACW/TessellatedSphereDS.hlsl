//Globals
Texture2D displacementTexture : register(t0);
SamplerState sampleType : register(s0);

// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer CameraBuffer : register(b1)
{
	float3 cameraPosition;
	float cameraPadding;
};

cbuffer DisplacementPowerBuffer : register(b2)
{
	float displacementPower;
	float3 padding;
};

//Type definitions
struct PatchConstantOutput
{
	float edges[4] : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

struct DomainShaderInput
{
	float3 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

struct PixelShaderInput
{
	float4 positionH : SV_POSITION;
	float3 positionW : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 viewDirection : TEXCOORD1;
};

static float PI = 3.14159265359;

[domain("quad")]
PixelShaderInput main(in PatchConstantOutput input, in const float2 uvCoord : SV_DomainLocation, const OutputPatch<DomainShaderInput, 4> patch)
{
	PixelShaderInput output;

	//Calculate new vertex position
	output.positionW.x = 1.0f * cos((uvCoord.x + 0.5f) * (2 * PI)) * sin((uvCoord.y + 0.5f) * (2 * PI));
	output.positionW.y = 1.0f * sin((uvCoord.x + 0.5f) * (2 * PI)) * sin((uvCoord.y + 0.5f) * (2 * PI));
	output.positionW.z = 1.0f * cos((uvCoord.y + 0.5f) * (2 * PI));

	output.tex = uvCoord.x * patch[0].tex + uvCoord.y * patch[1].tex + uvCoord.x * patch[2].tex + uvCoord.y * patch[3].tex;
	output.normal = uvCoord.x * patch[0].normal + uvCoord.y * patch[1].normal + uvCoord.x * patch[2].normal + uvCoord.y * patch[3].normal;
	output.tangent = uvCoord.x * patch[0].tangent + uvCoord.y * patch[1].tangent + uvCoord.x * patch[2].tangent + uvCoord.y * patch[3].tangent;
	output.binormal = uvCoord.x * patch[0].binormal + uvCoord.y * patch[1].binormal + uvCoord.x * patch[2].binormal + uvCoord.y * patch[3].binormal;

	output.normal = normalize(output.normal);
	output.tangent = normalize(output.tangent);
	output.binormal = normalize(output.binormal);

	//Displacement mapping
	float mipLevel = clamp((distance(output.positionW, cameraPosition) - 20.0f) / 20.0f, 0.0f, 6.0f);

	//Sample height map
	float height = displacementTexture.SampleLevel(sampleType, output.tex, mipLevel).r;

	output.positionW += normalize(output.positionW) * (displacementPower * (height - 0.2));

	output.viewDirection = normalize(cameraPosition.xyz - output.positionW);

	output.positionH = mul(float4(output.positionW, 1.0f), model);
	output.positionH = mul(output.positionH, view);
	output.positionH = mul(output.positionH, projection);

	return output;
}