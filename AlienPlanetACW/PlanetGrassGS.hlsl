cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer timeConstantBuffer : register(b1)
{
	float time;
	float3 padding;
}

cbuffer CameraBuffer : register(b2)
{
	float3 cameraPosition;
	float cameraPadding;
};

struct GeometryShaderInput
{
	float4 position : SV_POSITION;
};

struct PixelShaderInput
{
	float4 positionH : SV_POSITION;
	float3 positionW : POSITION;
	float2 texcoord : TEXCOORD0;
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

[maxvertexcount(6)]
void main(point GeometryShaderInput input[1], inout TriangleStream<PixelShaderInput> outputStream)
{
	PixelShaderInput output;

	float4 position = input[0].position;

	float3 tDNoise = noise(position);
	position.y += tDNoise.y;
	//tDNoise.xz /= tDNoise.xz;

	output.positionW = mul(position, model);
	position = mul(position, model);

//	position = mul(position, sign(time * tDNoise.xz));

	position = mul(position, view);

	static float3 positions[4] =
	{
		float3(-0.02,0.25,0),
		float3(0.02,0.25,0),
		float3(-0.02,-0.25,0),
		float3(0.02,-0.25,0)
	};


	float movement = sin((time * 4) * noise(float3(tDNoise.x, tDNoise.y, tDNoise.z))) * 0.1f;

	positions[0].x += movement;
	positions[1].x += movement;

	/*Triange 1*/

	//Vertex1
	float quadSize = 0.1;
	output.positionH = position + float4(quadSize*positions[0].x, quadSize*positions[0].y, quadSize*positions[0].z, 0.0);
	output.positionH = mul(output.positionH, projection);

	//output.texcoord = (sign(input[0].position.xy) + 1.0) / 2.0;
	output.texcoord = float2(0.0, 1.0f);

	outputStream.Append(output);

	//Vertex2
	output.positionH = position + float4(quadSize*positions[1].x, quadSize*positions[1].y, quadSize*positions[1].z, 0.0);
	output.positionH = mul(output.positionH, projection);

	//output.texcoord = (sign(input[0].position.xy) + 1.0) / 2.0;
	output.texcoord = float2(1.0, 1.0);

	outputStream.Append(output);

	//Vertex3
	output.positionH = position + float4(quadSize*positions[2], 0.0);
	output.positionH = mul(output.positionH, projection);

	//output.texcoord = (sign(input[0].position.xy) + 1.0) / 2.0;
	output.texcoord = float2(0.0, 0.0f);

	outputStream.Append(output);

	outputStream.RestartStrip();

	/*Triangle 2*/

	//Vertex1
	output.positionH = position + float4(quadSize*positions[1].x, quadSize*positions[1].y, quadSize*positions[1].z, 0.0);
	output.positionH = mul(output.positionH, projection);

	//output.texcoord = (sign(input[0].position.xy) + 1.0) / 2.0;
	output.texcoord = float2(1.0, 1.0);

	outputStream.Append(output);

	//Vertex2
	output.positionH = position + float4(quadSize*positions[3], 0.0);
	output.positionH = mul(output.positionH, projection);

	//output.texcoord = (sign(input[0].position.xy) + 1.0) / 2.0;
	output.texcoord = float2(1.0, 0.0);

	outputStream.Append(output);

	//Vertex3
	output.positionH = position + float4(quadSize*positions[2], 0.0);
	output.positionH = mul(output.positionH, projection);

	//output.texcoord = (sign(input[0].position.xy) + 1.0) / 2.0;
	output.texcoord = float2(0.0, 0.0);

	outputStream.Append(output);
}