#include "D3D12Technique.h"

#include "D3D12Material.h"
#include "D3D12Texture2D.h"
#include "D3D12Transform.h"
#include "D3D12VertexBuffer.h"

#include "../Tools/Locator.h"

D3D12Technique::D3D12Technique(D3D12Material* m, bool wireFrameState) {
	material = m;
	m_wireFrame = wireFrameState;

	//Create pipeline state

	////// Input Layout //////
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);

	////// Pipline State description//////
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};

	//Specify pipeline stages: 
	gpsd.pRootSignature = Locator::getRootSignature();
	gpsd.InputLayout = inputLayoutDesc;
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	material->enable(&gpsd);

	//Specify render target and depthstencil usage.
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;

	//Specify rasterizer behaviour
	if (m_wireFrame == true) {
		gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	}
	else {
		gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	}

	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	//Specify blend descriptions.
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL };
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	//m_pipeLineState = Locator::getPipelineState();

	ThrowIfFailed(Locator::getDevice()->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&m_pipeLineState)));
}

D3D12Technique::~D3D12Technique() {
	if (m_pipeLineState != nullptr) {
		m_pipeLineState->Release();
	}
}

void D3D12Technique::enable(D3D12Renderer* renderer) {

	//Locator::getCommandList()->Reset(Locator::getCommandAllocator(), m_pipeLineState);
	Locator::getCommandList()->SetPipelineState(m_pipeLineState);
}