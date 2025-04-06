#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"

class Sky
{
public:
	Sky(const std::shared_ptr<Mesh> mesh,
		const Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState,
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		std::shared_ptr<SimplePixelShader> ps,
		std::shared_ptr<SimpleVertexShader> vs);
	~Sky();

	void Draw(std::shared_ptr<Camera> camera);

private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubeMapSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
	
	std::shared_ptr<Mesh> m_mesh;
	std::shared_ptr<SimplePixelShader> m_ps;
	std::shared_ptr<SimpleVertexShader> m_vs;

	// Provided code
	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);
};
