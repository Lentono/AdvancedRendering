#include "pch.h"
#include "PlanetSea.h"

using namespace AlienPlanetACW;

PlanetSea::PlanetSea(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<ResourceManager>& resourceManager) :
	m_deviceResources(deviceResources), m_resourceManager(resourceManager), m_position(0.0f, 0.35f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), m_scale(20.0f, 1.0f, 20.0f), m_loadingComplete(false), m_indexCount(0)
{
	CreateDeviceDependentResources();
}

void PlanetSea::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"PlanetTerrainVS.cso");
	auto loadHSTask = DX::ReadDataAsync(L"PlanetTerrainHS.cso");
	auto loadDSTask = DX::ReadDataAsync(L"PlanetSeaDS.cso");
	auto loadPSTask = DX::ReadDataAsync(L"PlanetSeaPS.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
			)
		);
	});

	auto createHSTask = loadHSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateHullShader(&fileData[0], fileData.size(), nullptr, &m_hullShader));
	});

	auto createDSTask = loadDSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateDomainShader(&fileData[0], fileData.size(), nullptr, &m_domainShader));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
			)
		);

		D3D11_SAMPLER_DESC samplerWrapDescription;

		samplerWrapDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerWrapDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerWrapDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerWrapDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerWrapDescription.MipLODBias = 0.0f;
		samplerWrapDescription.MaxAnisotropy = 1;
		samplerWrapDescription.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerWrapDescription.BorderColor[0] = 0.0f;
		samplerWrapDescription.BorderColor[1] = 0.0f;
		samplerWrapDescription.BorderColor[2] = 0.0f;
		samplerWrapDescription.BorderColor[3] = 0.0f;
		samplerWrapDescription.MinLOD = 0.0f;
		samplerWrapDescription.MaxLOD = D3D11_FLOAT32_MAX;

		m_deviceResources->GetD3DDevice()->CreateSamplerState(&samplerWrapDescription, &m_sampleStateWrap);

		CD3D11_BUFFER_DESC MVPBufferDescription(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&MVPBufferDescription, nullptr, &m_MVPBuffer));

		CD3D11_BUFFER_DESC timeBufferDescription(sizeof(TotalTimeConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&timeBufferDescription, nullptr, &m_timeBuffer));

		CD3D11_BUFFER_DESC cameraBufferDescription(sizeof(CameraPositionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&cameraBufferDescription, nullptr, &m_cameraBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createPlaneTask = (createPSTask && createDSTask && createHSTask && createVSTask).then([this]() {

		m_resourceManager->GetTexture(m_deviceResources->GetD3DDevice(), L"PlanetSeaNormal.dds", m_normalTexture);
		m_resourceManager->GetTexture(m_deviceResources->GetD3DDevice(), L"PlanetSeaSpecular.dds", m_specularTexture);
		m_resourceManager->GetTexture(m_deviceResources->GetD3DDevice(), L"PlanetSeaDisplacement.dds", m_displacementTexture);

		m_resourceManager->GetModel(m_deviceResources->GetD3DDevice(), "plane.obj", m_vertexBuffer, m_indexBuffer);
		m_indexCount = m_resourceManager->GetIndexCount("plane.obj");
	});

	createPlaneTask.then([this]() {
		m_loadingComplete = true;
	});
}

void PlanetSea::SetViewProjectionMatrixConstantBuffer(DirectX::XMMATRIX& view, DirectX::XMMATRIX& projection)
{
	DirectX::XMStoreFloat4x4(&m_MVPBufferData.view, DirectX::XMMatrixTranspose(view));

	DirectX::XMStoreFloat4x4(&m_MVPBufferData.projection, DirectX::XMMatrixTranspose(projection));
}

void PlanetSea::SetCameraPositionConstantBuffer(DirectX::XMFLOAT3& cameraPosition)
{
	m_cameraBufferData.position = cameraPosition;
}

void PlanetSea::Update(DX::StepTimer const& timer)
{
	// Convert degrees to radians, then convert seconds to rotation angle
	//float radiansPerSecond = DirectX::XMConvertToRadians(45);
	//double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
	//float radians = static_cast<float>(fmod(totalRotation, DirectX::XM_2PI));

	auto worldMatrix = DirectX::XMMatrixIdentity();

	worldMatrix = XMMatrixMultiply(worldMatrix, DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z));
	worldMatrix = XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z)));
	//worldMatrix = XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(radians));
	worldMatrix = XMMatrixMultiply(worldMatrix, DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z));

	m_timeBufferData.time = timer.GetTotalSeconds();

	DirectX::XMStoreFloat4x4(&m_MVPBufferData.model, DirectX::XMMatrixTranspose(worldMatrix));
}

void PlanetSea::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Prepare constant buffers to send it to the graphics device.
	context->UpdateSubresource1(
		m_MVPBuffer.Get(),
		0,
		NULL,
		&m_MVPBufferData,
		0,
		0,
		0
	);

	context->UpdateSubresource1(
		m_timeBuffer.Get(),
		0,
		NULL,
		&m_timeBufferData,
		0,
		0,
		0
	);

	context->UpdateSubresource1(
		m_cameraBuffer.Get(),
		0,
		NULL,
		&m_cameraBufferData,
		0,
		0,
		0
	);

	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(AlienPlanetACW::VertexPositionTexcoordNormalTangentBinormal);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
	);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_MVPBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->HSSetShader(
		m_hullShader.Get(),
		nullptr,
		0
	);

	context->DSSetShader(
		m_domainShader.Get(),
		nullptr,
		0
	);

	context->DSSetConstantBuffers1(
		0,
		1,
		m_MVPBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->DSGetConstantBuffers1(
		1,
		1,
		m_timeBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->DSSetConstantBuffers1(
		2,
		1,
		m_cameraBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->DSSetShaderResources(0, 1, &m_displacementTexture);
	context->DSSetSamplers(0, 1, &m_sampleStateWrap);

	context->GSSetShader(
		nullptr,
		nullptr,
		0
	);

	D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);

	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	//rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

	m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());
	context->RSSetState(m_rasterizerState.Get());

	context->PSSetConstantBuffers1(
		0,
		1,
		m_MVPBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
	);

	context->PSSetShaderResources(0, 1, &m_normalTexture);
	context->PSSetShaderResources(1, 1, &m_specularTexture);

	context->PSSetSamplers(0, 1, &m_sampleStateWrap);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
	);
}

void PlanetSea::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_hullShader.Reset();
	m_domainShader.Reset();
	m_pixelShader.Reset();
	m_MVPBuffer.Reset();
	m_timeBuffer.Reset();
	m_cameraBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}