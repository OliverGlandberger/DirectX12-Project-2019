#pragma once

#pragma region includes
#include <iostream>
#include <windows.h>
#include <d3d12.h>

#include <dxgi1_6.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>

#include "D3D12ConstantBuffer.h"
#include "D3D12Material.h"
#include "D3D12Mesh.h"
#include "D3D12Renderer.h"
#include "D3D12Sampler2D.h"
#include "D3D12Technique.h"
#include "D3D12Texture2D.h"
#include "D3D12VertexBuffer.h"
#include "Descriptions/TextureDescriptions.hpp"
#include "Bundles/OriginalBundle.h"
#include "D3D12Camera.h"
#include "FrameResourceManager.h"

#include "../GlobalDefines.h"

#pragma endregion
LRESULT CALLBACK wndProc2(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); //Window Proc callback function

namespace CLEAR_BUFFER_FLAGS {
	static const int COLOR = 1;
	static const int DEPTH = 2;
	static const int STENCIL = 4;
};

template<class T> inline void SafeRelease(T **ppInterface) {
	if (*ppInterface != NULL)
	{
		(*ppInterface)->Release();

		(*ppInterface) = NULL;
	}
}

typedef double long dong;

class D3D12Renderer{
private:
	TextureDescriptions textureDescs;
	FrameResourceManager frameResources;
	//OriginalBundle* testBundle = nullptr;
	void setBackBufferAsRenderTarget(UINT backBufferIndex, D3D12_CPU_DESCRIPTOR_HANDLE* cdh);
	void setBackBufferAsPresent(UINT backBufferIndex);
	bool useBundles = false;
	void activateBundleDescriptions();
	std::vector<DirectX::XMMATRIX> translationMatrices;

#pragma region memberVariables
	// Window- & HWND related data
	HWND m_wndHandle;
	unsigned int m_screenWidth, m_screenHeight;

	//Clearing
	float m_clearColor[4] = { 0,0,0,0 };

	// ALL D3D12-DESCRIPTIONS
	TextureDescriptions m_textureDescs;

	/////////////////////////
	//// D3D12 VARIABLES ////
	/////////////////////////
	D3D12_STATIC_SAMPLER_DESC	m_samplerDesc;

	ID3D12Device4*				m_device5 = nullptr;
	ID3D12GraphicsCommandList3*	m_commandList4 = nullptr;

	ID3D12CommandQueue*			m_commandQueue = nullptr;
	ID3D12CommandAllocator*		m_commandAllocators[BACKBUFFERCOUNT];

	IDXGISwapChain4*			m_swapChain4 = nullptr;

	ID3D12Fence1*				m_fence = nullptr;
	HANDLE						m_eventHandle = nullptr;
	UINT64						m_fenceValue = 0;

	ID3D12DescriptorHeap*		m_renderTargetsHeap = nullptr;
	ID3D12Resource1*			m_renderTargets[BACKBUFFERCOUNT] = {};
	UINT						m_renderTargetDescriptorSize = 0;

	D3D12_VIEWPORT				m_viewport = {};
	D3D12_RECT					m_scissorRect = {};

	ID3D12RootSignature*		m_rootSignature = nullptr;
	ID3D12PipelineState*		m_pipeLineState = nullptr;

	// GPU-Timestamps
	struct GPUTimeStamp {
		UINT64 start;
		UINT64 stop;
	};
	int gpuTimerCount = 1;	// Will be directly related to the number of draw calls.
	int maxGpuEntries = TESTDATA::OPTIONS::FRAMECOUNT;
	int writtenGpuEntries = 0;
	int loadingCounter = 0;
	int iterationCounter = 0;
	dong record[TESTDATA::OPTIONS::FRAMECOUNT];
	ID3D12QueryHeap* m_queryHeap;
	ID3D12PipelineState* m_queryState;
	ID3D12Resource* m_queryResourceCPU;
	ID3D12Resource* m_queryResourceGPU;
	void startGpuTimer(ID3D12GraphicsCommandList3* commandList, UINT timeStampID);
	void stopGpuTimer(ID3D12GraphicsCommandList3* commandList, UINT timeStampID);
	void resolveQueryToCPU(ID3D12GraphicsCommandList* commandList, UINT timeStampID);
	void resolveQueryToGPU(ID3D12GraphicsCommandList3* commandList, ID3D12Resource** ppQueryResourceGPUOut);
	void setGPUResourceState(ID3D12GraphicsCommandList3 * commandList, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	GPUTimeStamp getGPUTimeStamp(UINT timeStampID);
	

	//Vertex buffers
	D3D12VertexBuffer* m_pos;
	D3D12VertexBuffer* m_nor;
	D3D12VertexBuffer* m_uvs;

	//Content vectors
	std::vector<D3D12Mesh*> m_scene;
	std::vector<D3D12Material*> m_materials;
	std::vector<D3D12Technique*> m_techniques;
	std::vector<D3D12Texture2D*> m_textures;
	std::vector<D3D12Sampler2D*> m_samplers;

	//Draw list
	std::unordered_map<D3D12Technique*, std::vector<D3D12Mesh*>> m_drawList2;

	//Triangles
	int m_numberOfTriangles = TRIANGLECOUNT;
	float m_trigAngle = 0.0f;

	//Camera
	D3D12Camera m_camera;
	D3D12ConstantBuffer *m_viewProjConstantBuffer;

	//Benchmarking

	int m_recordListsID;

#pragma endregion

#pragma region privateFunctions
	// P R I V A T E     F U N C T I O N S
	//-------------------------------------

	//Helper function for resource transitions - what?
	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);

	HWND initWindow(unsigned int width = 800, unsigned int height = 600); // 1. Creates and returns a window
	void CreateDirect3DDevice(HWND wndHandle);					//2. Create Device
	void CreateCommandInterfacesAndSwapChain(HWND wndHandle);	//3. Create CommandQueue and SwapChain
	void CreateFenceAndEventHandle();							//4. Create Fence and Event handle
	void CreateRenderTargets();									//5. Create render targets for backbuffer
	void CreateViewportAndScissorRect();						//6. Create viewport and rect
	void CreateRootSignature();									//7. Create root signature
	void CreateQueryHeap();

#pragma endregion

	void appendCommandsToCommandList();
public:
#pragma region publicFunctions
	D3D12Renderer();
	~D3D12Renderer();

	//Helper function for syncronization of GPU/CPU
	void WaitForGpu();

	// GPU-Benchmarking functions
	void appendGPUDataToFile(std::string filePath);

	// Initialize functions
	int initialize(unsigned int width = 800, unsigned int height = 600);
	int initialiseTestbench();
	void populateBundles();
	void destroyBundles();
	void submit(D3D12Mesh* mesh);

	// 'Make' functions
	static D3D12Renderer* makeRenderer();
	std::string getShaderPath();
	std::string getShaderExtension();

	void setWinTitle(const char* title);
	void setClearColor(float r, float g, float b, float a);
	void updateScene(double dt);
	void recordList();
	void executeFrame();
	void updateMemoryGPU();
	void present();
	int shutdown();
#pragma endregion
};