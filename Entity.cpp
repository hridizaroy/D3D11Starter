#include "Entity.h"
#include "Graphics.h"

Entity::Entity(const std::shared_ptr<Mesh>& mesh,
	const std::shared_ptr<Material>& material) :
	m_colorTint(1.0f, 1.0f, 1.0f, 1.0f)
{
	m_mesh = mesh;
	m_transform = std::make_shared<Transform>();
	m_material = material;
}

void Entity::Draw(const std::shared_ptr<Camera>& camera)
{
	std::shared_ptr<SimpleVertexShader> vs = m_material->GetVertexShader();
	vs->SetFloat4("colorTint",m_material->GetColorTint());
	vs->SetMatrix4x4("world", m_transform->GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());

	vs->CopyAllBufferData();

	vs->SetShader();
	m_material->GetPixelShader()->SetShader();

	m_mesh->Draw();
}

std::shared_ptr<Mesh> Entity::GetMesh() const
{
	return m_mesh;
}

std::shared_ptr<Transform> Entity::GetTransform() const
{
	return m_transform;
}

std::shared_ptr<Material> Entity::GetMaterial() const
{
	return m_material;
}
