#include "Mesh.h"
#include "Graphics.h"

Mesh::Mesh(Vertex* vertices, unsigned int numVertices,
	unsigned int* indices, unsigned int numIndices, std::string meshName)
{
	// Create Vertex Buffer
	{
		D3D11_BUFFER_DESC vbd = {};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex) * numVertices;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initialVertexData = {};
		initialVertexData.pSysMem = vertices;

		Graphics::Device->CreateBuffer(&vbd, &initialVertexData, m_vertexBuffer.GetAddressOf());
	}
	
	// Create index buffer
	{
		D3D11_BUFFER_DESC ibd = {};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(unsigned int) * numIndices;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initialIndexData = {};
		initialIndexData.pSysMem = indices;

		Graphics::Device->CreateBuffer(&ibd, &initialIndexData, m_indexBuffer.GetAddressOf());
	}

	m_vertexCount = numVertices;
	m_indexCount = numIndices;
	m_name = meshName;
}

Mesh::~Mesh()
{

}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer() const
{
	return m_vertexBuffer;
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer() const
{
	return m_indexBuffer;
}

unsigned int Mesh::GetVertexCount() const
{
	return m_vertexCount;
}

unsigned int Mesh::GetIndexCount() const
{
	return m_indexCount;
}

std::string Mesh::GetMeshName() const
{
	return m_name;
}

void Mesh::Draw()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Graphics::Context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	Graphics::Context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	Graphics::Context->DrawIndexed(
		m_indexCount,
		0,
		0);
}