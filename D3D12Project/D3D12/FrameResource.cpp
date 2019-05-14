#include "FrameResource.h"

FrameResource::FrameResource(size_t targetBufferIndex)
{
	this->targetBufferIndex = targetBufferIndex;
}

FrameResource::~FrameResource()
{
}

size_t FrameResource::getTargetBufferIndex()
{
	return this->targetBufferIndex;
}
