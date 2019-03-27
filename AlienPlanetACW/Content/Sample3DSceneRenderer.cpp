#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace AlienPlanetACW;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_tessellationFactor(32.0f),
	m_displacementPower(0.4f),
	m_degreesPerSecond(45),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	m_resourceManager = std::make_shared<ResourceManager>();

	//Create alpha blending state
	D3D11_BLEND_DESC blendStateDescription = CD3D11_BLEND_DESC(D3D11_DEFAULT);

	ZeroMemory(&blendStateDescription, sizeof(blendStateDescription));

	blendStateDescription.RenderTarget[0].BlendEnable = true;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	//blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	//blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	//blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	//blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	//blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	//blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_deviceResources->GetD3DDevice()->CreateBlendState(&blendStateDescription, &m_alphaEnabledBlendState);

	blendStateDescription.RenderTarget[0].BlendEnable = false;

	m_deviceResources->GetD3DDevice()->CreateBlendState(&blendStateDescription, &m_alphaDisableBlendState);

	// Create device independent resources
	Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextFormat(
			L"Segoe UI",
			nullptr,
			DWRITE_FONT_WEIGHT_LIGHT,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			32.0f,
			L"en-US",
			&textFormat
		)
	);

	DX::ThrowIfFailed(
		textFormat.As(&m_textFormat)
	);

	DX::ThrowIfFailed(
		m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
	);

	DX::ThrowIfFailed(
		m_deviceResources->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock)
	);

	m_camera = std::make_unique<Camera>();
	m_camera->SetPosition(0.0f, 0.5f, -0.5f);
	m_camera->SetRotation(0.0f, 0.0f, 0.0f);

	m_planetTerrain = std::make_unique<PlanetTerrain>(deviceResources, m_resourceManager);
	m_planetGrass = std::make_unique<PlanetGrass>(deviceResources);
	m_parametricTorus = std::make_unique<ParametricTorus>(deviceResources, m_resourceManager);
	m_parametricEllipsoid = std::make_unique<ParametricEllipsoid>(deviceResources, m_resourceManager);
	m_tessellatedSphere = std::make_unique<TessellatedSphere>(deviceResources, m_resourceManager);
	m_cameraTessellatedSphere = std::make_unique<CameraTessellatedSphere>(deviceResources, m_resourceManager);
	m_mobiusStrip = std::make_unique<BezierCurve>(deviceResources);
	m_snake.emplace_back(std::make_unique<Snake>(deviceResources, m_resourceManager, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), 0.05f, 1, 15, true));
	m_snake.emplace_back(std::make_unique<Snake>(deviceResources, m_resourceManager, DirectX::XMFLOAT3(-0.5f, 0.0f, 0.5f), 0.02f, 1, 15, true));
	m_snake.emplace_back(std::make_unique<Snake>(deviceResources, m_resourceManager, DirectX::XMFLOAT3(0.5f, 0.0f, 0.0f), 0.01f, 2, 30, false));
	m_planetSea = std::make_unique<PlanetSea>(deviceResources, m_resourceManager);
	m_implicitRayModels = std::make_unique<ImplicitRayModels>(deviceResources);
	m_implicitRayTracedModels = std::make_unique<ImplicitRayTracedModels>(deviceResources);

	
	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_whiteBrush)
	);
	

	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
	Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * DirectX::XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	DirectX::XMMATRIX perspectiveMatrix = DirectX::XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);

	DirectX::XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	DirectX::XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	DirectX::XMStoreFloat4x4(&m_projectionMatrix, perspectiveMatrix * orientationMatrix);
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	std::wstring tessDispText = L"Tessallation Factor: " + std::to_wstring(m_tessellationFactor) + L" (- & = to adjust) \r\n Displacement Power: " + std::to_wstring(m_displacementPower) + L" ([ & ] to adjust)";

	Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextLayout(
			tessDispText.c_str(),
			(uint32)tessDispText.length(),
			m_textFormat.Get(),
			1200.0f, // Max width of the input text.
			100.0f, // Max height of the input text.
			&textLayout
		)
	);

	DX::ThrowIfFailed(
		textLayout.As(&m_textLayout)
	);

	DX::ThrowIfFailed(
		m_textLayout->GetMetrics(&m_textMetrics)
	);

	CheckInputCameraMovement(timer);

	m_planetTerrain->Update(timer);
	m_planetGrass->Update(timer);
	m_parametricTorus->Update(timer);
	m_parametricEllipsoid->Update(timer);
	m_tessellatedSphere->Update(timer);
	m_cameraTessellatedSphere->Update(timer);
	m_mobiusStrip->Update(timer);

	for (auto& snake : m_snake)
	{
		snake->Update(timer);
	}

	m_planetSea->Update(timer);

	m_implicitRayModels->Update(timer);
	m_implicitRayTracedModels->Update(timer);
}

void Sample3DSceneRenderer::CheckInputCameraMovement(DX::StepTimer const& timer)
{
	//CoreVirtualKeyStates::Down

	if (QueryKeyPressed(VirtualKey::W))
	{
		m_camera->MoveForwards(0.5f * timer.GetElapsedSeconds());
	}
	
	if (QueryKeyPressed(VirtualKey::S))
	{
		m_camera->MoveBackwards(0.5f * timer.GetElapsedSeconds());
	}

	if (QueryKeyPressed(VirtualKey::A))
	{
		m_camera->MoveLeft(0.5f * timer.GetElapsedSeconds());
	}

	if (QueryKeyPressed(VirtualKey::D))
	{
		m_camera->MoveRight(0.5f * timer.GetElapsedSeconds());
	}

	if (QueryKeyPressed(VirtualKey::Shift))
	{
		m_camera->AddPositionY(-0.5f * timer.GetElapsedSeconds());
	}

	if (QueryKeyPressed(VirtualKey::Space))
	{
		m_camera->AddPositionY(0.5f * timer.GetElapsedSeconds());
	}

	if (QueryKeyPressed(VirtualKey::Up))
	{
		m_camera->AddRotationY(50.0f * timer.GetElapsedSeconds());
	}

	if (QueryKeyPressed(VirtualKey::Down))
	{
		m_camera->AddRotationY(-50.0f * timer.GetElapsedSeconds());
	}

	if (QueryKeyPressed(VirtualKey::Left))
	{
		m_camera->AddRotationX(50.0f * timer.GetElapsedSeconds());
	}

	if (QueryKeyPressed(VirtualKey::Right))
	{
		m_camera->AddRotationX(-50.0f * timer.GetElapsedSeconds());
	}

	if (QueryKeyPressed(static_cast<VirtualKey>(VK_OEM_MINUS)))
	{
		if (m_tessellationFactor > 1.0f)
		{
			m_tessellationFactor -= 1.0f;
		}
	}

	if (QueryKeyPressed(static_cast<VirtualKey>(VK_OEM_PLUS)))
	{
		if (m_tessellationFactor < 64.0f)
		{
			m_tessellationFactor += 1.0f;
		}
	}

	if (QueryKeyPressed(static_cast<VirtualKey>(VK_OEM_4)))
	{
		m_displacementPower -= 0.01f;
	}

	if (QueryKeyPressed(static_cast<VirtualKey>(VK_OEM_6)))
	{
		m_displacementPower += 0.01f;
	}
}

bool Sample3DSceneRenderer::QueryKeyPressed(VirtualKey key)
{
	return (CoreWindow::GetForCurrentThread()->GetKeyState(key) & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render()
{
	m_camera->Render();

	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixIdentity();
	m_camera->GetViewMatrix(viewMatrix);

	m_planetTerrain->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_planetTerrain->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_planetTerrain->Render();

	m_planetGrass->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_planetGrass->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_planetGrass->Render();

	m_parametricTorus->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_parametricTorus->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_parametricTorus->Render();

	m_parametricEllipsoid->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_parametricEllipsoid->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_parametricEllipsoid->Render();

	m_tessellatedSphere->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_tessellatedSphere->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_tessellatedSphere->SetTessellationFactorConstantBuffer(m_tessellationFactor);
	m_tessellatedSphere->SetDisplacementPowerConstantBuffer(m_displacementPower);
	m_tessellatedSphere->Render();

	m_cameraTessellatedSphere->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_cameraTessellatedSphere->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_cameraTessellatedSphere->SetDisplacementPowerConstantBuffer(m_displacementPower);
	m_cameraTessellatedSphere->Render();

	m_mobiusStrip->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_mobiusStrip->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_mobiusStrip->Render();

	for (auto& snake : m_snake)
	{
		snake->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
		snake->SetCameraPositionConstantBuffer(m_camera->GetPosition());
		snake->Render();
	}

	m_planetSea->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_planetSea->SetCameraPositionConstantBuffer(m_camera->GetPosition());

	m_deviceResources->GetD3DDeviceContext()->OMSetBlendState(m_alphaEnabledBlendState, nullptr, 0xffffffff);

	m_planetSea->Render();

	m_deviceResources->GetD3DDeviceContext()->OMSetBlendState(m_alphaDisableBlendState, nullptr, 0xffffffff);

	m_implicitRayModels->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_implicitRayModels->SetInverseViewMatrixConstantBuffer(DirectX::XMMatrixInverse(nullptr, viewMatrix));
	m_implicitRayModels->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_implicitRayModels->Render();

	m_implicitRayTracedModels->SetViewProjectionMatrixConstantBuffer(viewMatrix, DirectX::XMLoadFloat4x4(&m_projectionMatrix));
	m_implicitRayTracedModels->SetInverseViewMatrixConstantBuffer(DirectX::XMMatrixInverse(nullptr, viewMatrix));
	m_implicitRayTracedModels->SetCameraPositionConstantBuffer(m_camera->GetPosition());
	m_implicitRayTracedModels->Render();

	ID2D1DeviceContext* context = m_deviceResources->GetD2DDeviceContext();
	Windows::Foundation::Size logicalSize = m_deviceResources->GetLogicalSize();

	context->SaveDrawingState(m_stateBlock.Get());
	context->BeginDraw();

	// Position on the bottom right corner
	D2D1::Matrix3x2F screenTranslation = D2D1::Matrix3x2F::Translation(
		logicalSize.Width - m_textMetrics.layoutWidth,
		logicalSize.Height - m_textMetrics.height - 50
	);

	context->SetTransform(screenTranslation * m_deviceResources->GetOrientationTransform2D());

	DX::ThrowIfFailed(
		m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING)
	);

	context->DrawTextLayout(
		D2D1::Point2F(0.f, 0.f),
		m_textLayout.Get(),
		m_whiteBrush.Get()
	);

	HRESULT hr = context->EndDraw();
	if (hr != D2DERR_RECREATE_TARGET)
	{
		DX::ThrowIfFailed(hr);
	}

	context->RestoreDrawingState(m_stateBlock.Get());
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_whiteBrush)
	);

	m_planetTerrain->CreateDeviceDependentResources();
	m_planetGrass->CreateDeviceDependentResources();
	m_parametricTorus->CreateDeviceDependentResources();
	m_parametricEllipsoid->CreateDeviceDependentResources();
	m_tessellatedSphere->CreateDeviceDependentResources();
	m_cameraTessellatedSphere->CreateDeviceDependentResources();
	m_mobiusStrip->CreateDeviceDependentResources();

	for (auto& snake : m_snake)
	{
		snake->CreateDeviceDependentResources();
	}

	m_planetSea->CreateDeviceDependentResources();

	m_implicitRayModels->CreateDeviceDependentResources();
	m_implicitRayTracedModels->CreateDeviceDependentResources();
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources()
{
	m_whiteBrush.Reset();

	m_planetTerrain->ReleaseDeviceDependentResources();
	m_planetGrass->ReleaseDeviceDependentResources();
	m_parametricTorus->ReleaseDeviceDependentResources();
	m_parametricEllipsoid->ReleaseDeviceDependentResources();
	m_tessellatedSphere->ReleaseDeviceDependentResources();
	m_cameraTessellatedSphere->ReleaseDeviceDependentResources();
	m_mobiusStrip->ReleaseDeviceDependentResources();

	for (auto& snake : m_snake)
	{
		snake->ReleaseDeviceDependentResources();
	}

	m_planetSea->ReleaseDeviceDependentResources();

	m_implicitRayModels->ReleaseDeviceDependentResources();
	m_implicitRayTracedModels->ReleaseDeviceDependentResources();
}