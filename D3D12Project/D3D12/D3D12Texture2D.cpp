#include "D3D12Texture2D.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../include/stb_image.h"

#include "../Tools/Locator.h"

#include "../../include/d3dx12.h"

/////////////////////////////////


/*+-+-+-+-+-+-+-+-+-+-+-+
	PRIVATE FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

void D3D12Texture2D::WaitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(Locator::getCommandQueue()->Signal(m_fence.Get(), fence));
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = Locator::getSwapChain()->GetCurrentBackBufferIndex();
}

UINT64 D3D12Texture2D::updateSubresources(
	ID3D12GraphicsCommandList* pCmdList,
	ID3D12Resource* pDestinationResource,
	ID3D12Resource* pIntermediate,
	UINT64 IntermediateOffset,
	UINT FirstSubresource,
	UINT NumSubresources,
	D3D12_SUBRESOURCE_DATA* pSrcData
)
{
	UINT64 RequiredSize = 0;
	// Determine how much memory needs to be allocated
	UINT64 MemToAlloc = static_cast<UINT64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * NumSubresources;
	if (MemToAlloc > SIZE_MAX)
	{
		return 0;
	}
	// Allocate memory
	void* pMem = HeapAlloc(GetProcessHeap(), 0, static_cast<SIZE_T>(MemToAlloc));
	if (pMem == nullptr)
	{
		return 0;
	}
	auto pLayouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(pMem);
	UINT64* pRowSizesInBytes = reinterpret_cast<UINT64*>(pLayouts + NumSubresources);
	UINT* pNumRows = reinterpret_cast<UINT*>(pRowSizesInBytes + NumSubresources);

	auto Desc = pDestinationResource->GetDesc();
	ID3D12Device* pDevice = nullptr;
	pDestinationResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
	pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, IntermediateOffset, pLayouts, pNumRows, pRowSizesInBytes, &RequiredSize);
	pDevice->Release();

	// Use all defined variables and packaged data above in the below function
	UINT64 Result = updateSubresourcesInternal(pCmdList, pDestinationResource, pIntermediate, FirstSubresource, NumSubresources, RequiredSize, pLayouts, pNumRows, pRowSizesInBytes, pSrcData);
	HeapFree(GetProcessHeap(), 0, pMem);
	return Result;
}

UINT64 D3D12Texture2D::updateSubresourcesInternal(
	ID3D12GraphicsCommandList* pCmdList,
	ID3D12Resource* pDestinationResource,
	ID3D12Resource* pIntermediate,
	UINT FirstSubresource,
	UINT NumSubresources,
	UINT64 RequiredSize,
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
	UINT* pNumRows,
	UINT64* pRowSizesInBytes,
	D3D12_SUBRESOURCE_DATA* pSrcData
)
{
	// Minor validation
	auto IntermediateDesc = pIntermediate->GetDesc();
	auto DestinationDesc = pDestinationResource->GetDesc();
	if (IntermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
		IntermediateDesc.Width < RequiredSize + pLayouts[0].Offset ||
		RequiredSize > SIZE_T(-1) ||
		(DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER &&
		(FirstSubresource != 0 || NumSubresources != 1)))
	{
		return 0;
	}

	BYTE* pData;
	// 'Open' (map) the 'pIntermediate' resource
	HRESULT hr = pIntermediate->Map(0, nullptr, reinterpret_cast<void**>(&pData));
	if (FAILED(hr))
	{
		return 0;
	}

	// Generate/Compile the memory for a subresource and then 'memcpy' that data
	for (UINT i = 0; i < NumSubresources; ++i)
	{
		if (pRowSizesInBytes[i] > SIZE_T(-1)) return 0;
		D3D12_MEMCPY_DEST DestData = { pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, SIZE_T(pLayouts[i].Footprint.RowPitch) * SIZE_T(pNumRows[i]) };

		for (UINT z = 0; z < pLayouts[i].Footprint.Depth; ++z)
		{
			BYTE* pDestSlice = reinterpret_cast<BYTE*>(DestData.pData) + DestData.SlicePitch * z;
			const BYTE* pSrcSlice = reinterpret_cast<const BYTE*>(pSrcData[i].pData) + pSrcData[i].SlicePitch * z;
			for (UINT y = 0; y < pNumRows[i]; ++y)
			{
				memcpy(pDestSlice + DestData.RowPitch * y,
					pSrcSlice + pSrcData[i].RowPitch * y,
					static_cast<SIZE_T>(pRowSizesInBytes[i]));
			}
		}
	}
	// 'Close' (unmap) the 'pIntermediate' resource
	pIntermediate->Unmap(0, nullptr);

	if (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
	{
		pCmdList->CopyBufferRegion(
			pDestinationResource, 0, pIntermediate, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
	}
	else
	{
		for (UINT i = 0; i < NumSubresources; ++i)
		{
			CD3DX12_TEXTURE_COPY_LOCATION Dst(pDestinationResource, i + FirstSubresource);
			CD3DX12_TEXTURE_COPY_LOCATION Src(pIntermediate, pLayouts[i]);
			pCmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);

		}
	}
	return RequiredSize;
}





/*+-+-+-+-+-+-+-+-+-+-+-+
	 PUBLIC FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

D3D12Texture2D::D3D12Texture2D()
{
	// Describe and create a shader resource view (SRV) heap for the texture.
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = texDescs.getSrvHeapDesc();

	ThrowIfFailed(Locator::getDevice()->CreateDescriptorHeap(
		&srvHeapDesc,
		IID_PPV_ARGS(&this->m_srvHeap)
	));
	m_srvHeap->SetName(L"m_srvHeap");
}

D3D12Texture2D::~D3D12Texture2D()
{

}

void D3D12Texture2D::clean()
{
	delete this->rgbTextureData;
}

int D3D12Texture2D::loadFromFile(std::string fileName)
{
	// The actual reading-from-file to load the texture
#pragma region
	int textureWidth, textureHeight, bpp;
	this->rgbTextureData = stbi_load(fileName.c_str(), &textureWidth, &textureHeight, &bpp, STBI_rgb_alpha);
	if (rgbTextureData == nullptr)
	{
		fprintf(stderr, "Error loading texture file: %s\n", fileName.c_str());
		return -1;
	}
#pragma endregion LOAD THE TEXTURE FROM FILE


	/// -------------  Create Committed Resource  --------------------
	// Create the description for the Texture Resource
	D3D12_RESOURCE_DESC textureDesc = texDescs.getTextureDesc(textureWidth, textureHeight);
	
	// Describe a heap for the resource
	D3D12_HEAP_PROPERTIES textureHeapProperties = texDescs.getTextHeapProp();

	// Create and store the texture into its 'ID3D12Resource' object
	ThrowIfFailed(Locator::getDevice()->CreateCommittedResource(
		&textureHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&this->textureResource)));
	textureResource->SetName(L"TextureResource");
	// --------------------------------------------------------------


	// --------------------------------------------------------------
	// Naming and storing some variables we will need
	auto Desc = this->textureResource->GetDesc();
	UINT64 RequiredSize = 0;
	UINT FirstSubresource = 0;
	UINT NumSubresources = 1;

	// Preparing the device
	ID3D12Device* pDevice = nullptr;
	this->textureResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
	pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, 0, nullptr, nullptr, nullptr, &RequiredSize);
	pDevice->Release();

	// Variables needed for the creation of the 'GPU Upload Heap'
	UINT64 uploadBufferSize = RequiredSize;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureUploadHeap;

	// Upload heap for the GPU
	D3D12_HEAP_PROPERTIES gpuUploadHeap = texDescs.getGpuUploadHeapProp();
	DXGI_SAMPLE_DESC sampleDesc = texDescs.getSampleDesc();

	// Upload heap description
	D3D12_RESOURCE_DESC gpuUploadBufferDesc = texDescs.getGpuUploadBufferDesc(uploadBufferSize);

	// Create the GPU upload heap (buffer)
	ThrowIfFailed(Locator::getDevice()->CreateCommittedResource(
		&gpuUploadHeap,
		D3D12_HEAP_FLAG_NONE,
		&gpuUploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUploadHeap)));
	textureUploadHeap->SetName(L"TextureUploadHeap");

	// Describing the texture data
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = rgbTextureData;
	textureData.RowPitch = (sizeof(char) * textureWidth * this->TexturePixelSize);
	textureData.SlicePitch = (textureData.RowPitch * textureHeight * this->TexturePixelSize);

	// Updates the resource data (see the function definition for more details)
	updateSubresources(Locator::getCommandList(), this->textureResource.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);

	// Describe the resource barrier, to-be-connected to the command list
	D3D12_RESOURCE_BARRIER resourceBarriar;
	resourceBarriar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarriar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarriar.Transition.pResource = this->textureResource.Get();
	resourceBarriar.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	resourceBarriar.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	resourceBarriar.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	Locator::getCommandList()->ResourceBarrier(1, &resourceBarriar);

	// Describe and create a SRV for the texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = texDescs.getSrvDesc();

	Locator::getDevice()->CreateShaderResourceView(
		this->textureResource.Get(),
		&srvDesc,
		m_srvHeap.Get()->GetCPUDescriptorHandleForHeapStart()
	);

	// Close the command list and execute it to begin the initial GPU setup
	ThrowIfFailed(Locator::getCommandList()->Close());
	ID3D12CommandList* ppCommandLists[] = { Locator::getCommandList() };
	Locator::getCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(Locator::getDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForPreviousFrame();
	}

	return 1;
}

void D3D12Texture2D::bind(unsigned int slot)
{
	// Fetch commandlist so that we can attach commands to it
	ID3D12GraphicsCommandList3* pCommandList = Locator::getCommandList();
	// Set SRV's Description Heap
	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	pCommandList->SetDescriptorHeaps(ARRAYSIZE(ppHeaps), ppHeaps);
	pCommandList->SetGraphicsRootDescriptorTable(
		3,
		m_srvHeap->GetGPUDescriptorHandleForHeapStart()
	);
}
