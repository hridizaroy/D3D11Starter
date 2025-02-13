#include "Transform.h"

using namespace DirectX;

Transform::Transform():
	m_position(0.0f, 0.0f, 0.0f),
	m_rotation(0.0f, 0.0f, 0.0f),
	m_scale(1.0f, 1.0f, 1.0f),
	m_isDirty(false)
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

void Transform::SetPosition(const XMFLOAT3& position)
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

void Transform::SetRotation(const XMFLOAT3& rotation)
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

void Transform::SetScale(const XMFLOAT3& scale)
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
	if (m_isDirty)
	{
		updateWorldMatrix();
	}

	return m_worldMatrix;
}

XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	if (m_isDirty)
	{
		updateWorldMatrix();
	}

	return m_worldInverseTransposeMatrix;
}

void Transform::updateWorldMatrix()
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

XMFLOAT3 Transform::GetPitchYawRoll() const
{
	return m_rotation;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	XMFLOAT3 moveVec(x, y, z);
	XMVECTOR currPos = XMLoadFloat3(&m_position);
	XMVECTOR moveValue = XMLoadFloat3(&moveVec);
	currPos += moveValue;

	XMStoreFloat3(&m_position, currPos);

	m_isDirty = true;
}

void Transform::MoveAbsolute(const XMFLOAT3& offset)
{
	XMVECTOR currPos = XMLoadFloat3(&m_position);
	XMVECTOR moveValue = XMLoadFloat3(&offset);
	currPos += moveValue;

	XMStoreFloat3(&m_position, currPos);
	m_isDirty = true;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	XMFLOAT3 rotateVec(pitch, yaw, roll);
	XMVECTOR currRotateValue = XMLoadFloat3(&m_rotation);
	XMVECTOR newRotateValue = XMLoadFloat3(&rotateVec);
	currRotateValue += newRotateValue;

	XMStoreFloat3(&m_rotation, currRotateValue);

	m_isDirty = true;
}

void Transform::Rotate(XMFLOAT3 rotation)
{
	XMVECTOR currRotateValue = XMLoadFloat3(&m_rotation);
	XMVECTOR newRotateValue = XMLoadFloat3(&rotation);
	currRotateValue += newRotateValue;

	XMStoreFloat3(&m_rotation, currRotateValue);

	m_isDirty = true;
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
