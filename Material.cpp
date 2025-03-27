#include "Material.h"

Material::Material(
	const DirectX::XMFLOAT4 colorTint,
	const std::shared_ptr<SimpleVertexShader> vertexShader,
	const std::shared_ptr<SimplePixelShader> pixelShader,
	const float roughness,
	const DirectX::XMFLOAT2 uvScale,
	const DirectX::XMFLOAT2 uvOffset) :
	m_colorTint(colorTint),
	m_vertexShader(vertexShader),
	m_pixelShader(pixelShader),
	m_roughness(roughness),
	m_uvScale(uvScale),
	m_uvOffset(uvOffset)
{
}

DirectX::XMFLOAT4 Material::GetColorTint() const
{
	return m_colorTint;
}

DirectX::XMFLOAT2 Material::GetUVScale() const
{
	return m_uvScale;
}

DirectX::XMFLOAT2 Material::GetUVOffset() const
{
	return m_uvOffset;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() const
{
	return m_vertexShader;
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader() const
{
	return m_pixelShader;
}

void Material::AddTextureSRV(const std::string shaderVarName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	m_textureSRVs.insert({ shaderVarName, srv });
}

void Material::AddSampler(const std::string shaderVarName, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState)
{
	m_samplers.insert({ shaderVarName, samplerState });
}

void Material::PrepareMaterial()
{
	m_pixelShader->SetFloat("roughness", m_roughness);

	for (const auto& t : m_textureSRVs)
	{
		m_pixelShader->SetShaderResourceView(t.first.c_str(), t.second);
	}

	for (const auto& s : m_samplers)
	{
		m_pixelShader->SetSamplerState(s.first.c_str(), s.second);
	}
}
