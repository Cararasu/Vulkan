#ifndef DATAWRAPPER_H
#define DATAWRAPPER_H

#include "VHeader.h"
#include "VInstance.h"


struct BufferWrapper{
	vk::DeviceSize bufferSize;
	vk::Buffer buffer;
	vk::DeviceMemory backedMemory;
	
	BufferWrapper(VInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags());
	~BufferWrapper();
	
	void destroy(VInstance* instance);
};

struct MappedBufferWrapper : public BufferWrapper{
	void* data;
	
	MappedBufferWrapper(VInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags());
	~MappedBufferWrapper();
	
	void destroy(VInstance* instance);
};
extern MappedBufferWrapper* stagingBuffer;

struct ImageWrapper{
	VInstance* instance;
	vk::Image image;
	vk::DeviceMemory backedMemory;
	vk::Extent3D extent;
	uint32_t mipMapLevels;
	uint32_t arraySize;
	vk::Format format;
	vk::ImageTiling tiling;
	vk::ImageUsageFlags usage;
	vk::ImageType type;
	vk::ImageLayout layout;
	vk::ImageAspectFlags aspectFlags;
	
	ImageWrapper(){}
	
	ImageWrapper(VInstance* instance, vk::Extent3D extent, uint32_t mipMapLevels, uint32_t arraySize, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags());
	~ImageWrapper();
	
	void destroy();
	
	void transitionImageLayout(vk::ImageLayout newLayout, uint32_t mipbase, uint32_t mipcount, uint32_t arrayIndex, uint32_t arrayCount, vk::CommandBuffer commandBuffer);
	void transitionImageLayout(vk::ImageLayout newLayout, uint32_t mipbase, uint32_t mipcount, uint32_t arrayIndex, uint32_t arrayCount, vk::CommandPool commandPool, vk::Queue submitQueue);
	
	inline void transitionImageLayout(vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer){
		transitionImageLayout(newLayout, 0, mipMapLevels, 0, arraySize, commandBuffer);
	}
	inline void transitionImageLayout(vk::ImageLayout newLayout, vk::CommandPool commandPool, vk::Queue submitQueue){
		transitionImageLayout(newLayout, 0, mipMapLevels, 0, arraySize, commandPool, submitQueue);
	}
	
	void generateMipmaps (uint32_t baseLevel, uint32_t generateLevels, uint32_t arrayIndex, uint32_t arrayCount, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer);
	void generateMipmaps (uint32_t baseLevel, uint32_t generateLevels, uint32_t arrayIndex, uint32_t arrayCount, vk::ImageLayout targetLayout, vk::CommandPool commandPool, vk::Queue submitQueue);
	
	inline void generateMipmaps(uint32_t baseLevel, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer){
		generateMipmaps(baseLevel, mipMapLevels, 0, arraySize, targetLayout, commandBuffer);
	}
	inline void generateMipmaps(uint32_t baseLevel, vk::ImageLayout targetLayout, vk::CommandPool commandPool, vk::Queue submitQueue){
		generateMipmaps(baseLevel, mipMapLevels, 0, arraySize, targetLayout, commandPool, submitQueue);
	}
};


#endif // DATAWRAPPER_H
