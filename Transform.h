#pragma once

#include <DirectXMath.h>

class Transform
{
public:
	Transform();

	// Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3 position);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(const DirectX::XMFLOAT3 rotation);
	void SetScale(float x, float y, float z);
	void SetScale(const DirectX::XMFLOAT3 scale);

	// Getters
	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT3 GetScale() const;
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();
	DirectX::XMFLOAT3 GetPitchYawRoll() const;
	DirectX::XMFLOAT3 GetRight();
	DirectX::XMFLOAT3 GetUp();
	DirectX::XMFLOAT3 GetForward();

	// Transformers
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(const DirectX::XMFLOAT3& offset);

	void MoveRelative(float x, float y, float z);
	void MoveRelative(const DirectX::XMFLOAT3& offset);

	void Rotate(float pitch, float yaw, float roll);
	void Rotate(const DirectX::XMFLOAT3& rotation);

	void Scale(float x, float y, float z);
	void Scale(const DirectX::XMFLOAT3& scale);

	// We need to give users direct control over our transformation variables via imgui
	friend class Game;

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMFLOAT3 m_scale;

	DirectX::XMFLOAT3 m_right;
	DirectX::XMFLOAT3 m_up;
	DirectX::XMFLOAT3 m_forward;

	DirectX::XMFLOAT4X4 m_worldMatrix;
	DirectX::XMFLOAT4X4 m_worldInverseTransposeMatrix;

	bool m_isDirty;
	bool m_isRotationDirty;

	void UpdateWorldMatrix();
	void UpdateLocalVectors();
	void MoveAbsolute(const DirectX::XMVECTOR& moveVector);
	void MoveRelative(const DirectX::XMVECTOR& moveVector);
	void Rotate(const DirectX::XMVECTOR& moveVector);
};