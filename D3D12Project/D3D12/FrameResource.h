#pragma once


class FrameResource {
protected:
	size_t targetBufferIndex;

public:
	FrameResource(size_t targetBufferIndex);
	virtual ~FrameResource();

	virtual void enable() = 0;
	virtual void destroy() = 0;
	virtual size_t getTargetBufferIndex();
};