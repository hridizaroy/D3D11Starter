#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"

class Entity
{
public:
	Entity(const std::shared_ptr<Mesh>& mesh);

	void Draw(const Microsoft::WRL::ComPtr<ID3D11Buffer>& constantBuffer);

	// Getters
	std::shared_ptr<Mesh> GetMesh() const;
	std::shared_ptr<Transform> GetTransform() const;

private:
	std::shared_ptr<Transform> m_transform;
	std::shared_ptr<Mesh> m_mesh;
	DirectX::XMFLOAT4 m_colorTint;
};
