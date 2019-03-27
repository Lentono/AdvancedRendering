//Globals
Texture2D shaderTexture;
SamplerState sampleType;

#define NUMBER_OF_LIGHTS 1

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

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 positionH : SV_POSITION;
	float3 positionW : POSITION;
	float2 texcoord : TEXCOORD0;
};

float4 PhongIllumination(float3 pos, float3 normal, float3 viewDir, float4 diffuse)
{
	float4 totalAmbient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 totalDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 totalSpecular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
	{
		totalAmbient += lights[i].ambientColour * diffuse;

		float3 lightDirection = normalize(lights[i].lightPosition - pos);
		float nDotL = dot(normal, lightDirection);
		float3 reflection = normalize(reflect(-lightDirection, normal));
		//float3 reflection = normalize(((2.0f * normal) * nDotL) - lightDirection);
		//float3 eyeDirection = normalize(mul(float4(eye, 1.0f), view).xyz - pos);
		float rDotV = max(0.0f, dot(reflection, viewDir));

		totalDiffuse += saturate(lights[i].diffuseColour * nDotL * diffuse);

		if (nDotL > 0.0f)
		{
			float4 specularIntensity = float4(1.0, 1.0, 1.0, 1.0);
			totalSpecular += lights[i].specularColour * (pow(rDotV, lights[i].specularPower)) * specularIntensity;
		}
	}
	return totalAmbient + totalDiffuse + totalSpecular;
}

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float range = 0.5f * 0.5f - dot(input.texcoord - 0.5f, input.texcoord - 0.5f);

	float shade = 2.0f / (1.0f + exp(12 * range));

	//Receive pixel sample from the texture2D using the samplerstate and receive the colours needed
	//float3 textureColour = shaderTexture.Sample(sampleType, input.tex).rgb;

	if (input.positionW.y > 0.75f)
	{
		discard;
	}

	if (input.positionW.y < 0.35f)
	{
		discard;
	}

	float4 colour = saturate(float4((1 - shade) * float3(0.6f, 1.0f - input.positionW.y, 0.8f), 1.0f));

	if ((colour.x + colour.y + colour.z / 3) < 0.5f)
	{
		discard;
	}

	float t = input.positionH.z / input.positionH.w;

	return float4(lerp(colour.xyz, float3(1.0f, 0.97255f, 0.86275f), exp(-5.0*t)), 1.0f);
	//return float4((1 - shade) * float3(1.0, 0.0f, 0.0f), 1.0f);
}