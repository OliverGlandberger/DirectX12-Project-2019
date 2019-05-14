#pragma once

#include "Bundle.h"

class ID3D12PipelineState;

#include <unordered_map>
#include <vector>
#include <wrl.h>
class D3D12Technique;
class D3D12Mesh;
class ID3D12PipelineState;
class IDXGISwapChain3;
class ID3D12DescriptorHeap;
class ID3D12Resource1;
class VertexBufferBind;
class D3D12ConstantBuffer;

struct OriginalBundleData {
	std::unordered_map<D3D12Technique*, std::vector<D3D12Mesh*>> *drawList;
	D3D12ConstantBuffer* *constantBuffer;
};

class OriginalBundle : public Bundle {
private:
	int currentWork = 0;
public:
	OriginalBundle(size_t targetBufferIndex, ID3D12PipelineState* pipelineState);
	virtual ~OriginalBundle();

	// Inherited from FrameResource
	void enable();
	void destroy();

	// Inherited from Bundle
	void populateBundle(void* inputData);
};