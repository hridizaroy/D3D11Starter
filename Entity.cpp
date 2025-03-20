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
	m_material->PrepareMaterial();

	std::shared_ptr<SimpleVertexShader> vs = m_material->GetVertexShader();
	std::shared_ptr<SimplePixelShader> ps = m_material->GetPixelShader();

	vs->SetMatrix4x4("world", m_transform->GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	ps->SetFloat4("colorTint", m_material->GetColorTint());
	ps->SetFloat2("uvScale", m_material->GetUVScale());
	ps->SetFloat2("uvOffset", m_material->GetUVOffset());

	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	vs->SetShader();
	ps->SetShader();

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
