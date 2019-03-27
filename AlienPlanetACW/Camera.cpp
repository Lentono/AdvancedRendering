#include "pch.h"
#include "Camera.h"

using namespace AlienPlanetACW;

Camera::Camera() : m_positionX(0), m_positionY(0), m_positionZ(0), m_rotationX(0), m_rotationY(0), m_rotationZ(0)
{
}

DirectX::XMFLOAT3 Camera::GetPosition() const 
{
	return DirectX::XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}

DirectX::XMFLOAT3 Camera::GetRotation() const 
{
	return DirectX::XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}

void Camera::SetPosition(const DirectX::XMFLOAT3& position)
{
	m_positionX = position.x;
	m_positionY = position.y;
	m_positionZ = position.z;
}


void Camera::SetPosition(const float x, const float y, const float z) {
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
}

void Camera::SetRotation(const float x, const float y, const float z) {
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
}

void Camera::MoveForwards(const float movementStep)
{
	m_positionX -= movementStep * m_lookAtVector.x;
	m_positionY -= movementStep * m_lookAtVector.y;
	m_positionZ -= movementStep * m_lookAtVector.z;
}

void Camera::MoveBackwards(const float movementStep)
{
	m_positionX += movementStep * m_lookAtVector.x;
	m_positionY += movementStep * m_lookAtVector.y;
	m_positionZ += movementStep * m_lookAtVector.z;
}

void Camera::MoveLeft(const float movementStep)
{
	m_positionX -= movementStep * m_LHVector.x;
	m_positionY -= movementStep * m_LHVector.y;
	m_positionZ -= movementStep * m_LHVector.z;
}

void Camera::MoveRight(const float movementStep)
{
	m_positionX += movementStep * m_LHVector.x;
	m_positionY += movementStep * m_LHVector.y;
	m_positionZ += movementStep * m_LHVector.z;
}

void Camera::AddPositionX(const float x)
{
	m_positionX += x;
}

void Camera::AddPositionY(const float y)
{
	m_positionY += y;
}

void Camera::AddPositionZ(const float z)
{
	m_positionZ += z;
}

void Camera::AddRotationX(const float x)
{
	m_rotationX += x;
}

void Camera::AddRotationY(const float y)
{
	m_rotationY += y;
}

void Camera::AddRotationZ(const float z)
{
	m_rotationZ += z;
}

void Camera::GetViewMatrix(DirectX::XMMATRIX& viewMatrix) const 
{
	viewMatrix = DirectX::XMLoadFloat4x4(&m_viewMatrix);
}

void Camera::Render()
{
	const auto positionVector = DirectX::XMVectorSet(m_positionX, m_positionY, m_positionZ, 0.0f);

	const auto yaw = DirectX::XMConvertToRadians(m_rotationX);
	const auto pitch = DirectX::XMConvertToRadians(m_rotationY);
	const auto roll = DirectX::XMConvertToRadians(m_rotationZ);

	const auto rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	//Transform the upVector and lookAtVector by our rotation matrix so the camera is rotated at the origin
	const auto upVector = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotationMatrix);
	auto newLookAtVector = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotationMatrix);

	DirectX::XMStoreFloat3(&m_lookAtVector, newLookAtVector);

	DirectX::XMStoreFloat3(&m_LHVector, DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), rotationMatrix));

	//Translate camera to look at position
	newLookAtVector = DirectX::XMVectorAdd(positionVector, newLookAtVector);

	//Create view matrix
	DirectX::XMStoreFloat4x4(&m_viewMatrix, DirectX::XMMatrixLookAtLH(positionVector, newLookAtVector, upVector));
}