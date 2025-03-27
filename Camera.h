#pragma once

#include <memory>
#include "Transform.h"

class Camera
{
public:
	Camera(const float aspectRatio, const DirectX::XMFLOAT3& position);

	// getters
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	std::shared_ptr<Transform> GetTransform();

	// setters
	void SetFOV(const float fov);

	// update
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(const float aspectRatio);

	void Update(const float dt);

	// We need to give imgui access to our info
	friend class Game;

private:
	std::shared_ptr<Transform> m_transform;
	DirectX::XMFLOAT4X4 m_viewMatrix;
	DirectX::XMFLOAT4X4 m_projectionMatrix;

	float m_fov;
	float m_nearDist;
	float m_farDist;
	float m_moveSpeed;
	float m_mouseLookSpeed;

	bool m_isPerspective;
};