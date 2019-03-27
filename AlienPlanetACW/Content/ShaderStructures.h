#pragma once

namespace AlienPlanetACW
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	struct InverseViewConstantBuffer
	{
		DirectX::XMFLOAT4X4 inverseView;
	};

	struct CameraPositionConstantBuffer
	{
		DirectX::XMFLOAT3 position;
		float padding;
	};

	struct TotalTimeConstantBuffer
	{
		float time;
		DirectX::XMFLOAT3 padding;
	};

	struct DeltaTimeConstantBuffer
	{
		float dt;
		DirectX::XMFLOAT3 padding;
	};

	struct TessellationFactorConstantBuffer 
	{
		float tessellationFactor;
		DirectX::XMFLOAT3 padding;
	};

	struct DisplacementPowerConstantBuffer 
	{
		float displacementPower;
		DirectX::XMFLOAT3 padding;
	};

	struct VertexPosition
	{
		DirectX::XMFLOAT3 position;
	};

	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 colour;
	};

	struct VertexPositionTexcoordNormal
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texcoord;
		DirectX::XMFLOAT3 normal;
	};

	struct VertexPositionTexcoordNormalTangentBinormal
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texcoord;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 binormal;
	};
}