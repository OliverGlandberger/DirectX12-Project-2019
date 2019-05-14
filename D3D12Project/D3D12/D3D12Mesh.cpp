#include "D3D12Mesh.h"

#include "D3D12Technique.h"
#include "D3D12VertexBuffer.h"
#include "D3D12Texture2D.h"
#include "D3D12Transform.h"
#include "D3D12VertexBuffer.h"
#include "D3D12ConstantBuffer.h"

D3D12Mesh::D3D12Mesh()
{

};

/*
	buffer: is a VertexBuffer*
	offset: offset of the first byte in the buffer used when binding
	size: how many elements (how many points, normals, UVs) should be read from this buffer
	inputStream: location of the binding in the VertexShader
*/
void D3D12Mesh::addIAVertexBufferBinding(
	D3D12VertexBuffer* buffer,
	size_t offset,
	size_t numElements,
	size_t sizeElement,
	unsigned int inputStream)
{
	// inputStream is unique (has to be!) for this D3D12Mesh
	buffer->incRef();
	geometryBuffers[inputStream] = { sizeElement, numElements, offset, buffer };
};

void D3D12Mesh::bindIAVertexBuffer(unsigned int location)
{
	// no checking if the key is valid...TODO
	const VertexBufferBind& vb = geometryBuffers[location];
	vb.buffer->bind(vb.offset, vb.numElements*vb.sizeElement, location);
}

// note, slot is a value set in the shader as well (registry, or binding)
void D3D12Mesh::addTexture(D3D12Texture2D* texture, unsigned int slot)
{
	// would override the slot if there is another pointer here.
	textures[slot] = texture;
}

D3D12Mesh::~D3D12Mesh()
{
	for (auto g : geometryBuffers) {
		g.second.buffer->decRef();
	}

	delete txBuffer;
}
