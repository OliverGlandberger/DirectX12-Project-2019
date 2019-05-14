#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>	// Enables IDXGIFactory4
#include <string>

#include "../GlobalDefines.h"

class D3D12Material;

class D3D12ConstantBuffer {

public:
	D3D12ConstantBuffer(std::string NAME, unsigned int location);
	~D3D12ConstantBuffer();
	void setInitData(const void* data, size_t size, unsigned int location);
	void setData(const void* data, size_t size, unsigned int location);
	void bind();
	void prepareForExec(UINT backBufferIndex);
	void bindToBundle(ID3D12GraphicsCommandList* pCommandList);
	
	unsigned int getLocation() { return this->m_location; }
	IDXGISwapChain3* getSwapChain() { return this->m_swapChain; }
	ID3D12DescriptorHeap* *getDescriptorHeap() { return this->m_descriptorHeap; }
	ID3D12Resource1* *getCBResource() { return this->m_constantBufferResource; }

private:
	const unsigned int m_frameCount = BACKBUFFERCOUNT;
	ID3D12Device4 *m_device;
	IDXGISwapChain3 *m_swapChain;

	ID3D12DescriptorHeap* m_descriptorHeap[BACKBUFFERCOUNT] = {};
	ID3D12Resource1* m_constantBufferResource[BACKBUFFERCOUNT] = {};
	ID3D12GraphicsCommandList3*	m_commandList4;

	std::string m_name;
	unsigned int m_location;
};