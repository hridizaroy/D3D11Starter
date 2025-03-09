#include "Material.h"

Material::Material(
	const DirectX::XMFLOAT4 colorTint,
	const std::shared_ptr<SimpleVertexShader> vertexShader,
	const std::shared_ptr<SimplePixelShader> pixelShader) :
	m_colorTint(colorTint),
	m_vertexShader(vertexShader),
	m_pixelShader(pixelShader)
{
}

DirectX::XMFLOAT4 Material::GetColorTint() const
{
	return m_colorTint;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() const
{
	return m_vertexShader;
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader() const
{
	return m_pixelShader;
}
