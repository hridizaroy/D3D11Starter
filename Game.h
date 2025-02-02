#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Mesh.h"
#include <memory>
#include <vector>

#include "BufferStructs.h"

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
	void LoadShaders();
	void CreateGeometry();
	void CreateConstantBuffer();

	// UI-related functions
	void UpdateImGui(float deltaTime);
	void BuildUI(float totalTime);

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
	
	VertexShaderData vertexShaderData;


	// ImGui UI related variables
	size_t numSecs;
	size_t fps;
	bool demoWindowVisible;
	float backgroundColor[4];
	bool darkModeEnabled;

	// Scene
	std::vector<std::shared_ptr<Mesh>> scene;

};

