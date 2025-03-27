#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"

#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "SimpleShader.h"
#include "Material.h"

#include "WICTextureLoader.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	CreateEntities();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// create cameras;
	cameras.reserve(3);
	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(-0.5f, 0.0f, -3.0f)));
	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0.5f, -0.3f, -5.0f)));
	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0.8f, 0.4f, -0.5f)));

	cameras[1]->SetFOV(XM_PIDIV2); // 90 degrees
	cameras[2]->SetFOV(XM_PI / 3.0f); // 60 degrees

	activeCameraIdx = 0;


	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());

	numSecs = 0;
	fps = 0;
	demoWindowVisible = true;
	backgroundColor[0] = 0.4f;
	backgroundColor[1] = 0.6f;
	backgroundColor[2] = 0.75f;
	backgroundColor[3] = 0.0f;
	darkModeEnabled = true;

	if (!darkModeEnabled)
	{
		ImGui::StyleColorsLight();
	}
	else
	{
		ImGui::StyleColorsDark();
	}
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


// --------------------------------------------------------
// Creates the entities we're going to draw
// --------------------------------------------------------
void Game::CreateEntities()
{
	// Load shaders
	std::shared_ptr<SimpleVertexShader> vs = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());

	// Pixel shaders
	std::shared_ptr<SimplePixelShader> ps = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> ps_decal = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DecalPS.cso").c_str());

	// Load textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> grassSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> decalSRV;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/grass.webp").c_str(), nullptr, grassSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/wood.jpg").c_str(), nullptr, woodSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/decal.jpg").c_str(), nullptr, decalSRV.GetAddressOf());

	// Create Sampler State
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Graphics::Device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());


	// Create Materials
	materials[0] = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vs, ps, 0.5);
	materials[1] = std::make_shared<Material>(XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), vs, ps, 1.0);
	materials[2] = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), vs, ps_decal, 0.0);
	materials[3] = std::make_shared<Material>(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), vs, ps_decal, 0.2);
	materials[4] = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vs, ps, 0.8);

	materials[0]->AddSampler("BasicSampler", samplerState);
	materials[0]->AddTextureSRV("SurfaceTexture", woodSRV);

	materials[1]->AddSampler("BasicSampler", samplerState);
	materials[1]->AddTextureSRV("SurfaceTexture", woodSRV);

	materials[2]->AddSampler("BasicSampler", samplerState);
	materials[2]->AddTextureSRV("SurfaceTexture", woodSRV);
	materials[2]->AddTextureSRV("DecalTexture", decalSRV);

	materials[3]->AddSampler("BasicSampler", samplerState);
	materials[3]->AddTextureSRV("SurfaceTexture", grassSRV);
	materials[3]->AddTextureSRV("DecalTexture", decalSRV);
	
	materials[4]->AddSampler("BasicSampler", samplerState);
	materials[4]->AddTextureSRV("SurfaceTexture", grassSRV);

	meshes[0] = std::make_shared<Mesh>(FixPath("../../Assets/Models/sphere.obj").c_str(), "Sphere");
	meshes[1] = std::make_shared<Mesh>(FixPath("../../Assets/Models/cube.obj").c_str(), "Cube");
	meshes[2] = std::make_shared<Mesh>(FixPath("../../Assets/Models/torus.obj").c_str(), "Torus");
	meshes[3] = std::make_shared<Mesh>(FixPath("../../Assets/Models/helix.obj").c_str(), "Helix");
	meshes[4] = std::make_shared<Mesh>(FixPath("../../Assets/Models/cylinder.obj").c_str(), "Cylinder");

	// UV shader
	scene[0] = std::make_shared<Entity>(meshes[0], materials[1]);
	scene[1] = std::make_shared<Entity>(meshes[1], materials[1]);
	scene[2] = std::make_shared<Entity>(meshes[2], materials[1]);
	scene[3] = std::make_shared<Entity>(meshes[3], materials[1]);
	scene[4] = std::make_shared<Entity>(meshes[4], materials[1]);

	// Normal shader
	scene[5] = std::make_shared<Entity>(meshes[0], materials[2]);
	scene[6] = std::make_shared<Entity>(meshes[1], materials[2]);
	scene[7] = std::make_shared<Entity>(meshes[2], materials[2]);
	scene[8] = std::make_shared<Entity>(meshes[3], materials[2]);
	scene[9] = std::make_shared<Entity>(meshes[4], materials[2]);

	// Custom shader and colorTint shader
	scene[10] = std::make_shared<Entity>(meshes[0], materials[0]);
	scene[11] = std::make_shared<Entity>(meshes[1], materials[4]);
	scene[12] = std::make_shared<Entity>(meshes[2], materials[3]);
	scene[13] = std::make_shared<Entity>(meshes[3], materials[3]);
	scene[14] = std::make_shared<Entity>(meshes[4], materials[3]);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	for (const auto& camera : cameras)
	{
		if (camera)
		{
			camera->UpdateProjectionMatrix(Window::AspectRatio());
		}
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	UpdateImGui(deltaTime);
	BuildUI(totalTime);

	scene[0]->GetTransform()->SetPosition(-2.0f, -1.0f, 0.0f);
	scene[0]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[1]->GetTransform()->SetPosition(-1.0f, -1.0f, 0.0f);
	scene[1]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[2]->GetTransform()->SetPosition(-0.3f, -1.0f, 0.0f);
	scene[2]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[3]->GetTransform()->SetPosition(0.5f, -1.0f, 0.0f);
	scene[3]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[4]->GetTransform()->SetPosition(1.3f, -1.0f, 0.0f);
	scene[4]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);


	scene[5]->GetTransform()->SetPosition(-2.0f, 0.0f, 0.0f);
	scene[5]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[6]->GetTransform()->SetPosition(-1.0f, 0.0f, 0.0f);
	scene[6]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[7]->GetTransform()->SetPosition(-0.3f, 0.0f, 0.0f);
	scene[7]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[8]->GetTransform()->SetPosition(0.5f, 0.0f, 0.0f);
	scene[8]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[9]->GetTransform()->SetPosition(1.3f, 0.0f, 0.0f);
	scene[9]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);


	scene[10]->GetTransform()->SetPosition(-2.0f, 1.0f, 0.0f);
	scene[10]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[11]->GetTransform()->SetPosition(-1.0f, 1.0f, 0.0f);
	scene[11]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[12]->GetTransform()->SetPosition(-0.3f, 1.0f, 0.0f);
	scene[12]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[13]->GetTransform()->SetPosition(0.5f, 1.0f, 0.0f);
	scene[13]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[14]->GetTransform()->SetPosition(1.3f, 1.0f, 0.0f);
	scene[14]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	cameras[activeCameraIdx]->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
}

void Game::UpdateImGui(float deltaTime)
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window conditionally
	if (demoWindowVisible)
	{
		ImGui::ShowDemoWindow();
	}
}

void Game::BuildUI(float totalTime)
{
	ImGui::Begin("Engine data");

	if (!darkModeEnabled)
	{
		ImGui::StyleColorsLight();
	}
	else
	{
		ImGui::StyleColorsDark();
	}
	
	ImGui::Text("Info:");

	// Update FPS every second
	size_t newNumSecs = (size_t)totalTime;
	if (newNumSecs > numSecs)
	{
		fps = (size_t)ImGui::GetIO().Framerate;
		numSecs = newNumSecs;
	}

	ImGui::Text("Framerate: %d fps", fps);

	// Window Dimensions
	ImGui::Text("Window Dimensions: %dx%d", Window::Width(), Window::Height());

	// Toggle button for demo window
	if (ImGui::Button("Toggle demo window visibility"))
	{
		demoWindowVisible = !demoWindowVisible;
	}

	// Color picker for changing background color
	ImGui::ColorEdit4("RGBA color editor", backgroundColor);

	// Checkbox for dark mode
	ImGui::Checkbox("Dark mode", &darkModeEnabled);

	// Camera switch
	std::vector<std::string> cameraOptions;
	size_t numCameras = cameras.size();
	cameraOptions.reserve(numCameras);
	for (size_t ii = 0; ii < numCameras; ii++)
	{
		cameraOptions.push_back("Camera " + std::to_string(ii + 1));
	}
	std::vector<const char*> cameraOptionPtrs;
	cameraOptionPtrs.reserve(numCameras);

	for (size_t ii = 0; ii < numCameras; ii++)
	{
		cameraOptionPtrs.push_back(cameraOptions[ii].c_str());
	}

	ImGui::Combo("Select Camera", &activeCameraIdx, cameraOptionPtrs.data(), static_cast<int>(numCameras));

	// Active camera info
	if (ImGui::CollapsingHeader("Camera Data"))
	{
		std::shared_ptr<Camera>& activeCamera = cameras[activeCameraIdx];

		ImGui::Text("Position: %f, %f, %f",
			activeCamera->m_transform->m_position.x,
			activeCamera->m_transform->m_position.y, 
			activeCamera->m_transform->m_position.z);
		ImGui::Text("Field of view: %f", activeCamera->m_fov);
		ImGui::Text("Rotation: %f, %f, %f",
			activeCamera->m_transform->m_rotation.x,
			activeCamera->m_transform->m_rotation.y,
			activeCamera->m_transform->m_rotation.z);
	}

	// Mesh info
	if (ImGui::CollapsingHeader("Meshes"))
	{
		for (const std::shared_ptr<Mesh>& mesh : meshes)
		{
			if (ImGui::CollapsingHeader(("Mesh: " + mesh->GetMeshName()).c_str()))
			{
				unsigned int numVertices = mesh->GetVertexCount();
				unsigned int numIndices = mesh->GetIndexCount();
				ImGui::Text("Triangles: %d", numIndices / 3);
				ImGui::Text("Vertices: %d", numVertices);
				ImGui::Text("Indices: %d", numIndices);
			}
		}
	}

	// Entity Info
	if (ImGui::CollapsingHeader("Scene Entities"))
	{
		uint32_t idx = 0;
		for (const std::shared_ptr<Entity>& entity : scene)
		{
			std::string header = "Entity " + std::to_string(idx);
			std::string posHeader = "Position##pos" + std::to_string(idx);
			std::string rotHeader = "Rotation (radians)##rot" + std::to_string(idx);
			std::string scaleHeader = "Scale##scale" + std::to_string(idx);

			if (ImGui::CollapsingHeader(header.c_str()))
			{
				ImGui::DragFloat3(posHeader.c_str(), &entity->GetTransform()->m_position.x);
				ImGui::DragFloat3(rotHeader.c_str(), &entity->GetTransform()->m_rotation.x);
				ImGui::DragFloat3(scaleHeader.c_str(), & entity->GetTransform()->m_scale.x);
			}

			idx++;
		}
	}

	// Material Info
	if (ImGui::CollapsingHeader("Materials"))
	{
		uint32_t idx = 0;
		ImVec2 imageSize(200, 200);

		for (const std::shared_ptr<Material>& material : materials)
		{
			std::string header = "Material " + std::to_string(idx);
			std::string colTintHeader = "Color tint##colTint" + std::to_string(idx);
			std::string uvScaleHeader = "UV Scale##scale" + std::to_string(idx);
			std::string uvOffsetHeader = "UV Offset##offset" + std::to_string(idx);

			if (ImGui::CollapsingHeader(header.c_str()))
			{
				ImGui::DragFloat4(colTintHeader.c_str(), &material->m_colorTint.x, 0.01f);
				ImGui::DragFloat2(uvScaleHeader.c_str(), &material->m_uvScale.x, 0.01f);
				ImGui::DragFloat2(uvOffsetHeader.c_str(), &material->m_uvOffset.x, 0.01f);

				for (const auto& t : material->m_textureSRVs)
				{
					ImGui::Image((ImTextureID)t.second.Get(), imageSize);
				}
			}

			idx++;
		}
	}

	ImGui::End();
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry
	{
		for (const std::shared_ptr<Entity>& entity : scene)
		{
			entity->Draw(cameras[activeCameraIdx]);
		}
	}

	ImGui::Render(); // Turns this frame’s UI into renderable triangles
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}



