//Globals
Texture2D displacementTexture : register(t0);
SamplerState sampleType : register(s0);

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer TimeConstantBuffer : register(b1)
{
	float time;
	float3 timePadding;
}

cbuffer CameraBuffer : register(b2)
{
	float3 cameraPosition;
	float cameraPadding;
};

//Type definitions
struct PatchConstantOutput
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
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

/*Perlin Noise 1*/
//Hash based 3D Value Noise
//Ported From https://www.shadertoy.com/view/XslGRr
//Created by iq (Inigo Quilez)

float hash(float n)
{
	return frac(sin(n)*43758.5453);
}

float noise(float3 x)
{
	float3 p = floor(x);
	float3 f = frac(x);

	f = f * f*(3.0 - 2.0*f);
	float n = p.x + p.y*57.0 + 113.0*p.z;

	return lerp(lerp(lerp(hash(n + 0.0), hash(n + 1.0), f.x),
		lerp(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
		lerp(lerp(hash(n + 113.0), hash(n + 114.0), f.x),
			lerp(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
}

[domain("tri")]
PixelShaderInput main(in PatchConstantOutput input, in const float3 uvwCoord : SV_DomainLocation, const OutputPatch<DomainShaderInput, 3> patch)
{
	PixelShaderInput output;

	//Calculate new vertex position
	output.positionW = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;
	output.tex = uvwCoord.x * patch[0].tex + uvwCoord.y * patch[1].tex + uvwCoord.z * patch[2].tex;
	output.normal = uvwCoord.x * patch[0].normal + uvwCoord.y * patch[1].normal + uvwCoord.z * patch[2].normal;
	output.tangent = uvwCoord.x * patch[0].tangent + uvwCoord.y * patch[1].tangent + uvwCoord.z * patch[2].tangent;
	output.binormal = uvwCoord.x * patch[0].binormal + uvwCoord.y * patch[1].binormal + uvwCoord.z * patch[2].binormal;

	output.normal = normalize(output.normal);
	output.tangent = normalize(output.tangent);
	output.binormal = normalize(output.binormal);


	//Displacement mapping
	float mipLevel = clamp((distance(output.positionW, cameraPosition) - 20.0f) / 20.0f, 0.0f, 6.0f);

	//Sample height map
	float height = displacementTexture.SampleLevel(sampleType, output.tex, mipLevel).r * sin(time * 2);

	output.positionW.y += sin(time * 2) * cos(output.positionW.x) * 0.01f;

	output.positionW += 0.02f * height * output.normal;

	//float noiseValue = noise(output.positionW * 100);

	//output.positionW.y += noiseValue * sin(time * 2) * output.normal;
	float noiseValue = noise(output.positionW * 10);
	output.positionW.z += noiseValue * cos(time * 2) * 0.1f;
	output.positionW.y += (noiseValue * 0.02f) * sin(time);

	output.viewDirection = normalize(cameraPosition.xyz - output.positionW);

	output.positionH = mul(float4(output.positionW, 1.0f), view);
	output.positionH = mul(output.positionH, projection);

	return output;
}