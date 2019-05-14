#ifndef GLOBAL_DEFINES_HPP
#define GLOBAL_DEFINES_HPP

// lissajous points
typedef union {
	struct { float x, y, z, w; };
	struct { float r, g, b, a; };
} float4;

typedef union {
	struct { float x, y; };
	struct { float u, v; };
} float2;

#define USEBUNDLES true
#define TRIANGLECOUNT 200
#define BENCHMARK_BUNDLE_CREATE_AND_DESTROY false
#define BACKBUFFERCOUNT 2

struct TESTDATA {
	struct OPTIONS {
		enum {
			FRAMECOUNT = 100,
			ITERATIONS = 500
		};
	};
};

#endif
