#pragma once

#include <DirectXMath.h>

namespace AlienPlanetACW
{
	class Camera
	{
	public:
		Camera();

		DirectX::XMFLOAT3 GetPosition() const;
		DirectX::XMFLOAT3 GetRotation() const;

		void SetPosition(const DirectX::XMFLOAT3& position);
		void SetPosition(const float x, const float y, const float z);
		void SetRotation(const float x, const float y, const float z);

		void MoveForwards(const float movementStep);
		void MoveBackwards(const float movementStep);
		void MoveLeft(const float movementStep);
		void MoveRight(const float movementStep);

		void AddPositionX(const float x);
		void AddPositionY(const float y);
		void AddPositionZ(const float z);

		void AddRotationX(const float x);
		void AddRotationY(const float y);
		void AddRotationZ(const float z);

		void GetViewMatrix(DirectX::XMMATRIX &viewMatrix) const;

		void Render();

	private:
		float m_positionX;
		float m_positionY;
		float m_positionZ;

		float m_rotationX;
		float m_rotationY;
		float m_rotationZ;

		DirectX::XMFLOAT3 m_LHVector;
		DirectX::XMFLOAT3 m_lookAtVector;

		DirectX::XMFLOAT4X4 m_viewMatrix;
	};
}

