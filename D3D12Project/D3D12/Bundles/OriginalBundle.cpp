#include "OriginalBundle.h"

#include "../../Tools/Locator.h"
#include <d3d12.h>

#include "../D3D12Renderer.h"
#include "../D3D12Mesh.h"
#include "../D3D12Technique.h"

OriginalBundle::OriginalBundle(size_t targetBufferIndex, ID3D12PipelineState* pipelineState)
	: Bundle(targetBufferIndex, pipelineState)
{

}

OriginalBundle::~OriginalBundle()
{
	// Clean is called automatically
}


void OriginalBundle::populateBundle(void* inputData)
{
	OriginalBundleData* input = (OriginalBundleData*)inputData;
	IDXGISwapChain4* swapChain = Locator::getSwapChain();

	ID3D12RootSignature* rootSig = nullptr;
	rootSig = Locator::getRootSignature();
	this->bundle->SetGraphicsRootSignature(rootSig);
	this->bundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Bind view-proj constant buffer
	(*input->constantBuffer)->bindToBundle(this->bundle);

	// Bind VertexBuffers once, since they are identical for all triangles.
	for (auto element : input->drawList->begin()->second[0]->geometryBuffers) {
		this->bindVertexBuffer(input->drawList->begin()->second[0]->geometryBuffers[element.first], element.first);
	}

	// Bind Texture once, since there is only 1 texture.
	for (auto technique : *(input->drawList)) {
		if (technique.second[0]->textures[0] != nullptr) {
			this->bindTexture(technique.second[0]->textures[0]->getSRVHeap().Get(), 3);
			break;
		}
	}

	for (auto work : *(input->drawList)) //Loop through 4 different techniques
	{
		// Initialize values here so code is readable later on
		int backBufferIndex = this->targetBufferIndex;

		D3D12Technique* currentTechnique = work.first;
		D3D12Material* currentMaterial = currentTechnique->getMaterial();
		unsigned int cbIndex = currentMaterial->getConstantBufferIndex();
		D3D12ConstantBuffer* currentCB = currentMaterial->getConstantBuffers()[cbIndex];
		ID3D12DescriptorHeap* *currentDescriptorHeap = currentCB->getDescriptorHeap();

		// Enable technique (via its pipelinestate)
		this->bundle->SetPipelineState(currentTechnique->getPipeLineState());

		//Bind color constant buffer
		if (cbIndex != -1) {
			this->bindConstantBuffer(backBufferIndex, currentDescriptorHeap, currentCB);
		}

		///Loop through all meshes that uses the "work" technique
		for (auto mesh : work.second) 
		{
			///Bind translation constant buffer
			this->bindConstantBuffer(backBufferIndex, mesh->txBuffer->getDescriptorHeap(), mesh->txBuffer);

			///Add draw command to command list
			this->bundle->DrawInstanced(3, 1, 0, 0); //3 Vertices, 1 triangle, start with vertex 0 and triangle 0
		}
	}

	// Close
	this->bundle->Close();
}

void OriginalBundle::enable()
{
	this->appendBundleToCommandList(Locator::getCommandList());
}

void OriginalBundle::destroy()
{
}
