#include "pch.h"
#include "Snake.h"

using namespace AlienPlanetACW;

Snake::Snake(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<ResourceManager>& resourceManager, DirectX::XMFLOAT3 position, float radius, const int length, const int segments, bool directionZ) :
	m_deviceResources(deviceResources), m_resourceManager(resourceManager), m_position(position), m_rotation(0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f), m_loadingComplete(false), m_indexCount(0),
	m_radius(radius), m_length(length), m_segments(segments), m_directionZ(directionZ)
{
	m_snakePropertiesBufferData.radius = m_radius;
	m_snakePropertiesBufferData.length = m_length;
	m_snakePropertiesBufferData.segments = m_segments;
	m_snakePropertiesBufferData.directionZ = m_directionZ;

	m_resourceManager->GetTexture(m_deviceResources->GetD3DDevice(), L"snake.dds", m_snakeSkin);

	CreateDeviceDependentResources();
}

void Snake::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SnakeVS.cso");
	auto loadGSTask = DX::ReadDataAsync(L"SnakeGS.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SnakePS.cso");

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

	auto createGSTask = loadGSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateGeometryShader(&fileData[0], fileData.size(), nullptr, &m_geometryShader));
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

		CD3D11_BUFFER_DESC cameraBufferDescription(sizeof(CameraPositionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&cameraBufferDescription, nullptr, &m_cameraBuffer));

		CD3D11_BUFFER_DESC timeBufferDescription(sizeof(TotalTimeConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&timeBufferDescription, nullptr, &m_timeBuffer));

		CD3D11_BUFFER_DESC snakePropertiesBufferDescription(sizeof(SnakePropertiesConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&snakePropertiesBufferDescription, nullptr, &m_snakePropertiesBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createPlaneTask = (createPSTask && createGSTask && createVSTask).then([this]() {

		//Make sure to pass values to constant buffer
		//Make member variables so they can be passed properly, manually matching for now
		const int snakeLength = m_length;
		const int snakeSegments = m_segments;

		//static VertexPosition pointVertices[snakeSegments] = {};
		
		std::vector<VertexPosition> pointVertices;
		

		for (auto i = 1; i <= snakeSegments; i++)
		{
			VertexPosition vertex;

			if (m_directionZ)
			{
				vertex.position = DirectX::XMFLOAT3(0.0f, 0.0f, (static_cast<float>(snakeLength) / static_cast<float>(snakeSegments)) * i);
			}
			else
			{
				vertex.position = DirectX::XMFLOAT3((static_cast<float>(snakeLength) / static_cast<float>(snakeSegments)) * i, 0.0f, 0.0f);
			}

			pointVertices.emplace_back(vertex);

			//pointVertices->emplace_back
		}


		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = &pointVertices[0];
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		CD3D11_BUFFER_DESC vertexBufferDescription(pointVertices.size() * 12, D3D11_BIND_VERTEX_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDescription, &vertexBufferData, &m_vertexBuffer));

		std::vector<unsigned int> pointIndices;

		auto test = sizeof(pointIndices);

		//static unsigned int pointIndices[snakeSegments] = {};

		for (auto i = 0; i < snakeSegments; i++)
		{
			pointIndices.emplace_back(i);
		}

		m_indexCount = pointIndices.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = &pointIndices[0];
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;

		CD3D11_BUFFER_DESC indexBufferDescription(pointIndices.size() * 4, D3D11_BIND_INDEX_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDescription, &indexBufferData, &m_indexBuffer));
	});

	createPlaneTask.then([this]() {
		m_loadingComplete = true;
	});
}

void Snake::SetViewProjectionMatrixConstantBuffer(DirectX::XMMATRIX& view, DirectX::XMMATRIX& projection)
{
	DirectX::XMStoreFloat4x4(&m_MVPBufferData.view, DirectX::XMMatrixTranspose(view));

	DirectX::XMStoreFloat4x4(&m_MVPBufferData.projection, DirectX::XMMatrixTranspose(projection));
}

void Snake::SetCameraPositionConstantBuffer(DirectX::XMFLOAT3& cameraPosition)
{
	m_cameraBufferData.position = cameraPosition;
}

void Snake::Update(DX::StepTimer const& timer)
{
	// Convert degrees to radians, then convert seconds to rotation angle
	//float radiansPerSecond = DirectX::XMConvertToRadians(45);
	//double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
	//float radians = static_cast<float>(fmod(totalRotation, DirectX::XM_2PI));

	auto worldMatrix = DirectX::XMMatrixIdentity();

	if (m_directionZ)
	{
		m_position.z += 0.5f * timer.GetElapsedSeconds();

		if (m_position.z > 5.0f)
		{
			m_position.z = -5.0f;
		}
	}
	else
	{
		m_position.x += 0.5f * timer.GetElapsedSeconds();

		if (m_position.x > 5.0f)
		{
			m_position.x = -5.0f;
		}
	}


	worldMatrix = XMMatrixMultiply(worldMatrix, DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z));
	worldMatrix = XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z)));
	//worldMatrix = XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(radians));

	worldMatrix = XMMatrixMultiply(worldMatrix, DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z));

	m_timeBufferData.time = timer.GetTotalSeconds();

	//DirectX::XMStoreFloat4x4(&m_MVPBufferData.model, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, worldMatrix)));
	DirectX::XMStoreFloat4x4(&m_MVPBufferData.model, DirectX::XMMatrixTranspose(worldMatrix));
}

void Snake::Render()
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
		m_cameraBuffer.Get(),
		0,
		NULL,
		&m_cameraBufferData,
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
		m_snakePropertiesBuffer.Get(),
		0,
		NULL,
		&m_snakePropertiesBufferData,
		0,
		0,
		0
	);

	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(AlienPlanetACW::VertexPosition);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

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
		nullptr,
		nullptr,
		0
	);

	context->DSSetShader(
		nullptr,
		nullptr,
		0
	);

	context->GSSetShader(
		m_geometryShader.Get(),
		nullptr,
		0
	);

	context->GSSetConstantBuffers1(
		0,
		1,
		m_MVPBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->GSGetConstantBuffers1(
		1,
		1,
		m_timeBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->GSSetConstantBuffers1(
		2,
		1,
		m_cameraBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->GSSetConstantBuffers1(
		3,
		1,
		m_snakePropertiesBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);

	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	//rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

	m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());
	context->RSSetState(m_rasterizerState.Get());

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
	);

	context->PSSetConstantBuffers1(
		0,
		1,
		m_MVPBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetShaderResources(0, 1, &m_snakeSkin);
	context->PSSetSamplers(0, 1, &m_sampleStateWrap);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
	);
}

void Snake::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_geometryShader.Reset();
	m_pixelShader.Reset();
	m_MVPBuffer.Reset();
	m_timeBuffer.Reset();
	m_cameraBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}