#include "Entity.h"
#include "Graphics.h"
#include "BufferStructs.h"

Entity::Entity(const std::shared_ptr<Mesh>& mesh) : 
	m_colorTint(1.0f, 1.0f, 1.0f, 1.0f)
{
	m_mesh = mesh;
	m_transform = std::make_shared<Transform>();
}

void Entity::Draw(const Microsoft::WRL::ComPtr<ID3D11Buffer>& constantBuffer,
	const std::shared_ptr<Camera>& camera)
{

	VertexShaderData vertexShaderData{};
	vertexShaderData.world = m_transform->GetWorldMatrix();
	vertexShaderData.colorTint = m_colorTint;
	vertexShaderData.view = camera->GetViewMatrix();
	vertexShaderData.projection = camera->GetProjectionMatrix();

	// Update constant buffer data
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	Graphics::Context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vertexShaderData, sizeof(vertexShaderData));
	Graphics::Context->Unmap(constantBuffer.Get(), 0);

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
