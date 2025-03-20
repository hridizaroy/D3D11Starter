#pragma once

#include <memory>
#include <unordered_map>
#include <DirectXMath.h>
#include "SimpleShader.h"

class Material
{
public:
	Material(const DirectX::XMFLOAT4 colorTint,
		const std::shared_ptr<SimpleVertexShader> vertexShader,
		const std::shared_ptr<SimplePixelShader> pixelShader,
		const DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1.0, 1.0),
		const DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0.0, 0.0));

	DirectX::XMFLOAT4 GetColorTint() const;
	DirectX::XMFLOAT2 GetUVScale() const;
	DirectX::XMFLOAT2 GetUVOffset() const;

	std::shared_ptr<SimpleVertexShader> GetVertexShader() const;
	std::shared_ptr<SimplePixelShader> GetPixelShader() const;

	void AddTextureSRV(const std::string shaderVarName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(const std::string shaderVarName, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState);

	void PrepareMaterial();

	// We need to give users direct control over variables via imgui
	friend class Game;

private:
	DirectX::XMFLOAT4 m_colorTint;
	DirectX::XMFLOAT2 m_uvScale;
	DirectX::XMFLOAT2 m_uvOffset;

	std::shared_ptr<SimpleVertexShader> m_vertexShader;
	std::shared_ptr<SimplePixelShader> m_pixelShader;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> m_samplers;
};