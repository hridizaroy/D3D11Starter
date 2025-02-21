#include "Transform.h"

using namespace DirectX;

Transform::Transform() :
	m_position(0.0f, 0.0f, 0.0f),
	m_rotation(0.0f, 0.0f, 0.0f),
	m_scale(1.0f, 1.0f, 1.0f),
	m_right(1.0f, 0.0f, 0.0f),
	m_up(0.0f, 1.0f, 0.0f),
	m_forward(0.0f, 0.0f, 1.0f),
	m_isDirty(false),
	m_isRotationDirty(false)
{
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_worldInverseTransposeMatrix, XMMatrixIdentity());
}
	
void Transform::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;

	m_isDirty = true;
}

void Transform::SetPosition(const XMFLOAT3 position)
{
	m_position = position;
	m_isDirty = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	m_rotation.x = pitch;
	m_rotation.y = yaw;
	m_rotation.z = roll;

	m_isDirty = true;
}

void Transform::SetRotation(const XMFLOAT3 rotation)
{
	m_rotation = rotation;
	m_isDirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	m_scale.x = x;
	m_scale.y = y;
	m_scale.z = z;

	m_isDirty = true;
}

void Transform::SetScale(const XMFLOAT3 scale)
{
	m_scale = scale;
	m_isDirty = true;
}

XMFLOAT3 Transform::GetPosition() const
{
	return m_position;
}

XMFLOAT3 Transform::GetScale() const
{
	return m_position;
}

XMFLOAT4X4 Transform::GetWorldMatrix()
{
	UpdateWorldMatrix();

	return m_worldMatrix;
}

XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	UpdateWorldMatrix();

	return m_worldInverseTransposeMatrix;
}

XMFLOAT3 Transform::GetRight()
{
	UpdateLocalVectors();
	return m_right;
}

XMFLOAT3 Transform::GetUp()
{
	UpdateLocalVectors();
	return m_up;
}

XMFLOAT3 Transform::GetForward()
{
	UpdateLocalVectors();
	return m_forward;
}

void Transform::UpdateLocalVectors()
{
	if (m_isRotationDirty)
	{
		XMVECTOR rotateQuat = XMQuaternionRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
		XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMVECTOR rightVector = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR forwardVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		XMVECTOR currUp = XMVector3Rotate(upVector, rotateQuat);
		XMVECTOR currRight = XMVector3Rotate(rightVector, rotateQuat);
		XMVECTOR currForward = XMVector3Rotate(forwardVector, rotateQuat);

		XMStoreFloat3(&m_up, currUp);
		XMStoreFloat3(&m_right, currRight);
		XMStoreFloat3(&m_forward, currForward);

		m_isRotationDirty = false;
	}
}

void Transform::UpdateWorldMatrix()
{
	if (m_isDirty)
	{
		XMMATRIX scaleMat = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
		XMMATRIX rotateMat = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
		XMMATRIX translateMat = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

		XMMATRIX worldMatrix = scaleMat * rotateMat * translateMat;

		XMStoreFloat4x4(&m_worldMatrix, worldMatrix);

		XMStoreFloat4x4(&m_worldInverseTransposeMatrix,
			XMMatrixInverse(0, XMMatrixTranspose(worldMatrix)));

		m_isDirty = false;
	}
}

XMFLOAT3 Transform::GetPitchYawRoll() const
{
	return m_rotation;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	XMVECTOR moveVector = XMVectorSet(x, y, z, 1.0f);
	
	XMVECTOR currPos = XMLoadFloat3(&m_position);
	currPos += moveVector;

	XMStoreFloat3(&m_position, currPos);
	m_isDirty = true;
}

void Transform::MoveAbsolute(const XMFLOAT3& offset)
{
	XMVECTOR moveVector = XMLoadFloat3(&offset);
	
	XMVECTOR currPos = XMLoadFloat3(&m_position);
	currPos += moveVector;

	XMStoreFloat3(&m_position, currPos);
	m_isDirty = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
	XMVECTOR moveVector = XMVectorSet(x, y, z, 1.0f);
	
	XMVECTOR rotateQuat = XMQuaternionRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

	XMVECTOR currPos = XMLoadFloat3(&m_position);
	currPos += XMVector3Rotate(moveVector, rotateQuat);

	XMStoreFloat3(&m_position, currPos);
	m_isDirty = true;
}

void Transform::MoveRelative(const XMFLOAT3& offset)
{
	XMVECTOR moveVector = XMLoadFloat3(&offset);
	
	XMVECTOR rotateQuat = XMQuaternionRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);

	XMVECTOR currPos = XMLoadFloat3(&m_position);
	currPos += XMVector3Rotate(moveVector, rotateQuat);

	XMStoreFloat3(&m_position, currPos);
	m_isDirty = true;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	XMVECTOR rotateValue = XMVectorSet(pitch, yaw, roll, 0.0f);
	
	XMVECTOR currRotation = XMLoadFloat3(&m_rotation);
	currRotation += rotateValue;

	XMStoreFloat3(&m_rotation, currRotation);
	m_isDirty = true;
	m_isRotationDirty = true;
}

void Transform::Rotate(const XMFLOAT3& rotation)
{
	XMVECTOR rotateValue = XMLoadFloat3(&rotation);
	
	XMVECTOR currRotation = XMLoadFloat3(&m_rotation);
	currRotation += rotateValue;

	XMStoreFloat3(&m_rotation, currRotation);
	m_isDirty = true;
	m_isRotationDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
	m_scale.x *= x;
	m_scale.y *= y;
	m_scale.z *= z;

	m_isDirty = true;
}

void Transform::Scale(const XMFLOAT3& scale)
{
	m_scale.x *= scale.x;
	m_scale.y *= scale.y;
	m_scale.z *= scale.z;

	m_isDirty = true;
}
