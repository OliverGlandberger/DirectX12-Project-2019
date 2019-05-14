#include "Bundle.h"

#include "../../Tools/Locator.h"
#include <d3d12.h>
#include "../../IA.h"

Bundle::Bundle(size_t targetBufferIndex, ID3D12PipelineState* pipelineState) : FrameResource(targetBufferIndex)
{
	///  ------  Create Bundle Components  ------ 
	ID3D12Device4* device = Locator::getDevice();

	// Create Bundle Allocator
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_BUNDLE,
		IID_PPV_ARGS(&bundleAllocator)
	));
	bundleAllocator->SetName(L"bundleAllocator");

	//// Create Bundle Command Allocator
	//ThrowIfFailed(device->CreateCommandAllocator(
	//	D3D12_COMMAND_LIST_TYPE_DIRECT,
	//	IID_PPV_ARGS(&bundleCommandAllocator)
	//));
	//bundleCommandAllocator->SetName(L"bundleCommandAllocator");

	// Create the bundle
	ThrowIfFailed(device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_BUNDLE,
		bundleAllocator,				// Where is the stack?
		pipelineState,
		IID_PPV_ARGS(&bundle)			// Where is the list?
	));
	bundle->SetName(L"bundle");

	// Closing the bundle is omitted since commands are recorded directly afterwards.
}

Bundle::~Bundle()
{
	this->clean();
}

void Bundle::clean()
{
	// Release Intenal Objects
	SafeRelease(&bundleAllocator);
	SafeRelease(&bundle);
}

void Bundle::bindConstantBuffer(
	int backBufferIndex,
	ID3D12DescriptorHeap* *descriptorHeapArray,
	D3D12ConstantBuffer* constantBuffer
)
{
	ID3D12Resource1* *resourceCB = constantBuffer->getCBResource();

	//Set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = {
	descriptorHeapArray[backBufferIndex]
	};
	this->bundle->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

	int rootIndex;
	int location = constantBuffer->getLocation();
	if (location == PIPELINEINPUT::CB::TRANSLATION_MATRIX) {
		rootIndex = PIPELINEINPUT::IA::ROOTINDEX::VS_CB_TRANSLATION;
	}
	else if (location == PIPELINEINPUT::CB::VIEWPROJ_MATRIX) {
		rootIndex = PIPELINEINPUT::IA::ROOTINDEX::VS_CB_VIEWPROJ;
	}
	else if (location == PIPELINEINPUT::CB::DIFFUSE_TINT) {
		rootIndex = PIPELINEINPUT::IA::ROOTINDEX::PS_CB_DIFFUSE_TINT;
	}

	this->bundle->SetGraphicsRootConstantBufferView(
		rootIndex,
		resourceCB[backBufferIndex]->GetGPUVirtualAddress()
	);
}

void Bundle::bindVertexBuffer(const VertexBufferBind& vb, const unsigned int startSlot)
{
	//Initialize vertex buffer view, used in the render call.
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	vertexBufferView.BufferLocation = vb.buffer->getVertexBufferResource()->GetGPUVirtualAddress() + vb.offset;
	vertexBufferView.StrideInBytes = (UINT)vb.buffer->getStride();
	vertexBufferView.SizeInBytes = (UINT)vb.numElements*vb.sizeElement;

	this->bundle->IASetVertexBuffers(startSlot, 1, &vertexBufferView);
}

void Bundle::bindTexture(ID3D12DescriptorHeap* descriptorHeap, UINT index)
{
	// Set SRV's Description Heap
	ID3D12DescriptorHeap* ppHeaps[] = { descriptorHeap };
	this->bundle->SetDescriptorHeaps(ARRAYSIZE(ppHeaps), ppHeaps);
	this->bundle->SetGraphicsRootDescriptorTable(
		index,
		descriptorHeap->GetGPUDescriptorHandleForHeapStart()
	);
}

void Bundle::reset(ID3D12GraphicsCommandList3 * mainCommandList, ID3D12PipelineState* pipeLineState)
{
	//// Reset the stack
	//ThrowIfFailed(bundleCommandAllocator->Reset());
	//// Reset the main command list
	//ThrowIfFailed(mainCommandList->Reset(bundleCommandAllocator, pipeLineState));
}

void Bundle::appendBundleToCommandList(ID3D12GraphicsCommandList3 * mainCommandList)
{
	mainCommandList->ExecuteBundle(bundle);
}
