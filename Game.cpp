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
	CreateShadowMapSetup();
	CreatePostProcessSetup();

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

	lights.resize(5);

	lights[0].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[0].Direction = XMFLOAT3(0.0, -1.0, 0.0);
	lights[0].Color = XMFLOAT3(0.8f, 1.0f, 0.8f);
	lights[0].Intensity = 0.5f;

	lights[1].Type = LIGHT_TYPE_SPOT;
	lights[1].Position = XMFLOAT3(-1.0, 0.0, -0.5);
	lights[1].Direction = XMFLOAT3(0.0, 0.0, 1.0);
	lights[1].Color = XMFLOAT3(0.0f, 0.0f, 1.0f);
	lights[1].Intensity = 0.5f;
	lights[1].Range = 10.0;
	lights[1].SpotInnerAngle = XM_PI / 10.0;
	lights[1].SpotOuterAngle = XM_PI / 5.0;

	lights[2].Type = LIGHT_TYPE_POINT;
	lights[2].Position = XMFLOAT3(3.0, 0.0, -20.0);
	lights[2].Color = XMFLOAT3(0.5f, 0.5f, 0.2f);
	lights[2].Intensity = 0.5f;
	lights[2].Range = 40.0;

	lights[3].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[3].Direction = XMFLOAT3(1.0, 1.0, 0.0);
	lights[3].Color = XMFLOAT3(0.2f, 1.0f, 1.0f);
	lights[3].Intensity = 0.5f;

	lights[4].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[4].Direction = XMFLOAT3(0.5, -1.0, 0.0);
	lights[4].Color = XMFLOAT3(0.8f, 0.5f, 0.2f);
	lights[4].Intensity = 2.0f;

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

	blurRadius = 5;

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

	// Vertex Shaders
	std::shared_ptr<SimpleVertexShader> vs = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	std::shared_ptr<SimpleVertexShader> skyVS = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"SkyVertexShader.cso").c_str());
	shadowMapVS = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"ShadowMapVS.cso").c_str());
	ppVS = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PostProcessVS.cso").c_str());

	// Pixel Shaders
	std::shared_ptr<SimplePixelShader> ps = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> skyPS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"SkyPixelShader.cso").c_str());
	blurPS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PostProcessPS.cso").c_str());
	caPS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"chromaticAberPS.cso").c_str());

	// Load textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneNormalsSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneMetalnessSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintNormalsSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintMetalnessSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedNormalsSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedMetalnessSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodNormalsSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodMetalnessSRV;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_albedo.png").c_str(), nullptr, cobblestoneAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_normals.png").c_str(), nullptr, cobblestoneNormalsSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_roughness.png").c_str(), nullptr, cobblestoneRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_metal.png").c_str(), nullptr, cobblestoneMetalnessSRV.GetAddressOf());

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_albedo.png").c_str(), nullptr, paintAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_normals.png").c_str(), nullptr, paintNormalsSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_roughness.png").c_str(), nullptr, paintRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_metal.png").c_str(), nullptr, paintMetalnessSRV.GetAddressOf());

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/scratched_albedo.png").c_str(), nullptr, scratchedAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/scratched_normals.png").c_str(), nullptr, scratchedNormalsSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/scratched_roughness.png").c_str(), nullptr, scratchedRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/scratched_metal.png").c_str(), nullptr, scratchedMetalnessSRV.GetAddressOf());

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_albedo.png").c_str(), nullptr, woodAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_normals.png").c_str(), nullptr, woodNormalsSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_roughness.png").c_str(), nullptr, woodRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_metal.png").c_str(), nullptr, woodMetalnessSRV.GetAddressOf());

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
	materials[0] = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vs, ps, 0.0f);
	materials[1] = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vs, ps, 1.0f);
	materials[2] = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vs, ps, 0.0f);
	materials[3] = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vs, ps, 1.0f);

	materials[0]->AddSampler("BasicSampler", samplerState);
	materials[0]->AddTextureSRV("Albedo", cobblestoneAlbedoSRV);
	materials[0]->AddTextureSRV("NormalMap", cobblestoneNormalsSRV);
	materials[0]->AddTextureSRV("RoughnessMap", cobblestoneRoughnessSRV);
	materials[0]->AddTextureSRV("MetalnessMap", cobblestoneMetalnessSRV);

	materials[1]->AddSampler("BasicSampler", samplerState);
	materials[1]->AddTextureSRV("Albedo", paintAlbedoSRV);
	materials[1]->AddTextureSRV("NormalMap", paintNormalsSRV);
	materials[1]->AddTextureSRV("RoughnessMap", paintRoughnessSRV);
	materials[1]->AddTextureSRV("MetalnessMap", paintMetalnessSRV);

	materials[2]->AddSampler("BasicSampler", samplerState);
	materials[2]->AddTextureSRV("Albedo", scratchedAlbedoSRV);
	materials[2]->AddTextureSRV("NormalMap", scratchedNormalsSRV);
	materials[2]->AddTextureSRV("RoughnessMap", scratchedRoughnessSRV);
	materials[2]->AddTextureSRV("MetalnessMap", scratchedMetalnessSRV);

	materials[3]->AddSampler("BasicSampler", samplerState);
	materials[3]->AddTextureSRV("Albedo", woodAlbedoSRV);
	materials[3]->AddTextureSRV("NormalMap", woodNormalsSRV);
	materials[3]->AddTextureSRV("RoughnessMap", woodRoughnessSRV);
	materials[3]->AddTextureSRV("MetalnessMap", woodMetalnessSRV);

	meshes[0] = std::make_shared<Mesh>(FixPath("../../Assets/Models/sphere.obj").c_str(), "Sphere");
	meshes[1] = std::make_shared<Mesh>(FixPath("../../Assets/Models/cube.obj").c_str(), "Cube");
	meshes[2] = std::make_shared<Mesh>(FixPath("../../Assets/Models/torus.obj").c_str(), "Torus");
	meshes[3] = std::make_shared<Mesh>(FixPath("../../Assets/Models/helix.obj").c_str(), "Helix");
	meshes[4] = std::make_shared<Mesh>(FixPath("../../Assets/Models/cylinder.obj").c_str(), "Cylinder");

	scene[0] = std::make_shared<Entity>(meshes[0], materials[1]);
	scene[1] = std::make_shared<Entity>(meshes[1], materials[1]);
	scene[2] = std::make_shared<Entity>(meshes[2], materials[1]);
	scene[3] = std::make_shared<Entity>(meshes[3], materials[1]);
	scene[4] = std::make_shared<Entity>(meshes[4], materials[1]);

	scene[5] = std::make_shared<Entity>(meshes[0], materials[0]);
	scene[6] = std::make_shared<Entity>(meshes[1], materials[0]);
	scene[7] = std::make_shared<Entity>(meshes[2], materials[0]);
	scene[8] = std::make_shared<Entity>(meshes[3], materials[0]);
	scene[9] = std::make_shared<Entity>(meshes[4], materials[0]);

	scene[10] = std::make_shared<Entity>(meshes[0], materials[2]);
	scene[11] = std::make_shared<Entity>(meshes[1], materials[2]);
	scene[12] = std::make_shared<Entity>(meshes[2], materials[2]);
	scene[13] = std::make_shared<Entity>(meshes[3], materials[2]);
	scene[14] = std::make_shared<Entity>(meshes[4], materials[2]);

	// floor
	scene[15] = std::make_shared<Entity>(meshes[1], materials[3]);

	// Create sky
	sky = std::make_shared<Sky>(
		meshes[1],
		samplerState,
		FixPath(L"../../Assets/Textures/CubeMaps/Clouds_Blue/right.png").c_str(),
		FixPath(L"../../Assets/Textures/CubeMaps/Clouds_Blue/left.png").c_str(),
		FixPath(L"../../Assets/Textures/CubeMaps/Clouds_Blue/up.png").c_str(),
		FixPath(L"../../Assets/Textures/CubeMaps/Clouds_Blue/down.png").c_str(),
		FixPath(L"../../Assets/Textures/CubeMaps/Clouds_Blue/front.png").c_str(),
		FixPath(L"../../Assets/Textures/CubeMaps/Clouds_Blue/back.png").c_str(),
		skyPS,
		skyVS);
}

void Game::CreateShadowMapSetup()
{
	// Shadow texture
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = 1024;
	shadowDesc.Height = 1024;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;


	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	Graphics::Device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());

	// SRV for shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	Graphics::Device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	Graphics::Device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	Graphics::Device->CreateSamplerState(&shadowSampDesc, &shadowSampler);
}

void Game::UpdateLightMatrices()
{
	// Update light view and projection matrices for shadow map
	XMVECTOR lightDirection = XMLoadFloat3(&lights[4].Direction);

	XMMATRIX lightView = XMMatrixLookToLH(
		-lightDirection * 30,
		lightDirection,
		XMVectorSet(0, 1, 0, 0)
	);

	float lightProjectionSize = 20.0f;
	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f);

	XMStoreFloat4x4(&lightViewMatrix, lightView);
	XMStoreFloat4x4(&lightProjectionMatrix, lightProjection);
}

void Game::PopulateShadowMap()
{
	// Set settings for shadow map
	Graphics::Context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	ID3D11RenderTargetView* nullRTV{};
	Graphics::Context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());
	Graphics::Context->PSSetShader(0, 0, 0);

	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)1024;
	viewport.Height = (float)1024;
	viewport.MaxDepth = 1.0f;
	Graphics::Context->RSSetViewports(1, &viewport);

	Graphics::Context->RSSetState(shadowRasterizer.Get());

	shadowMapVS->SetShader();
	shadowMapVS->SetMatrix4x4("view", lightViewMatrix);
	shadowMapVS->SetMatrix4x4("projection", lightProjectionMatrix);
	// Loop and draw all entities
	for (const auto& entity : scene)
	{
		shadowMapVS->SetMatrix4x4("world", entity->GetTransform()->GetWorldMatrix());
		shadowMapVS->CopyAllBufferData();

		entity->GetMesh()->Draw();
	}


	// Reset
	viewport.Width = (float)Window::Width();
	viewport.Height = (float)Window::Height();
	Graphics::Context->RSSetViewports(1, &viewport);
	Graphics::Context->OMSetRenderTargets(
		1,
		Graphics::BackBufferRTV.GetAddressOf(),
		Graphics::DepthBufferDSV.Get());
	Graphics::Context->RSSetState(0);
}

void Game::CreatePostProcessSetup()
{
	// Reset ComPtrs
	blurSRV.Reset();
	blurRTV.Reset();
	caSRV.Reset();
	caRTV.Reset();

	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	Graphics::Device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());

	// Describe the texture we're creating
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = Window::Width();
	textureDesc.Height = Window::Height();
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> blurTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> caTexture;
	Graphics::Device->CreateTexture2D(&textureDesc, 0, blurTexture.GetAddressOf());
	Graphics::Device->CreateTexture2D(&textureDesc, 0, caTexture.GetAddressOf());

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Graphics::Device->CreateRenderTargetView(
		blurTexture.Get(),
		&rtvDesc,
		blurRTV.ReleaseAndGetAddressOf());

	Graphics::Device->CreateRenderTargetView(
		caTexture.Get(),
		&rtvDesc,
		caRTV.ReleaseAndGetAddressOf());

	// Create the Shader Resource View
	// By passing it a null description for the SRV, we
	// get a "default" SRV that has access to the entire resource
	Graphics::Device->CreateShaderResourceView(
		blurTexture.Get(),
		0,
		blurSRV.ReleaseAndGetAddressOf());
	Graphics::Device->CreateShaderResourceView(
		caTexture.Get(),
		0,
		caSRV.ReleaseAndGetAddressOf());
}


// --------------------------------------------------------
// Handle resizing to match the new window size
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

	if (Graphics::Device.Get())
	{
		CreatePostProcessSetup();
	}	
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	UpdateImGui(deltaTime);
	BuildUI(totalTime);

	scene[0]->GetTransform()->SetPosition(-2.0f, -1.0f + sin(totalTime), 0.0f);
	scene[0]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[1]->GetTransform()->SetPosition(-1.0f, -1.0f + sin(totalTime), 0.0f);
	scene[1]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[2]->GetTransform()->SetPosition(-0.3f, -1.0f + sin(totalTime), 0.0f);
	scene[2]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[3]->GetTransform()->SetPosition(0.5f, -1.0f + sin(totalTime), 0.0f);
	scene[3]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[4]->GetTransform()->SetPosition(1.3f, -1.0f + sin(totalTime), 0.0f);
	scene[4]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);


	scene[5]->GetTransform()->SetPosition(-2.0f, 0.0f + sin(totalTime), 3.0f);
	scene[5]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[6]->GetTransform()->SetPosition(-1.0f, 0.0f + sin(totalTime), 3.0f);
	scene[6]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[7]->GetTransform()->SetPosition(-0.3f, 0.0f + sin(totalTime), 3.0f);
	scene[7]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[8]->GetTransform()->SetPosition(0.5f, 0.0f + sin(totalTime), 3.0f);
	scene[8]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[9]->GetTransform()->SetPosition(1.3f, 0.0f + sin(totalTime), 3.0f);
	scene[9]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);


	scene[10]->GetTransform()->SetPosition(-2.0f, 1.0f + sin(totalTime), -3.0f);
	scene[10]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[11]->GetTransform()->SetPosition(-1.0f, 1.0f + sin(totalTime), -3.0f);
	scene[11]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[12]->GetTransform()->SetPosition(-0.3f, 1.0f + sin(totalTime), -3.0f);
	scene[12]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[13]->GetTransform()->SetPosition(0.5f, 1.0f + sin(totalTime), -3.0f);
	scene[13]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[14]->GetTransform()->SetPosition(1.3f, 1.0f + sin(totalTime), -3.0f);
	scene[14]->GetTransform()->SetScale(0.3f, 0.3f, 0.3f);

	scene[15]->GetTransform()->SetPosition(0.0f, -2.5f, 0.0f);
	scene[15]->GetTransform()->SetScale(15.0f, 0.2f, 15.0f);

	cameras[activeCameraIdx]->Update(deltaTime);

	UpdateLightMatrices();

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

	ImGui::DragInt("Blur Radius", &blurRadius, 1.0f, 0, 25);

	// Light
	if (ImGui::CollapsingHeader("Light Data"))
	{
		for (size_t ii = 0; ii < lights.size(); ii++)
		{
			std::string header = "Light " + std::to_string(ii) + " color";
			ImGui::ColorEdit3(header.c_str(), &lights[ii].Color.x);
		}
	}

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

	ImGui::Image((ImTextureID)shadowSRV.Get(), ImVec2(512, 512));

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
		Graphics::Context->ClearRenderTargetView(blurRTV.Get(), backgroundColor);
		Graphics::Context->ClearRenderTargetView(caRTV.Get(), backgroundColor);

		PopulateShadowMap();

		Graphics::Context->OMSetRenderTargets(1, blurRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());
	}

	// DRAW geometry
	{
		for (const std::shared_ptr<Entity>& entity : scene)
		{
			std::shared_ptr<SimpleVertexShader> vs = entity->GetMaterial()->GetVertexShader();
			std::shared_ptr<SimplePixelShader> ps = entity->GetMaterial()->GetPixelShader();

			ps->SetShaderResourceView("ShadowMap", shadowSRV);
			ps->SetSamplerState("ShadowSampler", shadowSampler);

			vs->SetMatrix4x4("lightView", lightViewMatrix);
			vs->SetMatrix4x4("lightProjection", lightProjectionMatrix);

			ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
			ps->SetInt("numLights", (int)lights.size());

			entity->Draw(cameras[activeCameraIdx]);
		}
	}

	// Draw Sky
	{
		sky->Draw(cameras[activeCameraIdx]);
	}

	// Post Processing - Blur
	{
		Graphics::Context->OMSetRenderTargets(1, caRTV.GetAddressOf(), 0);

		ppVS->SetShader();
		blurPS->SetShader();

		blurPS->SetShaderResourceView("Pixels", blurSRV.Get());
		blurPS->SetSamplerState("ClampSampler", ppSampler.Get());

		// Activate shaders and bind resources
		blurPS->SetInt("blurRadius", blurRadius);
		blurPS->SetFloat("pixelWidth", 1.0f / Window::Width());
		blurPS->SetFloat("pixelHeight", 1.0f / Window::Height());

		blurPS->CopyAllBufferData();

		Graphics::Context->Draw(3, 0);
	}

	// Post Processing - Chromatic Aberration
	{
		Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), 0);
		caPS->SetShader();

		caPS->SetShaderResourceView("Pixels", caSRV.Get());
		caPS->SetSamplerState("ClampSampler", ppSampler.Get());

		caPS->CopyAllBufferData();

		Graphics::Context->Draw(3, 0);
	}

	{
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
	}
	
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

		ID3D11ShaderResourceView* nullSRVs[128] = {};
		Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);
	}
}
