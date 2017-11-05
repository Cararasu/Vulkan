#ifndef DATAWRAPPER_H
#define DATAWRAPPER_H

#include "VHeader.h"


struct BufferWrapper{
	VkDeviceSize bufferSize;
	VkBuffer buffer;
	VkDeviceMemory backedMemory;
	
	BufferWrapper(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended = 0);
	~BufferWrapper();
};

struct MappedBufferWrapper : public BufferWrapper{
	void* data;
	
	MappedBufferWrapper(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended = 0);
	~MappedBufferWrapper();
};
extern MappedBufferWrapper* stagingBuffer;

struct ImageWrapper{
	VkImageType imageType;
	VkExtent3D imageSize;
	VkFormat imageFormat;
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory backedMemory = VK_NULL_HANDLE;
	
	ImageWrapper(VkExtent3D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended = 0);
	~ImageWrapper();
	
};


#endif // DATAWRAPPER_H
