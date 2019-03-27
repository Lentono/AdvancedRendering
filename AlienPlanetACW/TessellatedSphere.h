#pragma once

#include "..\Common\DeviceResources.h"
#include "..\Common\DirectXHelper.h"
#include "..\Content\ShaderStructures.h"
#include "..\Common\StepTimer.h"

#include "ResourceManager.h"
#include <DirectXMath.h>

namespace AlienPlanetACW
{
	class TessellatedSphere
	{
	public:
		TessellatedSphere(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<ResourceManager>& resourceManager);
		void CreateDeviceDependentResources();
		void SetViewProjectionMatrixConstantBuffer(DirectX::XMMATRIX& view, DirectX::XMMATRIX& projection);
		void SetCameraPositionConstantBuffer(DirectX::XMFLOAT3& cameraPosition);
		void SetTessellationFactorConstantBuffer(const float tessellationFactor);
		void SetDisplacementPowerConstantBuffer(const float displacementPower);
		void ReleaseDeviceDependentResources();

		void Update(DX::StepTimer const& timer);
		void Render();

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::shared_ptr<ResourceManager> m_resourceManager;

		DirectX::XMFLOAT3 m_position;
		DirectX::XMFLOAT3 m_rotation;
		DirectX::XMFLOAT3 m_scale;

		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;

		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11HullShader>	m_hullShader;
		Microsoft::WRL::ComPtr<ID3D11DomainShader>	m_domainShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_MVPBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_cameraBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_tessellationFactorBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_displacementPowerBuffer;

		ModelViewProjectionConstantBuffer			m_MVPBufferData;
		CameraPositionConstantBuffer				m_cameraBufferData;
		TessellationFactorConstantBuffer			m_tessellationFactorBufferData;
		DisplacementPowerConstantBuffer				m_displacementPowerBufferData;

		ID3D11ShaderResourceView*					m_diffuseTexture;
		ID3D11ShaderResourceView*					m_normalTexture;
		ID3D11ShaderResourceView*					m_specularTexture;
		ID3D11ShaderResourceView*					m_displacementTexture;
		ID3D11SamplerState*							m_sampleStateWrap;

		uint32										m_indexCount;

		bool										m_loadingComplete;
	};
}

