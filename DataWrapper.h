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
	vk::ImageType imageType;
	vk::Extent3D imageSize;
	vk::Format imageFormat;
	vk::Image image = vk::Image();
	vk::DeviceMemory backedMemory = vk::DeviceMemory();
	
	ImageWrapper(vk::Extent3D size, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags());
	~ImageWrapper();
	
};


#endif // DATAWRAPPER_H
