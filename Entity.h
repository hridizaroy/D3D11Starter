#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"

class Entity
{
public:
	Entity(const std::shared_ptr<Mesh>& mesh,
		const std::shared_ptr<Material>& material);

	void Draw(const std::shared_ptr<Camera>& camera);

	// Getters
	std::shared_ptr<Mesh> GetMesh() const;
	std::shared_ptr<Transform> GetTransform() const;
	std::shared_ptr<Material> GetMaterial() const;

private:
	std::shared_ptr<Transform> m_transform;
	std::shared_ptr<Mesh> m_mesh;
	DirectX::XMFLOAT4 m_colorTint;
	std::shared_ptr<Material> m_material;
};
