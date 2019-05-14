#pragma once

#include "../D3D12/D3D12Renderer.h"
#include "Benchmark.h"
#include <d3dcompiler.h>

#include <dxgi1_6.h>


#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif



inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		// Char buffer
		char stringBuffer[64] = {};
		// Append data to buffer
		sprintf_s(stringBuffer, "HRESULT of 0x%08X", static_cast<UINT>(hr));
		// Throw!
		throw std::runtime_error(std::string(stringBuffer));
	}
}

class Locator {
private:
	static ID3D12RootSignature** gRootSignature;
	static ID3D12Device4** gDevice;
	static IDXGISwapChain4** gSwapChain;
	static ID3D12GraphicsCommandList3** gCommandList;
	static ID3D12CommandAllocator** gCommandAllocator;
	static ID3D12CommandQueue** gCommandQueue;
	static Benchmark** m_benchmark;

public:
	Locator() {}
	~Locator() {}

	// PROVIDE
	static void provide(ID3D12RootSignature** rootSignature) {
		gRootSignature = rootSignature;
	}
	static void provide(ID3D12Device4** device) {
		gDevice = device;
	}
	static void provide(IDXGISwapChain4** swapChain) {
		gSwapChain = swapChain;
	}
	static void provide(ID3D12GraphicsCommandList3** commandList) {
		gCommandList = commandList;
	}
	static void provide(ID3D12CommandAllocator** commandAllocator) {
		gCommandAllocator = commandAllocator;
	}
	static void provide(ID3D12CommandQueue** commandQueue) {
		gCommandQueue = commandQueue;
	}

	static void provide(Benchmark** benchmark) {
		m_benchmark = benchmark;
	}

	// GET
	static ID3D12RootSignature* getRootSignature() {
		return *gRootSignature;
	}
	static ID3D12Device4* getDevice() {
		return *gDevice;
	}
	static IDXGISwapChain4* getSwapChain() {
		return *gSwapChain;
	}
	static ID3D12GraphicsCommandList3* getCommandList() {
		return *gCommandList;
	}
	static ID3D12CommandAllocator* getCommandAllocator() {
		return *gCommandAllocator;
	}
	static ID3D12CommandQueue* getCommandQueue() {
		return *gCommandQueue;
	}
	static Benchmark* getBenchmark() {
		return *m_benchmark;
	}
};