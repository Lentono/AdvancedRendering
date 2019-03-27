#define NUMBER_OF_LIGHTS 1

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState sampleType : register(s0);

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

struct Light
{
	float4 ambientColour;
	float4 diffuseColour;
	float4 specularColour;
	float3 lightPosition;
	float specularPower;
};

static Light lights[NUMBER_OF_LIGHTS] = {
	//LightOne
	{0.2, 0.2, 0.2, 1.0, 0.4, 0.4, 0.4, 1.0, 0.4, 0.4, 0.4, 1.0, 0.0, 3.0, 0.0, 20},
	//LightTwo
	//{0.2, 0.2, 0.2, 1.0, 0.4, 0.4, 0.4, 1.0, 0.4, 0.4, 0.4, 1.0, 0.0, -5.0, 0.0, 20},
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

float4 PhongIllumination(float3 pos, float3 normal, float3 viewDir, float2 texcoord)
{
	float4 totalAmbient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 totalDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 totalSpecular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
	{
		float4 baseColour = diffuseTexture.Sample(sampleType, texcoord);

		totalAmbient += lights[i].ambientColour * baseColour;

		float3 lightDirection = normalize(lights[i].lightPosition - pos);
		float nDotL = dot(normal, lightDirection);
		float3 reflection = normalize(reflect(-lightDirection, normal));
		//float3 reflection = normalize(((2.0f * normal) * nDotL) - lightDirection);
		//float3 eyeDirection = normalize(mul(float4(eye, 1.0f), view).xyz - pos);
		float rDotV = max(0.0f, dot(reflection, viewDir));

		totalDiffuse += saturate(lights[i].diffuseColour * nDotL * baseColour);

		if (nDotL > 0.0f)
		{
			float4 specularIntensity = float4(1.0, 1.0, 1.0, 1.0);
			totalSpecular += lights[i].specularColour * (pow(rDotV, lights[i].specularPower)) * specularIntensity;
		}
	}
	return totalAmbient + totalDiffuse + totalSpecular;
}

float4 main(PixelShaderInput input) : SV_TARGET
{
	//Bump Normal Calc
	float4 bumpMap = normalTexture.Sample(sampleType, input.tex);

	//Range is held in [0,1], need to expand that to [-1,1]
	bumpMap = (bumpMap * 2.0f) - 1.0f;

	//Calculate our normal
	float3 bumpNormal = normalize((bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal));

	float4 colour = PhongIllumination(input.positionW, bumpNormal, input.viewDirection, input.tex);

	return colour;
}