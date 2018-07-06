#pragma once
#include "VulkanInstance.h"


struct VulkanImageWrapper {
	VulkanInstance* instance;
	GPUMemory memory;
	vk::Image image;
	vk::Extent3D extent;
	u32 mipMapLevels;
	u32 arraySize;
	vk::Format format;
	vk::ImageTiling tiling;
	vk::ImageUsageFlags usage;
	vk::ImageType type;
	StaticArray<vk::ImageLayout> layouts;
	vk::ImageAspectFlags aspectFlags;

	VulkanImageWrapper() {}

	VulkanImageWrapper ( VulkanInstance* instance, vk::Extent3D extent, u32 mipMapLevels, u32 arraySize, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	VulkanImageWrapper ( VulkanInstance* instance, vk::Image image, vk::Extent3D extent, u32 mipMapLevels, u32 arraySize, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags );
	virtual ~VulkanImageWrapper();

	void destroy();

	//Vulkan-specific stuff
	vk::ImageMemoryBarrier transition_image_layout_impl ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> miprange, Range<u32> arrayrange, vk::PipelineStageFlags* srcStageFlags, vk::PipelineStageFlags* dstStageFlags );

	void transition_image_layout ( vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer );

	inline void transition_image_layout ( vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer ) {
		transition_image_layout ( newLayout, {0, mipMapLevels}, {0, arraySize}, commandBuffer );
	}

	void generate_mipmaps ( Range<u32> mip_range, Range<u32> array_range, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer );

	inline void generate_mipmaps ( u32 baseLevel, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) {
		generate_mipmaps ( {baseLevel, mipMapLevels}, {0, arraySize}, targetLayout, commandBuffer );
	}
	inline void generate_mipmaps ( vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) {
		generate_mipmaps ( {0, mipMapLevels}, {0, arraySize}, targetLayout, commandBuffer );
	}
};
