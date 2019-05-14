#pragma once

#include <d3d12.h>

class D3D12Material;
class D3D12Renderer;
class D3D12Technique {

	// INHERITED ---------------------
public:
	D3D12Material* getMaterial() { return material; };
//	D3D12RenderState* getRenderState() { return renderState; };
	ID3D12PipelineState* getPipeLineState() { return m_pipeLineState; }

protected:
	D3D12Material* material = nullptr;

	// WE MADE ---------------------
private:
	ID3D12PipelineState* m_pipeLineState = nullptr;
	bool m_wireFrame;

public:
	D3D12Technique(D3D12Material* m, bool wireFrameState);
	~D3D12Technique();

	void enable(D3D12Renderer* renderer);
};