#pragma once

#include <d3d12.h>

//#include "../ClassicModules/VertexBuffer.h"

class D3D12VertexBuffer  {
private:

	ID3D12Device4 *m_device;
	ID3D12GraphicsCommandList3*	m_commandList4;

	size_t m_bufferSize;
	//DATA_USAGE m_usage;
	size_t m_stride;

	ID3D12Resource1*			m_vertexBufferResource = nullptr;
	//D3D12_VERTEX_BUFFER_VIEW	m_vertexBufferView = {};

	UINT8* m_dataBegin = nullptr;    // starting position of upload buffer
	UINT8* m_dataCurrent = nullptr;  // current position of upload buffer
	UINT8* m_dataEnd = nullptr;      // ending position of upload buffer

	//cheap ref counting
	unsigned int refs = 0;

public:
	enum DATA_USAGE { STATIC = 0, DYNAMIC = 1, DONTCARE = 2 };
	D3D12VertexBuffer(size_t size);
	virtual ~D3D12VertexBuffer();

	// Fetched by bundle
	ID3D12Resource1* getVertexBufferResource() { return this->m_vertexBufferResource; }
	size_t getStride() { return this->m_stride; }

	void incRef() { refs++; };
	void decRef() { if (refs > 0) refs--; };
	inline unsigned int refCount() { return refs; };
	
	virtual void setData(const void* data, size_t size, size_t offset);
	virtual void bind(size_t offset, size_t size, unsigned int location);
	void bindBundle(ID3D12GraphicsCommandList* commandList, size_t offset, size_t size, unsigned int location);
	virtual void unbind();
	virtual size_t getSize();
};