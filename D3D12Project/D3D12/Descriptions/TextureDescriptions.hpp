#pragma once

#include <d3d12.h>

/* TOUCHES:
- D3D12Renderer::CreateRootSignature()
- D3D12Texture2D::Constructor()
- D3D12Texture2D::LoadFromFile
*/
class TextureDescriptions {
private:
	// Descriptions
	D3D12_STATIC_SAMPLER_DESC		staticSamplerDesc = {};
	D3D12_DESCRIPTOR_HEAP_DESC		srvHeapDesc = {};
	D3D12_RESOURCE_DESC				textureDesc = {};
	D3D12_HEAP_PROPERTIES			texHeapProp = {};
	D3D12_HEAP_PROPERTIES			gpuUploadHeapProp = {};
	D3D12_RESOURCE_DESC				gpuUploadBufferDesc = {};
	DXGI_SAMPLE_DESC				sampleDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

public:
	TextureDescriptions();
	~TextureDescriptions();

	D3D12_STATIC_SAMPLER_DESC		getStaticSamplerDesc();
	D3D12_DESCRIPTOR_HEAP_DESC		getSrvHeapDesc();
	D3D12_RESOURCE_DESC				getTextureDesc(UINT width, UINT height);
	D3D12_HEAP_PROPERTIES			getTextHeapProp();
	D3D12_HEAP_PROPERTIES			getGpuUploadHeapProp();
	D3D12_RESOURCE_DESC				getGpuUploadBufferDesc(UINT64 uploadBufferSize);
	DXGI_SAMPLE_DESC				getSampleDesc();
	D3D12_SHADER_RESOURCE_VIEW_DESC getSrvDesc();
};