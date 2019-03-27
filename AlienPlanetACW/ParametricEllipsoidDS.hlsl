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
	//output.positionW = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;

	output.positionW.x = 1.0f * cos((uvCoord.x + 0.5f) * (2 * PI)) * sin((uvCoord.y + 0.5f) * (2 * PI));
	output.positionW.y = 1.0f * sin((uvCoord.x + 0.5f) * (2 * PI)) * sin((uvCoord.y + 0.5f) * (2 * PI));
	output.positionW.z = 0.5f * cos((uvCoord.y + 0.5f) * (2 * PI));

	output.tex = uvCoord.x * patch[0].tex + uvCoord.y * patch[1].tex + uvCoord.x * patch[2].tex + uvCoord.y * patch[3].tex;
	output.normal = uvCoord.x * patch[0].normal + uvCoord.y * patch[1].normal + uvCoord.x * patch[2].normal + uvCoord.y * patch[3].normal;
	output.tangent = uvCoord.x * patch[0].tangent + uvCoord.y * patch[1].tangent + uvCoord.x * patch[2].tangent + uvCoord.y * patch[3].tangent;
	output.binormal = uvCoord.x * patch[0].binormal + uvCoord.y * patch[1].binormal + uvCoord.x * patch[2].binormal + uvCoord.y * patch[3].binormal;

	//output.tex = output.tex * 2.0f;

	output.normal = normalize(output.normal);
	output.tangent = normalize(output.tangent);
	output.binormal = normalize(output.binormal);

	//float height = noise(output.positionW);

	//output.positionW += height * output.normal;

	output.viewDirection = normalize(cameraPosition.xyz - output.positionW);

	output.positionH = mul(float4(output.positionW, 1.0f), model);
	output.positionH = mul(output.positionH, view);
	output.positionH = mul(output.positionH, projection);

	return output;
}