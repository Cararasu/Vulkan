#ifndef DATAWRAPPER_H
#define DATAWRAPPER_H

#include "VHeader.h"
#include "VInstance.h"

struct GPUMemory{
	vk::DeviceMemory memory;
	vk::DeviceSize size;
};

struct BufferWrapper{
	VInstance* instance;
	GPUMemory memory;
	vk::Buffer buffer;
	
	BufferWrapper(VInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags());
	~BufferWrapper();
	
	void destroy();
};

struct MappedBufferWrapper : public BufferWrapper{
	void* data;
	
	MappedBufferWrapper(VInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags());
	~MappedBufferWrapper();
	
	void destroy();
};
extern MappedBufferWrapper* stagingBuffer;

void copyBuffer();

struct ImageWrapper{
	VInstance* instance;
	GPUMemory memory;
	vk::Image image;
	vk::Extent3D extent;
	u32 mipMapLevels;
	u32 arraySize;
	vk::Format format;
	vk::ImageTiling tiling;
	vk::ImageUsageFlags usage;
	vk::ImageType type;
	vk::ImageLayout layout;
	vk::ImageAspectFlags aspectFlags;
	
	ImageWrapper(){}
	
	ImageWrapper(VInstance* instance, vk::Extent3D extent, u32 mipMapLevels, u32 arraySize, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags());
	~ImageWrapper();
	
	void destroy();
	
	void transitionImageLayout(vk::ImageLayout newLayout, u32 mipbase, u32 mipcount, u32 arrayIndex, u32 arrayCount, vk::CommandBuffer commandBuffer);
	void transitionImageLayout(vk::ImageLayout newLayout, u32 mipbase, u32 mipcount, u32 arrayIndex, u32 arrayCount, vk::CommandPool commandPool, vk::Queue submitQueue);
	
	inline void transitionImageLayout(vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer){
		transitionImageLayout(newLayout, 0, mipMapLevels, 0, arraySize, commandBuffer);
	}
	inline void transitionImageLayout(vk::ImageLayout newLayout, vk::CommandPool commandPool, vk::Queue submitQueue){
		transitionImageLayout(newLayout, 0, mipMapLevels, 0, arraySize, commandPool, submitQueue);
	}
	
	void generateMipmaps (u32 baseLevel, u32 generateLevels, u32 arrayIndex, u32 arrayCount, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer);
	void generateMipmaps (u32 baseLevel, u32 generateLevels, u32 arrayIndex, u32 arrayCount, vk::ImageLayout targetLayout, vk::CommandPool commandPool, vk::Queue submitQueue);
	
	inline void generateMipmaps(u32 baseLevel, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer){
		generateMipmaps(baseLevel, mipMapLevels, 0, arraySize, targetLayout, commandBuffer);
	}
	inline void generateMipmaps(u32 baseLevel, vk::ImageLayout targetLayout, vk::CommandPool commandPool, vk::Queue submitQueue){
		generateMipmaps(baseLevel, mipMapLevels, 0, arraySize, targetLayout, commandPool, submitQueue);
	}
};


#endif // DATAWRAPPER_H
