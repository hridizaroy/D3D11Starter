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
	LoadShaders();
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	CreateConstantBuffer();

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
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Colors
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 yellow = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 magenta = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 cyan = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	// Mesh data
	Vertex quadVertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
		{ XMFLOAT3(+0.6f, +0.5f, +0.0f), white }
	};

	unsigned int quadIndices[] = { 0, 1, 2, 0, 3, 1 };

	Vertex hexagonVertices[] =
	{
		{ XMFLOAT3(+0.8f, +0.9f, +0.0f), red },
		{ XMFLOAT3(+0.9f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(+0.75f, -0.7f, +0.0f), green },
		{ XMFLOAT3(+0.6f, -0.65f, +0.0f), white },
		{ XMFLOAT3(+0.55f, -0.6f, +0.0f), yellow },
		{ XMFLOAT3(+0.7f, +0.85f, +0.0f), magenta }
	};

	unsigned int hexagonIndices[] = {0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5};

	Vertex pentagonVertices[] =
	{
		{ XMFLOAT3(-0.55f, -0.6f, +0.0f), yellow },
		{ XMFLOAT3(-0.6f, -0.65f, +0.0f), white },
		{ XMFLOAT3(-0.75f, -0.7f, +0.0f), magenta },
		{ XMFLOAT3(-0.9f, -0.5f, +0.0f), cyan },
		{ XMFLOAT3(-0.8f, +0.9f, +0.0f), black }		
	};

	unsigned int pentagonIndices[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4 };

	std::shared_ptr<Mesh> quad = std::make_shared<Mesh>(quadVertices, 4,
									quadIndices, 6, "Quad");

	std::shared_ptr<Mesh> hexagon = std::make_shared<Mesh>(hexagonVertices, 6,
		hexagonIndices, 12, "Hexagon");

	std::shared_ptr<Mesh> pentagon = std::make_shared<Mesh>(pentagonVertices, 5,
		pentagonIndices, 9, "Pentagon");

	meshes[0] = quad;
	meshes[1] = pentagon;
	meshes[2] = hexagon;

	scene[0] = std::make_shared<Entity>(quad);
	scene[1] = std::make_shared<Entity>(quad);
	scene[2] = std::make_shared<Entity>(quad);
	scene[3] = std::make_shared<Entity>(hexagon);
	scene[4] = std::make_shared<Entity>(pentagon);
}

// --------------------------------------------------------
// Creates a constant buffer for our vertex shader
// --------------------------------------------------------
void Game::CreateConstantBuffer()
{
	unsigned int size = sizeof(VertexShaderData);
	size = (size + 15) / 16 * 16;

	// Describe the constant buffer
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = size;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;

	Graphics::Device->CreateBuffer(&cbDesc, 0, constantBuffer.GetAddressOf());

	Graphics::Context->VSSetConstantBuffers(
		0,
		1,
		constantBuffer.GetAddressOf()
	);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	UpdateImGui(deltaTime);
	BuildUI(totalTime);

	scene[0]->GetTransform()->SetScale((DirectX::XMScalarSin(deltaTime) / 2.0f + 6.0f), 0.5f, 0.5f);
	scene[0]->GetTransform()->Rotate(0.0f, 0.0f, DirectX::XMScalarSin(deltaTime));
	scene[1]->GetTransform()->SetPosition(0.5f, totalTime * 0.05f, 0.0f);
	scene[1]->GetTransform()->SetScale(0.5f, 0.5f, 0.5f);
	scene[2]->GetTransform()->MoveAbsolute(DirectX::XMScalarSin(deltaTime) * 0.05f, 0.2f * deltaTime, 0.0f);
	scene[3]->GetTransform()->MoveAbsolute(0.01f * deltaTime, 0.0f, 0.0f);
	scene[4]->GetTransform()->Rotate(0.0f, 0.0f, DirectX::XMScalarSin(deltaTime));

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
			entity->Draw(constantBuffer);
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



