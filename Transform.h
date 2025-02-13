#pragma once

#include <DirectXMath.h>

class Transform
{
public:
	Transform();

	// Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3& position);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(const DirectX::XMFLOAT3& rotation);
	void SetScale(float x, float y, float z);
	void SetScale(const DirectX::XMFLOAT3& scale);

	// Getters
	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT3 GetScale() const;
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();
	DirectX::XMFLOAT3 GetPitchYawRoll() const;

	// Transformers
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(const DirectX::XMFLOAT3& offset);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 rotation);
	void Scale(float x, float y, float z);
	void Scale(const DirectX::XMFLOAT3& scale);

	// We need to give users direct control over our transformation variables via imgui
	friend class Game;

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMFLOAT3 m_scale;

	DirectX::XMFLOAT4X4 m_worldMatrix;
	DirectX::XMFLOAT4X4 m_worldInverseTransposeMatrix;

	bool m_isDirty;

	void updateWorldMatrix();
};