#pragma once

class Bundle;

#include "FrameResource.h"

#include "../GlobalDefines.h"

#include <vector>

struct Frame {
	std::vector<FrameResource*> bundles;

	void enable() {
		// Enables all bundles in the frame
		for (auto & bundle : bundles) {
			bundle->enable();
		}
	}
	void destroy() {
		for (size_t i = 0; i < bundles.size(); i++) {
			bundles[i]->destroy();
			delete bundles[i];
			bundles.clear();
		}
	}
};

class FrameResourceManager {
private:
	Frame frames[BACKBUFFERCOUNT];

public:
	FrameResourceManager();
	~FrameResourceManager();

	void initialize();
	void enableFrame(size_t targetFrame);
	void addBundle(Bundle* bundle);
	void clean();
};