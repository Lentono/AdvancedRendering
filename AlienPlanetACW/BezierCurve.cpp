#include "pch.h"
#include "BezierCurve.h"

using namespace AlienPlanetACW;

BezierCurve::BezierCurve(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources), m_position(2.0f, 2.0f, -1.0f), m_rotation(0.0f, 0.0f, 0.0f), m_scale(0.3f, 0.3f, 0.3f), m_loadingComplete(false), m_indexCount(0)
{
	CreateDeviceDependentResources();
}

void BezierCurve::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"BezierCurveVS.cso");
	auto loadHSTask = DX::ReadDataAsync(L"BezierCurveHS.cso");
	auto loadDSTask = DX::ReadDataAsync(L"BezierCurveDS.cso");
	auto loadPSTask = DX::ReadDataAsync(L"BezierCurvePS.cso");

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

		CD3D11_BUFFER_DESC MVPBufferDescription(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&MVPBufferDescription, nullptr, &m_MVPBuffer));

		CD3D11_BUFFER_DESC cameraBufferDescription(sizeof(CameraPositionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&cameraBufferDescription, nullptr, &m_cameraBuffer));

		CD3D11_BUFFER_DESC timeBufferDescription(sizeof(TotalTimeConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&timeBufferDescription, nullptr, &m_timeBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createPlaneTask = (createPSTask && createHSTask && createDSTask && createVSTask).then([this]() {

		static VertexPosition pointVertices[64] = {
			{ DirectX::XMFLOAT3(1.0f, -0.5f, 0.0f)},
			{ DirectX::XMFLOAT3(1.0f, -0.5f, 0.5f)},
			{ DirectX::XMFLOAT3(0.5f, -0.3536f, 1.354f)},
			{ DirectX::XMFLOAT3(0.0f, -0.3536f, 1.354f)},
			{ DirectX::XMFLOAT3(1.0f, -0.1667f, 0.0f) },
			{ DirectX::XMFLOAT3(1.0f, -0.1667f, 0.5f) },
			{ DirectX::XMFLOAT3(0.5f, -0.1179f, 1.118f)},
			{ DirectX::XMFLOAT3(0.0f, -0.1179f, 1.118f) },
			{ DirectX::XMFLOAT3(1.0f, 0.1667f, 0.0f) },
			{ DirectX::XMFLOAT3(1.0f, 0.1667f, 0.5f) },
			{ DirectX::XMFLOAT3(0.5f, 0.1179f, 0.8821f) },
			{ DirectX::XMFLOAT3(0.0f, 0.1179f, 0.8821f)},
			{ DirectX::XMFLOAT3(1.0f, 0.5f, 0.0f) },
			{ DirectX::XMFLOAT3(1.0f, 0.5f, 0.5f) },
			{ DirectX::XMFLOAT3(0.5f, 0.3536f, 0.6464f) },
			{ DirectX::XMFLOAT3(0.0f, 0.3536f, 0.6464f) },
			{ DirectX::XMFLOAT3(0.0f, -0.3536f, 1.354f) },
			{ DirectX::XMFLOAT3(-0.5f, -0.3536f, 1.354f) },
			{ DirectX::XMFLOAT3(-1.5f, 0.0f, 0.5f) },
			{ DirectX::XMFLOAT3(-1.5f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(0.0f, -0.1179f, 1.118f) },
			{ DirectX::XMFLOAT3(-0.5f, -0.1179f, 1.118f) },
			{ DirectX::XMFLOAT3(-1.167f, 0.0f, 0.5f) },
			{ DirectX::XMFLOAT3(-1.167f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(0.0f, 0.1179f, 0.8821f) },
			{ DirectX::XMFLOAT3(-0.5f, 0.1179f, 0.8821f) },
			{ DirectX::XMFLOAT3(-0.8333f, 0.0f, 0.5f) },
			{ DirectX::XMFLOAT3(-0.8333f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(0.0f, 0.3536f, 0.6464f) },
			{ DirectX::XMFLOAT3(-0.5f, 0.3536f, 0.6464f) },
			{ DirectX::XMFLOAT3(-0.5f, 0.0f, 0.5f) },
			{ DirectX::XMFLOAT3(-0.5f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(-1.5f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(-1.5f, 0.0f, -0.5f) },
			{ DirectX::XMFLOAT3(-0.5f, 0.3536f, -1.354f) },
			{ DirectX::XMFLOAT3(0.0f, 0.3536f, -1.354f) },
			{ DirectX::XMFLOAT3(-1.167f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(-1.167f, 0.0f, -0.5f) },
			{ DirectX::XMFLOAT3(-0.5f, 0.1179f, -1.118f) },
			{ DirectX::XMFLOAT3(0.0f, 0.1179f, -1.118f) },
			{ DirectX::XMFLOAT3(-0.8333f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(-0.8333f, 0.0f, -0.5f) },
			{ DirectX::XMFLOAT3(-0.5f, -0.1179f, -0.8821f) },
			{ DirectX::XMFLOAT3(0.0f, -0.1179f, -0.8821f) },
			{ DirectX::XMFLOAT3(-0.5f, 0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(-0.5f, 0.0f, -0.5f) },
		    { DirectX::XMFLOAT3(-0.5f, -0.3536f, -0.6464f) },
			{ DirectX::XMFLOAT3(0.0f, -0.3536f, -0.6464f) },
			{ DirectX::XMFLOAT3(0.0f, 0.3536f, -1.354f) },
			{ DirectX::XMFLOAT3(0.5f, 0.3536f, -1.354f) },
			{ DirectX::XMFLOAT3(1.0f, 0.5f, -0.5f) },
			{ DirectX::XMFLOAT3(1.0f, 0.5f, 0.0f) },
			{ DirectX::XMFLOAT3(0.0f, 0.1179f, -1.118f) },
			{ DirectX::XMFLOAT3(0.5f, 0.1179f, -1.118f) },
			{ DirectX::XMFLOAT3(1.0f, 0.1667f, -0.5f) },
			{ DirectX::XMFLOAT3(1.0f, 0.1667f, 0.0f) },
			{ DirectX::XMFLOAT3(0.0f, -0.1179f, -0.8821f) },
			{ DirectX::XMFLOAT3(0.5f, -0.1179f, -0.8821f) },
			{ DirectX::XMFLOAT3(1.0f, -0.1667f, -0.5f) },
			{ DirectX::XMFLOAT3(1.0f, -0.1667f, 0.0f) },
			{ DirectX::XMFLOAT3(0.0f, -0.3536f, -0.6464f) },
			{ DirectX::XMFLOAT3(0.5f, -0.3536f, -0.6464f) },
			{ DirectX::XMFLOAT3(1.0f, -0.5f, -0.5f) },
			{ DirectX::XMFLOAT3(1.0f, -0.5f, 0.0f) },
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = pointVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		CD3D11_BUFFER_DESC vertexBufferDescription(sizeof(pointVertices), D3D11_BIND_VERTEX_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDescription, &vertexBufferData, &m_vertexBuffer));

		static unsigned int pointIndices[64] = {};

		for (auto i = 0; i < 64; i++)
		{
			pointIndices[i] = i;
		}

		m_indexCount = ARRAYSIZE(pointIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = pointIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;

		CD3D11_BUFFER_DESC indexBufferDescription(sizeof(pointIndices), D3D11_BIND_INDEX_BUFFER);

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDescription, &indexBufferData, &m_indexBuffer));
	});

	createPlaneTask.then([this]() {
		m_loadingComplete = true;
	});
}

void BezierCurve::SetViewProjectionMatrixConstantBuffer(DirectX::XMMATRIX& view, DirectX::XMMATRIX& projection)
{
	DirectX::XMStoreFloat4x4(&m_MVPBufferData.view, DirectX::XMMatrixTranspose(view));

	DirectX::XMStoreFloat4x4(&m_MVPBufferData.projection, DirectX::XMMatrixTranspose(projection));
}

void BezierCurve::SetCameraPositionConstantBuffer(DirectX::XMFLOAT3& cameraPosition)
{
	m_cameraBufferData.position = cameraPosition;
}

void BezierCurve::Update(DX::StepTimer const& timer)
{
	auto worldMatrix = DirectX::XMMatrixIdentity();

	worldMatrix = XMMatrixMultiply(worldMatrix, DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z));
	worldMatrix = XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z)));
	//worldMatrix = XMMatrixMultiply(worldMatrix, DirectX::XMMatrixRotationY(radians));

	worldMatrix = XMMatrixMultiply(worldMatrix, DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z));

	m_timeBufferData.time = timer.GetTotalSeconds();

	//DirectX::XMStoreFloat4x4(&m_MVPBufferData.model, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, worldMatrix)));
	DirectX::XMStoreFloat4x4(&m_MVPBufferData.model, DirectX::XMMatrixTranspose(worldMatrix));
}

void BezierCurve::Render()
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

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
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

	context->DSSetConstantBuffers1(
		1,
		1,
		m_cameraBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->GSSetShader(
		nullptr,
		nullptr,
		0
	);

	D3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);

	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

	m_deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());
	context->RSSetState(m_rasterizerState.Get());

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
	);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
	);
}

void BezierCurve::ReleaseDeviceDependentResources()
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