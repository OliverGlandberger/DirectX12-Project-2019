#include "GpuClock.h"
#include "Locator.h"

GpuClock::GpuClock()
{
	// Init desc
	D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
	queryHeapDesc.Count = 1;
	queryHeapDesc.NodeMask;
	queryHeapDesc.Type;

	// Create ID3D12Object
	ThrowIfFailed(Locator::getDevice()->CreateQueryHeap(
		&queryHeapDesc,
		IID_PPV_ARGS(&m_queryHeap)
	));
}

GpuClock::~GpuClock()
{
	m_queryHeap->Release();
	m_queryHeap = nullptr;
}
