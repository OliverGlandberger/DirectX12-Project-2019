#ifndef D3D12_SAMPLER_2D
#define D3D12_SAMPLER_2D

#include <d3d12.h> 
#include <d3dcompiler.h>
#include <dxgi.h>		// DirectX Graphics Infrastructure

// just some values to play with, not complete!
enum WRAPPING { REPEAT = 0, CLAMP = 1 };

// just some values to play with, not complete!
enum FILTER { POINT_SAMPLER = 0, LINEAR = 0 };

class D3D12Sampler2D {
private:
	D3D12_STATIC_SAMPLER_DESC samplerDesc;

public:
	FILTER minFilter, magFilter;
	WRAPPING wrapS, wrapT;
		
	D3D12Sampler2D();
	virtual ~D3D12Sampler2D();

	void setMagFilter(FILTER filter);
	void setMinFilter(FILTER filter);
	void setWrap(WRAPPING s, WRAPPING t);

	D3D12_STATIC_SAMPLER_DESC* createSampler();
};

#endif
