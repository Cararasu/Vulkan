#ifndef DATAWRAPPER_H
#define DATAWRAPPER_H

#include "VHeader.h"


struct BufferWrapper{
	vk::DeviceSize bufferSize;
	vk::Buffer buffer;
	vk::DeviceMemory backedMemory;
	
	BufferWrapper(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags());
	~BufferWrapper();
};

struct MappedBufferWrapper : public BufferWrapper{
	void* data;
	
	MappedBufferWrapper(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags());
	~MappedBufferWrapper();
};
extern MappedBufferWrapper* stagingBuffer;

struct ImageWrapper{
	vk::Image image;
	vk::DeviceMemory backedMemory;
	vk::Extent3D extent;
	vk::Format format;
	vk::ImageTiling tiling;
	vk::ImageUsageFlags usage;
	vk::ImageType type;
	vk::ImageLayout layout;
	vk::ImageAspectFlags aspectFlags;
	
	ImageWrapper(){}
	
	ImageWrapper(vk::Extent3D extent, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags());
	~ImageWrapper();
	
	void transitionImageLayout(vk::ImageLayout imageLayout, vk::CommandPool commandPool, vk::Queue submitQueue);
	
};


#endif // DATAWRAPPER_H
