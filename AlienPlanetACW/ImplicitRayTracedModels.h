#pragma once

#include "..\Common\DeviceResources.h"
#include "..\Common\DirectXHelper.h"
#include "..\Content\ShaderStructures.h"
#include "..\Common\StepTimer.h"

#include <DirectXMath.h>

namespace AlienPlanetACW
{

	class ImplicitRayTracedModels
	{
	public:
		ImplicitRayTracedModels(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void SetViewProjectionMatrixConstantBuffer(DirectX::XMMATRIX& view, DirectX::XMMATRIX& projection);
		void SetInverseViewMatrixConstantBuffer(DirectX::XMMATRIX& inverseView);
		void SetCameraPositionConstantBuffer(DirectX::XMFLOAT3& cameraPosition);
		void ReleaseDeviceDependentResources();

		void Update(DX::StepTimer const& timer);
		void Render();

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;

		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_MVPBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_cameraBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_timeBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_inverseViewBuffer;

		ModelViewProjectionConstantBuffer			m_MVPBufferData;
		CameraPositionConstantBuffer				m_cameraBufferData;
		TotalTimeConstantBuffer						m_timeBufferData;
		InverseViewConstantBuffer					m_inverseViewBufferData;

		uint32	m_indexCount;

		bool	m_loadingComplete;
	};
}
