#pragma once

//#include "../ClassicModules/Transform.h"
#include <unordered_map>

class D3D12Technique;
class D3D12VertexBuffer;
class D3D12Texture2D;
class D3D12Transform;
class D3D12VertexBuffer;
class D3D12ConstantBuffer;

struct VertexBufferBind {
	size_t sizeElement, numElements, offset;
	D3D12VertexBuffer* buffer;
};

class D3D12Mesh {
public:
	D3D12Mesh();
	~D3D12Mesh();

	std::unordered_map<unsigned int, VertexBufferBind> geometryBuffers;
	std::unordered_map<unsigned int, D3D12Texture2D*> textures;

	// technique has: Material, RenderState, Attachments (color, depth, etc)
	D3D12Technique* technique;

	// translation buffers
	D3D12ConstantBuffer* txBuffer;
	// local copy of the translation
	D3D12Transform* transform;

	void addTexture(D3D12Texture2D* texture, unsigned int slot);

	// array of buffers with locations (binding points in shaders)
	void addIAVertexBufferBinding(
		D3D12VertexBuffer* buffer,
		size_t offset,
		size_t numElements,
		size_t sizeElement,
		unsigned int inputStream);

	void bindIAVertexBuffer(unsigned int location);
};