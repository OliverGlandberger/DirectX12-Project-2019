#include "TextureDescriptions.hpp"

TextureDescriptions::TextureDescriptions()
{
	// Static Sampler	|	 D3D12Renderer::CreateRootSignature()
	this->staticSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	this->staticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	this->staticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	this->staticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	this->staticSamplerDesc.MipLODBias = 0;
	this->staticSamplerDesc.MaxAnisotropy = 0;
	this->staticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	this->staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	this->staticSamplerDesc.MinLOD = 0.0f;
	this->staticSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	this->staticSamplerDesc.ShaderRegister = 0;
	this->staticSamplerDesc.RegisterSpace = 0;
	this->staticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// ShaderResourceView Heap Description	|	D3D12Texture2D::Constructor()
	this->srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	this->srvHeapDesc.NumDescriptors = 1;
	this->srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	// Texture Resource Description	(and below)	|	D3D12Texture2D::LoadFromFile
	this->textureDesc.MipLevels = 1;
	this->textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	this->textureDesc.Width = 0;		// Needs as input
	this->textureDesc.Height = 0;		// Needs as input
	this->textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	this->textureDesc.DepthOrArraySize = 1;
	this->textureDesc.SampleDesc.Count = 1;
	this->textureDesc.SampleDesc.Quality = 0;
	this->textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	// Texture Heap Properties
	this->texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	this->texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	this->texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	this->texHeapProp.CreationNodeMask = 0;
	this->texHeapProp.VisibleNodeMask = 0;

	// GPU Upload Heap Properties
	this->gpuUploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	this->gpuUploadHeapProp.CPUPageProperty = this->texHeapProp.CPUPageProperty;
	this->gpuUploadHeapProp.MemoryPoolPreference = this->texHeapProp.MemoryPoolPreference;
	this->gpuUploadHeapProp.CreationNodeMask = this->texHeapProp.CreationNodeMask;
	this->gpuUploadHeapProp.VisibleNodeMask = this->texHeapProp.VisibleNodeMask;

	// DXGI Sample Description
	this->sampleDesc = this->textureDesc.SampleDesc;

	// GPU Upload Buffer Description
	this->gpuUploadBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	this->gpuUploadBufferDesc.Alignment = 0;
	this->gpuUploadBufferDesc.Width = 0;	// GIVEN AS INPUT DURING 'GET';
	this->gpuUploadBufferDesc.Height = 1;
	this->gpuUploadBufferDesc.DepthOrArraySize = this->textureDesc.DepthOrArraySize;
	this->gpuUploadBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	this->gpuUploadBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	this->gpuUploadBufferDesc.MipLevels = this->textureDesc.MipLevels;
	this->gpuUploadBufferDesc.SampleDesc = this->sampleDesc;
	this->gpuUploadBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// Shader Resource View Description
	this->srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	this->srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	this->srvDesc.Format = textureDesc.Format;
	this->srvDesc.Texture2D.MipLevels = this->textureDesc.MipLevels;
}

TextureDescriptions::~TextureDescriptions()
{
}

D3D12_STATIC_SAMPLER_DESC TextureDescriptions::getStaticSamplerDesc()
{
	return this->staticSamplerDesc;
}

D3D12_DESCRIPTOR_HEAP_DESC TextureDescriptions::getSrvHeapDesc()
{
	return this->srvHeapDesc;
}

D3D12_RESOURCE_DESC TextureDescriptions::getTextureDesc(UINT width, UINT height)
{
	this->textureDesc.Width = width;
	this->textureDesc.Height = height;
	return this->textureDesc;
}

D3D12_HEAP_PROPERTIES TextureDescriptions::getTextHeapProp()
{
	return this->texHeapProp;
}

D3D12_HEAP_PROPERTIES TextureDescriptions::getGpuUploadHeapProp()
{
	return this->gpuUploadHeapProp;
}

D3D12_RESOURCE_DESC TextureDescriptions::getGpuUploadBufferDesc(UINT64 uploadBufferSize)
{
	this->gpuUploadBufferDesc.Width = uploadBufferSize;
	return this->gpuUploadBufferDesc;
}

DXGI_SAMPLE_DESC TextureDescriptions::getSampleDesc()
{
	return this->sampleDesc;
}

D3D12_SHADER_RESOURCE_VIEW_DESC TextureDescriptions::getSrvDesc()
{
	return this->srvDesc;
}
