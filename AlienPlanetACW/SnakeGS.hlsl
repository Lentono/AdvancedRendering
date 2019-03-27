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

cbuffer SnakeProperties : register(b3)
{
	float radius;
	float length;
	float segments;
	int directionZ;
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

static float PI = 3.14159265359;

[maxvertexcount(100)]
void main(line GeometryShaderInput input[2], inout TriangleStream<PixelShaderInput> outputStream)
{
	PixelShaderInput output;

	float topRadius = radius;
	float bottomRadius = radius;

	float3 posOne = input[0].position.xyz;
	float3 posTwo = input[0].position.xyz;

	input[0].position = mul(input[0].position, model);
	input[1].position = mul(input[1].position, model);

	float tDNoise = noise(input[0].position.xyz);

	input[0].position.y = noise(input[0].position.xyz) + topRadius;
	input[1].position.y = noise(input[1].position.xyz) + bottomRadius;

	float movementO = sin((time * 5) * noise(noise(input[0].position.xyz))) * 0.01f;
	float movementT = sin((time * 5) * noise(noise(input[1].position.xyz))) * 0.01f;

	float3 P1 = input[0].position.xyz;
	float3 P2 = input[1].position.xyz;

	float3 cylinderAxis = normalize(P2 - P1);

	float3 A = (float3)(0.0f);
	float3 B = (float3)(0.0f);

	if (directionZ)
	{
		A = cross(cylinderAxis, float3(1.0f, 0.0f, 0.0f));
		B = cross(cylinderAxis, float3(0.0f, 1.0f, 0.0f));

		if (posOne.z <= length / segments)
		{
			topRadius = 0.0f;
		}

		if (posOne.z > length - (length / segments))
		{
			bottomRadius = 0.0f;
		}
	}
	else
	{
		//This could be wrong
		A = cross(cylinderAxis, float3(0.0f, 0.0f, 1.0f));
		B = cross(cylinderAxis, float3(0.0f, 1.0f, 0.0f));

		if (posTwo.x <= length / segments)
		{
			topRadius = 0.0f;
		}

		if (posTwo.x > length - (length / segments))
		{
			bottomRadius = 0.0f;
		}
	}

	for (int i = 0; i < 10; i++)
	{
		float n = 0;
		float theta1 = i * 2*PI / 10;
		float theta2 = (i + 1) * 2 * PI / 10;


		float3 pointOne = P1 + topRadius * cos(theta1) * A + topRadius * sin(theta1) * B;
		float3 pointTwo = P2 + bottomRadius * cos(theta1) * A + bottomRadius * sin(theta1) * B;
		float3 pointThree = P2 + bottomRadius * cos(theta2) * A + bottomRadius * sin(theta2) * B;
		float3 pointFour = P1 + topRadius * cos(theta2) * A + topRadius * sin(theta2) * B;

		//pointOne = mul(pointOne, model);
		//pointTwo = mul(pointTwo, model);
		//pointThree = mul(pointThree, model);
		//pointFour = mul(pointFour, model);

		if (directionZ)
		{
			pointOne.x += movementO;
			pointTwo.x += movementT;
			pointThree.x += movementT;
			pointFour.x += movementO;
		}
		else
		{
			pointOne.z += movementO;
			pointTwo.z += movementT;
			pointThree.z += movementT;
			pointFour.z += movementO;
		}

		float texCoordTest = (1.0f / 12) * i;

		output.positionW = pointOne;
		output.positionH = mul(float4(pointOne, 1.0f), view);
		output.positionH = mul(output.positionH, projection);
		output.texcoord = float2(0.0f, 0.0f);

		outputStream.Append(output);

		output.positionW = pointTwo;
		output.positionH = mul(float4(pointTwo, 1.0f), view);
		output.positionH = mul(output.positionH, projection);
		output.texcoord = float2(0.1f, 0.0f);

		outputStream.Append(output);

		output.positionW = pointThree;
		output.positionH = mul(float4(pointThree, 1.0f), view);
		output.positionH = mul(output.positionH, projection);
		output.texcoord = float2(0.1f, 0.1f);

		outputStream.Append(output);

		outputStream.RestartStrip();

		output.positionW = pointOne;
		output.positionH = mul(float4(pointOne, 1.0f), view);
		output.positionH = mul(output.positionH, projection);
		output.texcoord = float2(0.0f, 0.0f);

		outputStream.Append(output);

		output.positionW = pointThree;
		output.positionH = mul(float4(pointThree, 1.0f), view);
		output.positionH = mul(output.positionH, projection);
		output.texcoord = float2(0.1f, 0.1f);

		outputStream.Append(output);

		output.positionW = pointFour;
		output.positionH = mul(float4(pointFour, 1.0f), view);
		output.positionH = mul(output.positionH, projection);
		output.texcoord = float2(0.0f, 0.1f);

		outputStream.Append(output);

		outputStream.RestartStrip();
	}


	//float3 tDNoise = noise(position);
	//position.y += tDNoise.y;
	////tDNoise.xz /= tDNoise.xz;

	//output.positionW = mul(position, model);
	//position = mul(position, model);

	////	position = mul(position, sign(time * tDNoise.xz));

	//position = mul(position, view);

	//static float3 positions[4] =
	//{
	//	float3(-0.02,0.25,0),
	//	float3(0.02,0.25,0),
	//	float3(-0.02,-0.25,0),
	//	float3(0.02,-0.25,0)
	//};


	//float movement = sin((time * 4) * noise(float3(tDNoise.x, tDNoise.y, tDNoise.z))) * 0.1f;

	//positions[0].x += movement;
	//positions[1].x += movement;

	///*Triange 1*/

	////Vertex1
	//float quadSize = 0.1;
	//output.positionH = position + float4(quadSize*positions[0].x, quadSize*positions[0].y, quadSize*positions[0].z, 0.0);
	//output.positionH = mul(output.positionH, projection);

	////output.texcoord = (sign(input[0].position.xy) + 1.0) / 2.0;

	//outputStream.Append(output);

	////Vertex2
	//output.positionH = position + float4(quadSize*positions[1].x, quadSize*positions[1].y, quadSize*positions[1].z, 0.0);
	//output.positionH = mul(output.positionH, projection);


	//outputStream.Append(output);

	////Vertex3
	//output.positionH = position + float4(quadSize*positions[2], 0.0);
	//output.positionH = mul(output.positionH, projection);

	////output.texcoord = (sign(input[0].position.xy) + 1.0) / 2.0;
	//output.texcoord = float2(0.0, 0.0f);

	//outputStream.Append(output);

	//outputStream.RestartStrip();

	///*Triangle 2*/

	////Vertex1
	//output.positionH = position + float4(quadSize*positions[1].x, quadSize*positions[1].y, quadSize*positions[1].z, 0.0);
	//output.positionH = mul(output.positionH, projection);

	////output.texcoord = (sign(input[0].position.xy) + 1.0) / 2.0;
	//output.texcoord = float2(1.0, 1.0);

	//outputStream.Append(output);

	////Vertex2
	//output.positionH = position + float4(quadSize*positions[3], 0.0);
	//output.positionH = mul(output.positionH, projection);

	////output.texcoord = (sign(input[0].position.xy) + 1.0) / 2.0;
	//output.texcoord = float2(1.0, 0.0);

	//outputStream.Append(output);

	////Vertex3
	//output.positionH = position + float4(quadSize*positions[2], 0.0);
	//output.positionH = mul(output.positionH, projection);

	////output.texcoord = (sign(input[0].position.xy) + 1.0) / 2.0;
	//output.texcoord = float2(0.0, 0.0);

	//outputStream.Append(output);
}