#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>

#include "Vertex.h"

class Mesh
{
public:
	Mesh(Vertex* vertices, unsigned int numVertices,
		unsigned int* indices, unsigned int numIndices, std::string name);
	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() const;
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() const;

	unsigned int GetVertexCount() const;
	unsigned int GetIndexCount() const;

	std::string GetMeshName() const;

	void Draw();

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

	unsigned int m_indexCount;
	unsigned int m_vertexCount;

	std::string m_name;
};