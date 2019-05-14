#include "D3D12Renderer.h"
#include "../Tools/Locator.h"
#include <thread>
#include <fstream>

#include "D3D12Mesh.h"
#include "D3D12Material.h"
#include "D3D12ConstantBuffer.h"
#include "D3D12VertexBuffer.h"
#include "D3D12Technique.h"
#include "D3D12Sampler2D.h"
#include "D3D12Texture2D.h"

#include <d3dx12.h>

#include "../IA.h"

#include "../GlobalDefines.h"

//////////////////////////////



#pragma region wndProc2
LRESULT CALLBACK wndProc2(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
#pragma endregion


/*+-+-+-+-+-+-+-+-+-+-+-+
	PRIVATE FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

#pragma region SetResourceTransitionBarrier
void D3D12Renderer::startGpuTimer(ID3D12GraphicsCommandList3* commandList, UINT timeStampID)
{
	commandList->EndQuery(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, timeStampID * 2);
}
void D3D12Renderer::stopGpuTimer(ID3D12GraphicsCommandList3* commandList, UINT timeStampID)
{
	commandList->EndQuery(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, timeStampID * 2 + 1);
}
void D3D12Renderer::resolveQueryToCPU(ID3D12GraphicsCommandList * commandList, UINT timeStampID)
{
	commandList->ResolveQueryData(
		m_queryHeap,
		D3D12_QUERY_TYPE_TIMESTAMP,
		timeStampID * 2,
		2,
		m_queryResourceCPU,
		sizeof(GPUTimeStamp) * timeStampID
	);
}
void D3D12Renderer::resolveQueryToGPU(ID3D12GraphicsCommandList3 * commandList, ID3D12Resource** ppQueryResourceGPUOut)
{
	// Set GPU Resource State
	setGPUResourceState(
		commandList,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_COPY_DEST
	);

	// Resolve Query
	commandList->ResolveQueryData(
		m_queryHeap,
		D3D12_QUERY_TYPE_TIMESTAMP,
		0,
		gpuTimerCount * 2,
		m_queryResourceGPU,
		0
	);

	// Set GPU Resource State
	setGPUResourceState(
		commandList,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_COPY_SOURCE
	);

	// What is this?
	if (ppQueryResourceGPUOut) {
		*ppQueryResourceGPUOut = m_queryResourceGPU;
	}
}
void D3D12Renderer::setGPUResourceState(ID3D12GraphicsCommandList3 * commandList, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER barrierDesc = {};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = m_queryResourceGPU;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = before;
	barrierDesc.Transition.StateAfter = after;
	commandList->ResourceBarrier(1, &barrierDesc);
}

D3D12Renderer::GPUTimeStamp D3D12Renderer::getGPUTimeStamp(UINT timeStampID)
{
	GPUTimeStamp timeStamp;
	GPUTimeStamp* mappedMemory = nullptr;

	D3D12_RANGE readRange{
		sizeof(timeStamp) * timeStampID,			// Offset to beginning		
		sizeof(timeStamp) * (timeStampID + 1)	// Offset to end
	};
	D3D12_RANGE writeRange{
		0,						// No writing allowed!
		0						// -
	};
	
	ThrowIfFailed(m_queryResourceCPU->Map(0, &readRange, (void**)&mappedMemory));
	mappedMemory += 0;	// If we want more timers simultaneously
	timeStamp = *mappedMemory;
	m_queryResourceCPU->Unmap(0, &writeRange);

	return timeStamp;
}

void D3D12Renderer::appendGPUDataToFile(std::string filePath)
{
	// First 1000 samples, save that time-data
	if (writtenGpuEntries < maxGpuEntries) {
		/// Get TimeData
		// GetTimeStamp
		GPUTimeStamp timeStamp = getGPUTimeStamp(0);
		// dt
		UINT64 dt = timeStamp.stop - timeStamp.start;
		// TimeStampFrequency
		UINT64 queueFrequency;
		m_commandQueue->GetTimestampFrequency(&queueFrequency);
		long double timeStampToMs = (1.0 / queueFrequency) * 1000.0;
		// Time In ms.
		long double timeInMs = dt * timeStampToMs;

		/// Save it to Array
		this->record[writtenGpuEntries] = timeInMs;

		writtenGpuEntries++;
	}
	// After X iterations, average and save to file.
	else if (writtenGpuEntries == maxGpuEntries) {	// Save averaged data to file.

		// ...Reset so we keep going through the next iteration
		loadingCounter = 0;

		// If we're not done yet...
		if (iterationCounter++ < TESTDATA::OPTIONS::ITERATIONS) {
			
			// Calculate the average
			dong average = 0;
			for (int i = 0; i < writtenGpuEntries; i++) {
				average += record[i];
			}
			average /= writtenGpuEntries;

			// Save the average to file
			std::ofstream file;
			std::string fileName = "TimeRecords/GPUTime_B";
			if (USEBUNDLES) {
				fileName += "Y";
			}
			else {
				fileName += "N";
			}
			fileName += "_N" + std::to_string(TRIANGLECOUNT);
			fileName += ".txt";

			file.open(fileName, std::ofstream::app);
			if (file.is_open() == true) {
				file << average << "\n";
				file.close();
			}

			writtenGpuEntries = 0;
		} 
		// If we're done...
		else { 
			// ...Make it so that we don't enter any of these if-cases again.
			writtenGpuEntries = (TESTDATA::OPTIONS::FRAMECOUNT + 1);
		}

	}


}

void D3D12Renderer::SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
	D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
{
	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = resource;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = StateBefore;
	barrierDesc.Transition.StateAfter = StateAfter;

	commandList->ResourceBarrier(1, &barrierDesc);
}
#pragma endregion

#pragma region initWindow
void D3D12Renderer::setBackBufferAsRenderTarget(UINT backBufferIndex, D3D12_CPU_DESCRIPTOR_HANDLE* cdh)
{
	//Indicate that the back buffer will be used as render target.
	SetResourceTransitionBarrier(m_commandList4,
		m_renderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);
	// Alter the backbufferIndex
	cdh->ptr += m_renderTargetDescriptorSize * backBufferIndex;
}

void D3D12Renderer::setBackBufferAsPresent(UINT backBufferIndex)
{
	//Indicate that the back buffer will now be used to present.
	SetResourceTransitionBarrier(m_commandList4,
		m_renderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);
}

void D3D12Renderer::activateBundleDescriptions()
{
	UINT backBufferIndex = m_swapChain4->GetCurrentBackBufferIndex();

	// Set up descriptor heaps in advance to sync with heaps set in bundles
	for (auto work : m_drawList2) {
		D3D12Technique* currentTechnique = work.first;
		D3D12Material * currentMaterial = currentTechnique->getMaterial();
		unsigned int cbIndex = currentMaterial->getConstantBufferIndex();
		D3D12ConstantBuffer* currentCB = currentMaterial->getConstantBuffers()[cbIndex];
		ID3D12DescriptorHeap* *currentDescriptorHeap = currentCB->getDescriptorHeap();

		//Set constant buffer descriptor heap (COLOR)
		ID3D12DescriptorHeap* descriptorHeaps[] = {
			currentDescriptorHeap[backBufferIndex]
		};
		m_commandList4->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

		for (auto mesh : work.second) {
			//Set constant buffer descriptor heap (TRANSFORMATIONS)
			ID3D12DescriptorHeap* descriptorHeaps[] = {
			mesh->txBuffer->getDescriptorHeap()[backBufferIndex]
			};
			m_commandList4->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

			//Set Shader Resource View Descriptor Heap (TEXTURE)
			if (mesh->textures[0] != nullptr) {
				ID3D12DescriptorHeap* ppHeaps[] = { mesh->textures[0]->getSRVHeap().Get() };
				m_commandList4->SetDescriptorHeaps(ARRAYSIZE(ppHeaps), ppHeaps);
			}
		}
	}
}

HWND D3D12Renderer::initWindow(unsigned int width, unsigned int height)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = wndProc2;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"D3D12_Project - Investigating Bundles";
	if (!RegisterClassEx(&wcex))
	{
		return false;
	}

	RECT rc = { 0, 0, (LONG)width, (LONG)height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	return CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		wcex.lpszClassName,
		L"Direct 3D proj",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);
}
#pragma endregion

#pragma region CreateDirect3DDevice
void D3D12Renderer::CreateDirect3DDevice(HWND wndHandle)
{


#ifdef _DEBUG
	ID3D12Debug* dbgC;
	ID3D12Debug1* dbgC1;
	D3D12GetDebugInterface(IID_PPV_ARGS(&dbgC));
	dbgC->QueryInterface(IID_PPV_ARGS(&dbgC1));
	dbgC1->SetEnableGPUBasedValidation(true);

	//Enable the D3D12 debug layer.
	ID3D12Debug1* debugController = nullptr;

	HMODULE mD3D12 = GetModuleHandle(L"D3D12.dll");
	PFN_D3D12_GET_DEBUG_INTERFACE f = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mD3D12, "D3D12GetDebugInterface");
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(true);
	}
	SafeRelease(&debugController);
#endif

	//dxgi1_6 is only needed for the initialization process using the adapter.
	IDXGIFactory6*	factory = nullptr;
	IDXGIAdapter1*	adapter = nullptr;
	//First a factory is created to iterate through the adapters available.
	CreateDXGIFactory(IID_PPV_ARGS(&factory));
	for (UINT adapterIndex = 0;; ++adapterIndex)
	{
		adapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters1(adapterIndex, &adapter))
		{
			break; //No more adapters to enumerate.
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device4), nullptr)))
		{
			break;
		}

		SafeRelease(&adapter);
	}
	if (adapter)
	{
		HRESULT hr = S_OK;
		//Create the actual device.
		if (SUCCEEDED(hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device5))))
		{

		}

		SafeRelease(&adapter);
	}
	else
	{
		//Create warp device if no adapter was found.
		factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device5));
	}
	//m_device5->SetStablePowerState(true);
	SafeRelease(&factory);
}
#pragma endregion

#pragma region CreateCommandInterfacesAndSwapChain
void D3D12Renderer::CreateCommandInterfacesAndSwapChain(HWND wndHandle)
{
	//Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC cqd = {};
	if (m_device5->CreateCommandQueue(&cqd, IID_PPV_ARGS(&m_commandQueue))) {
		throw;
	}


	//Create command allocator. The command allocator object corresponds
	//to the underlying allocations in which GPU commands are stored.
	for (int i = 0; i < BACKBUFFERCOUNT; i++) {
		m_device5->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
	}

	//Create command list.
	if (m_device5->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocators[0],
		nullptr,
		IID_PPV_ARGS(&m_commandList4)))

	//Command lists are created in the recording state. Since there is nothing to
	//record right now and the main loop expects it to be closed, we close it.
	m_commandList4->Close();

	IDXGIFactory5*	factory = nullptr;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));

	//Create swap chain.
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = 0;
	scDesc.Height = 0;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = BACKBUFFERCOUNT;
	scDesc.Scaling = DXGI_SCALING_NONE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = 0;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	IDXGISwapChain1* swapChain1 = nullptr;
	if (SUCCEEDED(factory->CreateSwapChainForHwnd(
		m_commandQueue,
		wndHandle,
		&scDesc,
		nullptr,
		nullptr,
		&swapChain1)))
	{
		if (SUCCEEDED(swapChain1->QueryInterface(IID_PPV_ARGS(&m_swapChain4))))
		{
			m_swapChain4->Release();
		}
	}

	SafeRelease(&factory);
}
#pragma endregion

#pragma region CreateFenceAndEventHandle
void D3D12Renderer::CreateFenceAndEventHandle()
{
	m_device5->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	m_fenceValue = 1;
	//Create an event handle to use for GPU synchronization.
	m_eventHandle = CreateEvent(0, false, false, 0);
}
#pragma endregion

#pragma region CreateRenderTargets
void D3D12Renderer::CreateRenderTargets()
{
	//Create descriptor heap for render target views.
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = BACKBUFFERCOUNT;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	HRESULT hr = m_device5->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_renderTargetsHeap));

	//Create resources for the render targets.
	m_renderTargetDescriptorSize = m_device5->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();

	//One RTV for each frame.
	for (UINT n = 0; n < BACKBUFFERCOUNT; n++)
	{
		hr = m_swapChain4->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
		m_device5->CreateRenderTargetView(m_renderTargets[n], nullptr, cdh);
		cdh.ptr += m_renderTargetDescriptorSize;
	}
}
#pragma endregion

#pragma region CreateViewportAndScissorRect
void D3D12Renderer::CreateViewportAndScissorRect()
{
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Width = (float)m_screenWidth;
	m_viewport.Height = (float)m_screenHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_scissorRect.left = (long)0;
	m_scissorRect.right = (long)m_screenWidth;
	m_scissorRect.top = (long)0;
	m_scissorRect.bottom = (long)m_screenHeight;
}
#pragma endregion

#pragma region CreateRootSignature
void D3D12Renderer::CreateRootSignature()
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(m_device5->CheckFeatureSupport(
		D3D12_FEATURE_ROOT_SIGNATURE,
		&featureData,
		sizeof(featureData)
	))) {
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	//// Construct range for texture-srv's
	CD3DX12_DESCRIPTOR_RANGE1 textureRange[1];
	textureRange->Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,
		0,
		0,
		D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC
	);

	CD3DX12_ROOT_PARAMETER1 rootParam0[PIPELINEINPUT::IA::ROOTINDEX::COUNT];
	rootParam0[PIPELINEINPUT::IA::ROOTINDEX::VS_CB_TRANSLATION].InitAsConstantBufferView(PIPELINEINPUT::CB::TRANSLATION_MATRIX, 0);
	rootParam0[PIPELINEINPUT::IA::ROOTINDEX::VS_CB_VIEWPROJ].InitAsConstantBufferView(PIPELINEINPUT::CB::VIEWPROJ_MATRIX, 0);
	rootParam0[PIPELINEINPUT::IA::ROOTINDEX::PS_CB_DIFFUSE_TINT].InitAsConstantBufferView(PIPELINEINPUT::CB::DIFFUSE_TINT, 0);
	rootParam0[PIPELINEINPUT::IA::ROOTINDEX::PS_TEXTURE].InitAsDescriptorTable(
		1,
		&textureRange[0],
		D3D12_SHADER_VISIBILITY_PIXEL
	);

	// Define static sampler in its description
	m_samplerDesc = m_textureDescs.getStaticSamplerDesc();

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rsDesc0 = {};
	rsDesc0.Init_1_1(
		ARRAYSIZE(rootParam0),
		rootParam0,
		1,
		&m_samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	ID3DBlob* signature;
	ID3DBlob* error;

	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(
		&rsDesc0,
		featureData.HighestVersion,
		&signature,
		&error
	));

	ThrowIfFailed(m_device5->CreateRootSignature(
		0,
		signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)
	));

	Locator::provide(&this->m_rootSignature);
	Locator::provide(&this->m_device5);
	Locator::provide(&this->m_swapChain4);
	Locator::provide(&this->m_commandList4);
	Locator::provide(&this->m_commandAllocators[0]);
	Locator::provide(&this->m_commandQueue);
}
void D3D12Renderer::CreateQueryHeap()
{
	/// ----- HEAP CONSTRUCTION ----- 
	{
		// Init desc
		D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
		queryHeapDesc.Count = gpuTimerCount * 2;
		queryHeapDesc.NodeMask = 0;
		queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

		// Create ID3D12Object
		ThrowIfFailed(m_device5->CreateQueryHeap(
			&queryHeapDesc,
			IID_PPV_ARGS(&m_queryHeap)
		));
	}
	
	/// ----- CPU & GPU Resources ----- 
	{
		// Resource Description
		D3D12_RESOURCE_DESC resDesc;
		ZeroMemory(&resDesc, sizeof(resDesc));
		resDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; 
		resDesc.DepthOrArraySize = 1;
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resDesc.Format = DXGI_FORMAT_UNKNOWN;
		resDesc.Height = 1;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resDesc.MipLevels = 1;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Width = sizeof(GPUTimeStamp) * gpuTimerCount;
		
		// Heap Properties
		D3D12_HEAP_PROPERTIES heapProp;
		heapProp.Type = D3D12_HEAP_TYPE_READBACK;
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.CreationNodeMask = 1;
		heapProp.VisibleNodeMask = 1;

		// CPU Resource
		ThrowIfFailed(m_device5->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_queryResourceCPU)
		));
		m_queryResourceCPU->SetName(L"QueryResourceCPU");

		// GPU Resource
		ThrowIfFailed(m_device5->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,	// Used to be source, but doesn't work with HEAP_TYPE_READBACK above
			nullptr,
			IID_PPV_ARGS(&m_queryResourceGPU)
		));
		m_queryResourceGPU->SetName(L"QueryResourceGPU");
	}
}
void D3D12Renderer::appendCommandsToCommandList()
{

}
void D3D12Renderer::populateBundles()
{
	///  ------  BUNDLES PER FRAME  ----
	// Prepare Input
	OriginalBundleData bundleInput;
	bundleInput.drawList = &m_drawList2;
	bundleInput.constantBuffer = &m_viewProjConstantBuffer;

	// Per Swap Buffer
	for (int swapBufferIndex = 0; swapBufferIndex < BACKBUFFERCOUNT; swapBufferIndex++) {
		// Create Bundle
		Bundle* bundle = DBG_NEW OriginalBundle(swapBufferIndex, m_pipeLineState);
		// Bundle(constantbuffers etc) is populated according to backBufferIndex	
		bundle->populateBundle((void*)&bundleInput);
		// Append Bundles to FrameResourceManager according to backbufferIndex
		this->frameResources.addBundle(bundle);
	}
}

void D3D12Renderer::destroyBundles() {
	this->frameResources.clean();
}

void D3D12Renderer::submit(D3D12Mesh* mesh)
{
	m_drawList2[mesh->technique].push_back(mesh);
}
#pragma endregion





#pragma region publicFuncs
/*+-+-+-+-+-+-+-+-+-+-+-+
	 PUBLIC FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

D3D12Renderer::D3D12Renderer() {
	m_clearColor[0] = 0.2f;
	m_clearColor[1] = 0.2f;
	m_clearColor[2] = 0.2f;
	m_clearColor[3] = 1.0f;

	m_recordListsID = Locator::getBenchmark()->getClockID("RecordLists");

	for (int i = 0; i < BACKBUFFERCOUNT; i++) {
		m_commandAllocators[i] = nullptr;
	}
}

D3D12Renderer::~D3D12Renderer() {

}

void D3D12Renderer::WaitForGpu()
{
	//Signal and increment the fence value.
	const UINT64 tempFence = m_fenceValue;
	m_commandQueue->Signal(m_fence, tempFence);
	m_fenceValue++;

	//Wait until command queue is done.
	if (m_fence->GetCompletedValue() < tempFence)
	{
		m_fence->SetEventOnCompletion(tempFence, m_eventHandle);
		WaitForSingleObject(m_eventHandle, INFINITE);
	}
}

D3D12Renderer * D3D12Renderer::makeRenderer()
{
	return DBG_NEW D3D12Renderer();
}

std::string D3D12Renderer::getShaderPath()
{
	return "..\\assets\\D3D12\\";
}

std::string D3D12Renderer::getShaderExtension()
{
	return ".hlsl";
}

int D3D12Renderer::initialize(unsigned int width, unsigned int height)
{
	m_screenWidth = width;
	m_screenHeight = height;

	m_wndHandle = initWindow(width, height);//1. Create Window

	if (m_wndHandle)
	{
		CreateDirect3DDevice(m_wndHandle);					//2. Create Device

		CreateCommandInterfacesAndSwapChain(m_wndHandle);	//3. Create CommandQueue and SwapChain

		CreateFenceAndEventHandle();						//4. Create Fence and Event handle

		CreateRenderTargets();								//5. Create render targets for backbuffer

		CreateViewportAndScissorRect();						//6. Create viewport and rect

		CreateRootSignature();								//7. Create root signature

		CreateQueryHeap();

		WaitForGpu();

		ShowWindow(m_wndHandle, 1); //Display window

	}

	return 1;
}

int D3D12Renderer::initialiseTestbench()
{
	std::string defineDiffuse = "#define DIFFUSE_SLOT " + std::to_string(DIFFUSE_SLOT) + "\n";

	std::vector<std::vector<std::string>> materialDefs = {
		// vertex shader, fragment shader, defines
		// shader filename extension must be asked to the renderer
		// these strings should be constructed from the IA.h file!!!

		{ "VertexShader", "FragmentShader", ""},

		{ "VertexShader", "FragmentShader", ""},

		{ "VertexShader", "FragmentShader", defineDiffuse},

		{ "VertexShader", "FragmentShader", ""},
	};

	// triangle geometry:
	float4 triPos[3] = { { 0.0f,  0.05f, 0.0f, 1.0f },{ 0.05f, -0.05f, 0.0f, 1.0f },{ -0.05f, -0.05f, 0.0f, 1.0f } };
	float4 triNor[3] = { { 0.0f,  0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f, 0.0f } };
	float2 triUV[3] = { { 0.5f,  -0.99f },{ 1.49f, 1.1f },{ -0.51f, 1.1f } };

	// load Materials.
	std::string shaderPath = this->getShaderPath();
	std::string shaderExtension = this->getShaderExtension();
	float diffuse[4][4] = {
		0.0,0.0,1.0,1.0,
		0.0,1.0,0.0,1.0,
		1.0,1.0,1.0,1.0,
		1.0,0.0,0.0,1.0
	};

	for (int i = 0; i < materialDefs.size(); i++)
	{
		// set material name from text file?
		D3D12Material* m = DBG_NEW D3D12Material("material_" + std::to_string(i));
		m->setShader(shaderPath + materialDefs[i][0] + shaderExtension, D3D12Material::ShaderType::VS);
		m->setShader(shaderPath + materialDefs[i][1] + shaderExtension, D3D12Material::ShaderType::PS);

		m->addDefine(materialDefs[i][2], D3D12Material::ShaderType::VS);
		m->addDefine(materialDefs[i][2], D3D12Material::ShaderType::PS);

		std::string err;
		m->compileMaterial(err);

		// add a constant buffer to the material, to tint every triangle using this material
		m->addConstantBuffer(DIFFUSE_TINT_NAME, PIPELINEINPUT::CB::DIFFUSE_TINT);
		// no need to update anymore
		// when material is bound, this buffer should be also bound for access.

		m->setConstantBuffer(diffuse[i], 4 * sizeof(float), PIPELINEINPUT::CB::DIFFUSE_TINT);

		m_materials.push_back(m);
	}

	// basic technique
	m_techniques.push_back(DBG_NEW D3D12Technique(m_materials.at(0), true));
	m_techniques.push_back(DBG_NEW D3D12Technique(m_materials.at(1), false));
	m_techniques.push_back(DBG_NEW D3D12Technique(m_materials.at(2), false));
	m_techniques.push_back(DBG_NEW D3D12Technique(m_materials.at(3), false));

	// create texture
	D3D12Texture2D* fatboy = DBG_NEW D3D12Texture2D();
	fatboy->loadFromFile("../assets/textures/fatboy.png");
	D3D12Sampler2D* sampler = DBG_NEW D3D12Sampler2D();
	sampler->setWrap(WRAPPING::REPEAT, WRAPPING::REPEAT);
	//fatboy->sampler = sampler;

	m_textures.push_back(fatboy);
	m_samplers.push_back(sampler);

	// pre-allocate one single vertex buffer for ALL triangles
	m_pos = DBG_NEW D3D12VertexBuffer(m_numberOfTriangles * sizeof(triPos));
	m_nor = DBG_NEW D3D12VertexBuffer(m_numberOfTriangles * sizeof(triNor));
	m_uvs = DBG_NEW D3D12VertexBuffer(m_numberOfTriangles * sizeof(triUV));

	// Create a mesh array with 3 basic vertex buffers.
	for (int i = 0; i < m_numberOfTriangles; i++) {
		D3D12Mesh* m = DBG_NEW D3D12Mesh();

		constexpr auto numberOfPosElements = std::extent<decltype(triPos)>::value;
		size_t offset = i * sizeof(triPos);
		m_pos->setData(triPos, sizeof(triPos), offset);
		m->addIAVertexBufferBinding(m_pos, offset, numberOfPosElements, sizeof(float4), POSITION);

		constexpr auto numberOfNorElements = std::extent<decltype(triNor)>::value;
		offset = i * sizeof(triNor);
		m_nor->setData(triNor, sizeof(triNor), offset);
		m->addIAVertexBufferBinding(m_nor, offset, numberOfNorElements, sizeof(float4), NORMAL);

		constexpr auto numberOfUVElements = std::extent<decltype(triUV)>::value;
		offset = i * sizeof(triUV);
		m_uvs->setData(triUV, sizeof(triUV), offset);
		m->addIAVertexBufferBinding(m_uvs, offset, numberOfUVElements, sizeof(float2), TEXTCOORD);

		// we can create a constant buffer outside the material, for example as part of the Mesh.
		m->txBuffer = DBG_NEW D3D12ConstantBuffer(std::string(TRANSLATION_NAME), PIPELINEINPUT::CB::TRANSLATION_MATRIX);

		m->technique = m_techniques.at(i % 4);
		if (i % 4 == 2)
			m->addTexture(m_textures.at(0), DIFFUSE_SLOT);

		m_scene.push_back(m);
	}

	//Camera (view and proj)
	m_viewProjConstantBuffer = new D3D12ConstantBuffer(VIEWPROJ_NAME, PIPELINEINPUT::CB::VIEWPROJ_MATRIX);

	//Define view-proj. Move this to a camera class
	DirectX::XMMATRIX viewProj = m_camera.getViewProjection();

	m_viewProjConstantBuffer->setData(&viewProj, sizeof(viewProj), PIPELINEINPUT::CB::VIEWPROJ_MATRIX);


	// Used to submit/clear each frame, now only submits once.
	for (auto m : m_scene)
	{
		this->submit(m);
	}

	if (BENCHMARK_BUNDLE_CREATE_AND_DESTROY && USEBUNDLES) {
		//Record benchmark for bundle creation and destruction
		int populateBundlesID = Locator::getBenchmark()->getClockID("PopulateBundles");
		int detroyBundlesID = Locator::getBenchmark()->getClockID("DestroyBundles");

		for (int i = 0; i < TESTDATA::OPTIONS::ITERATIONS; i++) {
			Locator::getBenchmark()->startRecording();
			for (int j = 0; j < TESTDATA::OPTIONS::FRAMECOUNT; j++) {
				//Create bundle
				Locator::getBenchmark()->startTest(populateBundlesID);
				this->populateBundles();
				Locator::getBenchmark()->stopTest(populateBundlesID);

				//Destroy bundle
				Locator::getBenchmark()->startTest(detroyBundlesID);
				this->destroyBundles();
				Locator::getBenchmark()->stopTest(detroyBundlesID);
			}
			Locator::getBenchmark()->stopRecording();
		}
	}

	//Create bundle
	this->populateBundles();

	return 0;
}


void D3D12Renderer::setWinTitle(const char * title)
{
	SetWindowTextA(m_wndHandle, title);
}

void D3D12Renderer::setClearColor(float r, float g, float b, float a)
{
	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
	m_clearColor[3] = a;
}

/*
 update positions of triangles in the screen changing a translation only
*/
void D3D12Renderer::updateScene(double dt)
{
	/*
		For each mesh in scene list, update their position
	*/
	{
		const int size = (int)m_scene.size();
		for (int i = 0; i < size; i++)
		{
			float tempAngle = m_trigAngle + i * 6.28f / (size * 2.0f + 1.0f);
			DirectX::XMVECTOR trans{
				1.3f * cosf(3.0f * tempAngle + 2.0f * m_trigAngle),
				-0.6f + 0.3f * sinf(1.0f * tempAngle),
				4.5f - i * (9.0f / size)
			};

			DirectX::XMMATRIX worldMatrix;

			worldMatrix = DirectX::XMMatrixTranslationFromVector(trans);
			this->translationMatrices.push_back(worldMatrix);
		}

	}

	m_trigAngle = (float)fmod(m_trigAngle + dt * 1.0f, 6.28f);

	m_camera.update(dt);

	DirectX::XMMATRIX viewProj = m_camera.getViewProjection();
	this->translationMatrices.push_back(viewProj);

	return;
};

void D3D12Renderer::recordList()
{
	Locator::getBenchmark()->startTest(m_recordListsID);
	UINT backBufferIndex = m_swapChain4->GetCurrentBackBufferIndex();
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();

	m_commandAllocators[backBufferIndex]->Reset();
	m_commandList4->Reset(m_commandAllocators[backBufferIndex], nullptr);

	this->setBackBufferAsRenderTarget(backBufferIndex, &cdh);

	// Non-Bundable-Commands
	{
		m_commandList4->OMSetRenderTargets(1, &cdh, true, nullptr);
		m_commandList4->ClearRenderTargetView(cdh, m_clearColor, 0, nullptr);
		//Set necessary states.
		m_commandList4->RSSetViewports(1, &m_viewport);
		m_commandList4->RSSetScissorRects(1, &m_scissorRect);
	}

	if (USEBUNDLES) {
		// Prep descriptor heap for the camera
		m_viewProjConstantBuffer->prepareForExec(backBufferIndex);

		//Set Shader Resource View Descriptor Heap (TEXTURE)
		ID3D12DescriptorHeap* ppHeaps[] = { m_textures[0]->getSRVHeap().Get() };
		m_commandList4->SetDescriptorHeaps(ARRAYSIZE(ppHeaps), ppHeaps);

		// Set up descriptor heaps in advance to sync with heaps set in bundles
		for (auto work : m_drawList2) {
			D3D12Technique* currentTechnique = work.first;
			D3D12Material * currentMaterial = currentTechnique->getMaterial();
			unsigned int cbIndex = currentMaterial->getConstantBufferIndex();
			D3D12ConstantBuffer* currentCB = currentMaterial->getConstantBuffers()[cbIndex];
			ID3D12DescriptorHeap* *currentDescriptorHeap = currentCB->getDescriptorHeap();

			//Set constant buffer descriptor heap (COLOR)
			ID3D12DescriptorHeap* descriptorHeaps[] = {
				currentDescriptorHeap[backBufferIndex]
			};
			m_commandList4->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

			for (auto mesh : work.second) {
				//Set constant buffer descriptor heap (TRANSFORMATIONS)
				ID3D12DescriptorHeap* descriptorHeaps[] = {
				mesh->txBuffer->getDescriptorHeap()[backBufferIndex]
				};
				m_commandList4->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);
			}
		}

		/// ------------- GPU EXECUTE TIME  ------------- ///
		/// -------------   OPEN ---------------- ///
		startGpuTimer(m_commandList4, 0);		// Start
		this->frameResources.enableFrame(backBufferIndex);
		stopGpuTimer(m_commandList4, 0);		// Stop
		resolveQueryToCPU(m_commandList4, 0);	// Save data for later fetch
		/// -------------  CLOSE  --------------- ///
		/// ------------- GPU EXECUTE TIME  ------------- ///
	}

	else {
		/// ------------- GPU EXECUTE TIME  ------------- ///
		startGpuTimer(m_commandList4, 0);		// Start
		/// -------------   OPEN    ------------- ///

		m_commandList4->SetGraphicsRootSignature(m_rootSignature);
		m_commandList4->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		//Bind view-proj constant buffer
		m_viewProjConstantBuffer->bind();

		// Bind VertexBuffers once, since they are identical for all triangles.
		for (int i = 0; i < m_scene[0]->geometryBuffers.size(); i++) {
			m_scene[0]->bindIAVertexBuffer(i);
		}

		// Bind Texture once, since there is only 1 texture.
		m_textures[0]->bind(0);

		for (auto work : m_drawList2) //Loop through 4 different techniques
		{
			//Enable technique
			work.first->enable(this);

			//Binds colour constant buffer (Can be optimized through grouping by color)
			work.first->getMaterial()->enable();

			for (auto mesh : work.second) //Loop through all meshes that uses the "work" technique
			{
				// Bind Translation - Unique, necessary.
				mesh->txBuffer->bind(/*work.first->getMaterial()*/); //Binds translation constant buffer

				//Add draw command to command list
				m_commandList4->DrawInstanced(3, 1, 0, 0); //3 Vertices, 1 triangle, start with vertex 0 and triangle 0
			}
		}

		/// -------------   CLOSE   ------------- ///
		stopGpuTimer(m_commandList4, 0);		// Stop
		resolveQueryToCPU(m_commandList4, 0);	// Save data for later fetch
		/// ------------- GPU EXECUTE TIME  ------------- ///
	}
	Locator::getBenchmark()->stopTest(m_recordListsID);
}

void D3D12Renderer::executeFrame()
{
	UINT backBufferIndex = m_swapChain4->GetCurrentBackBufferIndex();
	this->setBackBufferAsPresent(backBufferIndex);

	//Close the list to prepare it for execution.
	m_commandList4->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { m_commandList4 };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);
}

void D3D12Renderer::updateMemoryGPU()
{
	for (int i = 0; i < (this->translationMatrices.size() - 1); i++) {
		m_scene.at(i)->txBuffer->setData(&this->translationMatrices.at(i), sizeof(this->translationMatrices.at(i)), PIPELINEINPUT::CB::TRANSLATION_MATRIX);
	}
	m_viewProjConstantBuffer->setData(&(this->translationMatrices.at(this->translationMatrices.size() - 1)), sizeof((this->translationMatrices.at(this->translationMatrices.size() - 1))), PIPELINEINPUT::CB::VIEWPROJ_MATRIX);

	this->translationMatrices.clear();
	this->translationMatrices.resize(0);
}

void D3D12Renderer::present()
{
	//Present the frame.
	DXGI_PRESENT_PARAMETERS pp = {};
	m_swapChain4->Present1(0, 0, &pp);
}

int D3D12Renderer::shutdown()
{
	// Wait, as to not accidentally destroy resources still being used
	this->WaitForGpu();

	//assert(m_pos->refCount() == 0);
	delete m_pos;
	//assert(m_nor->refCount() == 0);
	delete m_nor;
	//assert(m_uvs->refCount() == 0);
	delete m_uvs;

	// shutdown.
	// delete dynamic objects
	for (auto m : m_materials)
	{
		delete m;
	}
	for (auto t : m_techniques)
	{
		delete t;
	}

	for (auto m : m_scene)
	{
		delete m;
	};

	for (auto s : m_samplers)
	{
		delete s;
	}

	for (auto t : m_textures)
	{
		t->clean();
		delete t;
	}

	this->frameResources.clean();

	//WaitForGpu();
	CloseHandle(m_eventHandle);
	SafeRelease(&m_device5);
	SafeRelease(&m_commandQueue);
	SafeRelease(&m_commandList4);
	SafeRelease(&m_swapChain4);
	SafeRelease(&m_queryHeap);

	SafeRelease(&m_fence);

	SafeRelease(&m_renderTargetsHeap);
	for (int i = 0; i < BACKBUFFERCOUNT; i++)
	{
		SafeRelease(&m_renderTargets[i]);
		SafeRelease(&m_commandAllocators[i]);
	}

	SafeRelease(&m_rootSignature);

	delete m_viewProjConstantBuffer;

	return 0;
}


#pragma endregion