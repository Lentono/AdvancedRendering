#pragma once

#include "..\Common\DeviceResources.h"
#include "..\Common\DirectXHelper.h"
#include "..\Content\ShaderStructures.h"
#include "..\Common\StepTimer.h"

#include "ResourceManager.h"
#include <vector>
#include <DirectXMath.h>

namespace AlienPlanetACW
{
	class Snake
	{
	public:
		Snake(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<ResourceManager>& resourceManager, DirectX::XMFLOAT3 position, float radius, int length, int segments, bool directionZ);
		void CreateDeviceDependentResources();
		void SetViewProjectionMatrixConstantBuffer(DirectX::XMMATRIX& view, DirectX::XMMATRIX& projection);
		void SetCameraPositionConstantBuffer(DirectX::XMFLOAT3& cameraPosition);
		void ReleaseDeviceDependentResources();

		void Update(DX::StepTimer const& timer);
		void Render();

	private:
		struct SnakePropertiesConstantBuffer
		{
			float radius;
			float length;
			float segments;
			int directionZ;
		};

		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::shared_ptr<ResourceManager> m_resourceManager;

		DirectX::XMFLOAT3 m_position;
		DirectX::XMFLOAT3 m_rotation;
		DirectX::XMFLOAT3 m_scale;

		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;

		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader>m_geometryShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_MVPBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_timeBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_cameraBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_snakePropertiesBuffer;

		ModelViewProjectionConstantBuffer			m_MVPBufferData;
		TotalTimeConstantBuffer						m_timeBufferData;
		CameraPositionConstantBuffer				m_cameraBufferData;
		SnakePropertiesConstantBuffer				m_snakePropertiesBufferData;

		ID3D11ShaderResourceView*					m_snakeSkin;
		ID3D11SamplerState*							m_sampleStateWrap;

		uint32	m_indexCount;

		float m_radius;
		const int m_length;
		const int m_segments;
		bool m_directionZ;

		bool	m_loadingComplete;
	};
}

