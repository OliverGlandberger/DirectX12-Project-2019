#include "FrameResourceManager.h"

#include "Bundles/Bundle.h"
#include "FrameResource.h"


FrameResourceManager::FrameResourceManager()
{
}

FrameResourceManager::~FrameResourceManager()
{
}

void FrameResourceManager::initialize()
{
}

void FrameResourceManager::enableFrame(size_t targetFrame)
{
	this->frames[targetFrame].enable();
}

void FrameResourceManager::addBundle(Bundle* bundle)
{
	this->frames[bundle->getTargetBufferIndex()].bundles.push_back(static_cast<FrameResource*>(bundle));
}

void FrameResourceManager::clean()
{
	for (auto & frame : this->frames) {
		frame.destroy();
	}
}
