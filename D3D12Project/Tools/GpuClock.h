#pragma once

#include <d3d12.h>


class GpuClock {
private:
	ID3D12QueryHeap* m_queryHeap;


public:
	GpuClock();
	~GpuClock();
};	