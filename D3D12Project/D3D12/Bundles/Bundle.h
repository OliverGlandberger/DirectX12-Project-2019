#pragma once

#include <d3d12.h>
#include "../FrameResource.h"

class ID3D12DescriptorHeap;
class D3D12ConstantBuffer;
class VertexBufferBind;

class Bundle : public FrameResource {
private:
	// Internal Objects
	ID3D12CommandAllocator*		bundleAllocator = nullptr;

	/*
	- Releases internal D3D12 objects.
	- Calls automatically on the destructor!
	*/
	void clean();

protected:
	ID3D12GraphicsCommandList3*	bundle = nullptr;
	// OBS! Not independent! rootIndex is semi-hardcoded!
	void bindConstantBuffer(
		int backBufferIndex,
		ID3D12DescriptorHeap* *descriptorHeapArray,
		D3D12ConstantBuffer* constantBuffer
	);
	// Completely independent, should work independent of outside factors!
	void bindVertexBuffer(const VertexBufferBind& vb, const unsigned int startSlot);
	// Completely independent, should work independent of outside factors!
	void bindTexture(ID3D12DescriptorHeap* descriptorHeap, UINT index);

public:
	Bundle(size_t targetBufferIndex, ID3D12PipelineState* pipelineState);
	virtual ~Bundle();

	// Inherited from FrameResource
	virtual void enable() = 0;

	// OBS! Bundle should be closed after it has been populated
	virtual void populateBundle(void* inputData) = 0;
	void reset(ID3D12GraphicsCommandList3* mainCommandList, ID3D12PipelineState* pipeLineState);
	void appendBundleToCommandList(ID3D12GraphicsCommandList3* mainCommandList);
};