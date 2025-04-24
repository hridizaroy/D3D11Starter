#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Mesh.h"
#include <memory>
#include <vector>

#include "Entity.h"
#include "Camera.h"
#include "Lights.h"
#include "Sky.h"

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:
	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void CreateEntities();

	// UI-related functions
	void UpdateImGui(float deltaTime);
	void BuildUI(float totalTime);

	// Shadow Map helper functions
	void CreateShadowMapSetup();
	void UpdateLightMatrices();
	void PopulateShadowMap();

	// ImGui UI related variables
	size_t numSecs;
	size_t fps;
	bool demoWindowVisible;
	float backgroundColor[4];
	bool darkModeEnabled;

	// Scene
	std::shared_ptr<Mesh> meshes[5];
	std::shared_ptr<Entity> scene[16];
	std::shared_ptr<Material> materials[4];
	std::vector<std::shared_ptr<Camera>> cameras;
	std::vector<Light> lights;
	std::shared_ptr<Sky> sky;

	int activeCameraIdx;

	// Shadow Map
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;

	std::shared_ptr<SimpleVertexShader> shadowMapVS;
};

