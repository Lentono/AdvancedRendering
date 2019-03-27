#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include <vector>
#include <string>

#include "ResourceManager.h"

#include "Camera.h"
#include "PlanetTerrain.h"
#include "PlanetGrass.h"
#include "ParametricTorus.h"
#include "ParametricEllipsoid.h"
#include "TessellatedSphere.h"
#include "CameraTessellatedSphere.h"
#include "BezierCurve.h"
#include "Snake.h"
#include "PlanetSea.h"
#include "ImplicitRayModels.h"
#include "ImplicitRayTracedModels.h"

namespace AlienPlanetACW
{
	using namespace Windows::System;
	using namespace Windows::UI::Core;

	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();

	private:
		void CheckInputCameraMovement(DX::StepTimer const& timer);
		bool QueryKeyPressed(VirtualKey key);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::shared_ptr<ResourceManager> m_resourceManager;

		ID3D11BlendState* m_alphaEnabledBlendState;
		ID3D11BlendState* m_alphaDisableBlendState;

		// Resources related to text rendering.

		Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1> m_stateBlock;
		Microsoft::WRL::ComPtr<IDWriteTextFormat2>      m_textFormat;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>    m_whiteBrush;
		Microsoft::WRL::ComPtr<IDWriteTextLayout3>      m_textLayout;
		DWRITE_TEXT_METRICS	                            m_textMetrics;



		std::unique_ptr<Camera> m_camera;
		std::unique_ptr<PlanetTerrain> m_planetTerrain;
		std::unique_ptr<PlanetGrass> m_planetGrass;
		std::unique_ptr<ParametricTorus> m_parametricTorus;
		std::unique_ptr<ParametricEllipsoid> m_parametricEllipsoid;
		std::unique_ptr<TessellatedSphere> m_tessellatedSphere;
		std::unique_ptr<CameraTessellatedSphere> m_cameraTessellatedSphere;
		std::unique_ptr<BezierCurve> m_mobiusStrip;
		std::vector<std::unique_ptr<Snake>> m_snake;
		std::unique_ptr<PlanetSea> m_planetSea;
		std::unique_ptr<ImplicitRayModels> m_implicitRayModels;
		std::unique_ptr<ImplicitRayTracedModels> m_implicitRayTracedModels;

		DirectX::XMFLOAT4X4 m_projectionMatrix;

		// Variables used with the rendering loop.

		float m_tessellationFactor;
		float m_displacementPower;

		float	m_degreesPerSecond;
		bool	m_tracking;
	};
}

