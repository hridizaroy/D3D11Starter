#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Mesh.h"
#include <memory>
#include <vector>

#include "Entity.h"
#include "Camera.h"

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

	// ImGui UI related variables
	size_t numSecs;
	size_t fps;
	bool demoWindowVisible;
	float backgroundColor[4];
	bool darkModeEnabled;

	// Scene
	std::shared_ptr<Mesh> meshes[3];
	std::shared_ptr<Entity> scene[5];
	std::vector<std::shared_ptr<Camera>> cameras;

	int activeCameraIdx;

};

