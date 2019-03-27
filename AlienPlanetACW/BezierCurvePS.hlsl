#define NUMBER_OF_LIGHTS 1

struct Light
{
	float3 ambientColour;
	float3 diffuseColour;
	float3 specularColour;
	float3 lightPosition;
	float specularPower;
};

static Light lights[NUMBER_OF_LIGHTS] = {
	//LightOne
	{0.2, 0.2, 0.2, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.0, 3.0, 0.0, 20},
	//LightTwo
	//{0.2, 0.2, 0.2, 1.0, 0.4, 0.4, 0.4, 1.0, 0.4, 0.4, 0.4, 1.0, 0.0, -5.0, 0.0, 20},
};

struct PixelShaderInput
{
	float4 positionH : SV_POSITION;
	float3 positionW : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 viewDirection : TEXCOORD0;
};

float4 PhongIllumination(float3 pos, float3 normal, float3 viewDir, float3 diffuse)
{
	float3 totalAmbient = float3(0.0f, 0.0f, 0.0f);
	float3 totalDiffuse = float3(0.0f, 0.0f, 0.0f);
	float3 totalSpecular = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
	{
		totalAmbient += lights[i].ambientColour * diffuse;

		float3 lightDirection = normalize(lights[i].lightPosition - pos);
		float nDotL = dot(normal, lightDirection);
		float3 reflection = normalize(reflect(-lightDirection, normal));
		float rDotV = max(0.0f, dot(reflection, viewDir));

		totalDiffuse += saturate(lights[i].diffuseColour * nDotL * diffuse);

		if (nDotL > 0.0f)
		{
			float3 specularIntensity = float3(1.0, 1.0, 1.0);
			totalSpecular += lights[i].specularColour * (pow(rDotV, lights[i].specularPower)) * specularIntensity;
		}
	}
	return float4(totalAmbient + totalDiffuse + totalSpecular, 1.0f);
}

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 colour = PhongIllumination(input.positionW, input.normal, input.viewDirection, float3(0.3735f, 0.6196f, 0.6275f));

	return colour;
}