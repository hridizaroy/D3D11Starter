#pragma once

#include <memory>
#include <DirectXMath.h>
#include "SimpleShader.h"

class Material
{
public:
	Material(const DirectX::XMFLOAT4 colorTint,
		const std::shared_ptr<SimpleVertexShader> vertexShader,
		const std::shared_ptr<SimplePixelShader> pixelShader);

	DirectX::XMFLOAT4 GetColorTint() const;
	std::shared_ptr<SimpleVertexShader> GetVertexShader() const;
	std::shared_ptr<SimplePixelShader> GetPixelShader() const;

private:
	DirectX::XMFLOAT4 m_colorTint;
	std::shared_ptr<SimpleVertexShader> m_vertexShader;
	std::shared_ptr<SimplePixelShader> m_pixelShader;
};